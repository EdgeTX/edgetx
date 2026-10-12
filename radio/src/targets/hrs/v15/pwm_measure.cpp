/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Measure incoming servo PWM on V15 J10 (PJ8) via EXTI + us timestamp.
 * 5V rail overcurrent monitor matches servo tester protection.
 */

#include "pwm_measure.h"
#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "auto_measure.h"
#include "stm32_gpio.h"
#include "timers_driver.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"

#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr uint8_t RAW_PULSE_DEPTH = 16;
constexpr uint8_t RAW_PERIOD_DEPTH = 8;
constexpr tmr10ms_t FILTER_PERIOD_10MS = 5;
constexpr uint32_t EMA_HIST_W = 1;
constexpr uint32_t EMA_NEW_W = 9;
constexpr uint32_t EMA_SUM_W = 10;
constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;

volatile bool g_active = false;
volatile bool g_awaitFall = false;
volatile bool g_havePrevRise = false;
volatile bool g_haveSignal = false;
volatile uint32_t g_riseUs = 0;
volatile uint32_t g_lastRiseUs = 0;
volatile tmr10ms_t g_lastEdge10ms = 0;

volatile uint16_t g_rawPulse[RAW_PULSE_DEPTH] = {};
volatile uint8_t g_rawPulseIdx = 0;
volatile uint8_t g_rawPulseCount = 0;
volatile uint16_t g_rawPeriod[RAW_PERIOD_DEPTH] = {};
volatile uint8_t g_rawPeriodIdx = 0;
volatile uint8_t g_rawPeriodCount = 0;

uint16_t g_pulseUs = 0;
uint16_t g_periodUs = 0;
uint16_t g_minUs = 0;
uint16_t g_maxUs = 0;
uint32_t g_emaPulsexN = 0;
uint32_t g_emaPeriodxN = 0;
bool g_emaPulseValid = false;
bool g_emaPeriodValid = false;
tmr10ms_t g_lastFilterTick = 0;

bool g_powerOn = false;
bool g_overcurrentFault = false;
bool g_ocBaselineValid = false;
tmr10ms_t g_ocSettleUntil = 0;
tmr10ms_t g_lastOcCheckTick = 0;
int16_t g_ocBaselineMa = 0;
int16_t g_lastCurrentMa = 0;
int16_t g_currentMa = 0;

int g_savedAuxMode = UART_MODE_NONE;
bool g_auxTakenOver = false;

void setExtSignalPath(bool connect)
{
#if defined(CHIP_FUN_GPIO)
  gpio_init(CHIP_FUN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (connect) {
    gpio_set(CHIP_FUN_GPIO);
  } else {
    gpio_clear(CHIP_FUN_GPIO);
  }
#endif
#if defined(CHIP_CS1_GPIO)
  if (connect) {
    gpio_write(CHIP_CS1_GPIO, 1);
    gpio_write(CHIP_CS2_GPIO, 1);
    gpio_write(CHIP_CS3_GPIO, 1);
  }
#endif
}

void setPortPower(bool on)
{
  gpio_init(SERVO_TESTER_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (on) {
    gpio_set(SERVO_TESTER_PWR_GPIO);
  } else {
    gpio_clear(SERVO_TESTER_PWR_GPIO);
  }
}

void takeOverAuxPort()
{
  if (g_auxTakenOver) return;
  g_savedAuxMode = serialGetMode(SP_AUX1);
  if (g_savedAuxMode != UART_MODE_NONE) {
    serialInit(SP_AUX1, UART_MODE_NONE);
  }
  g_auxTakenOver = true;
}

void restoreAuxPort()
{
  if (!g_auxTakenOver) return;
  g_auxTakenOver = false;
  serialInit(SP_AUX1, g_savedAuxMode);
  setPortPower(false);
}

void pushRawPulse(uint16_t width)
{
  g_rawPulse[g_rawPulseIdx] = width;
  g_rawPulseIdx = (uint8_t)((g_rawPulseIdx + 1) % RAW_PULSE_DEPTH);
  if (g_rawPulseCount < RAW_PULSE_DEPTH) g_rawPulseCount++;
}

void pushRawPeriod(uint16_t period)
{
  g_rawPeriod[g_rawPeriodIdx] = period;
  g_rawPeriodIdx = (uint8_t)((g_rawPeriodIdx + 1) % RAW_PERIOD_DEPTH);
  if (g_rawPeriodCount < RAW_PERIOD_DEPTH) g_rawPeriodCount++;
}

void tripOvercurrent()
{
  if (g_overcurrentFault) return;

  g_overcurrentFault = true;
  g_powerOn = false;
  g_haveSignal = false;
  g_awaitFall = false;
  g_havePrevRise = false;

  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
  setExtSignalPath(false);
  setPortPower(false);
}

void checkOvercurrent()
{
  if (!g_powerOn || g_overcurrentFault) {
    return;
  }

#if !defined(MODULE_BATTERY_SENSOR) || defined(SIMU)
  return;
#else
  const tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - g_lastOcCheckTick) < OC_CHECK_PERIOD_10MS) {
    return;
  }
  g_lastOcCheckTick = now;

  int16_t raw = 0;
  if (!csd203DriverEnsureReady() || !csd203DriverReadCurrent(raw)) {
    return;
  }

  const int16_t ima = (raw < 0) ? static_cast<int16_t>(-raw) : raw;
  g_currentMa = ima;

  if (!g_ocBaselineValid) {
    if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
        PWM_MEASURE_OC_DELTA_MA) {
      tripOvercurrent();
      return;
    }
    g_lastCurrentMa = ima;
    if (static_cast<int32_t>(now - g_ocSettleUntil) >= 0) {
      g_ocBaselineMa = ima;
      g_ocBaselineValid = true;
    }
    return;
  }

  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
      PWM_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }

  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      PWM_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }

  g_lastCurrentMa = ima;
#endif
}

void pwmEdgeIsr()
{
  if (!g_active || g_overcurrentFault || !g_powerOn) return;

  const uint32_t now = timersGetUsTick();
  g_lastEdge10ms = get_tmr10ms();

  if (gpio_read(SERVO_TESTER_PWM_GPIO)) {
    if (g_havePrevRise) {
      const uint32_t period = now - g_lastRiseUs;
      if (period >= PWM_MEASURE_PERIOD_MIN_US &&
          period <= PWM_MEASURE_PERIOD_MAX_US) {
        pushRawPeriod((uint16_t)period);
      }
    }
    g_riseUs = now;
    g_lastRiseUs = now;
    g_havePrevRise = true;
    g_awaitFall = true;
  } else if (g_awaitFall) {
    // Falling edge: pulse width; +offset corrects SSR rising-edge delay
    uint32_t width = now - g_riseUs;
    width += PWM_MEASURE_PULSE_OFFSET_US;
    if (width >= PWM_MEASURE_PULSE_MIN_US && width <= PWM_MEASURE_PULSE_MAX_US) {
      pushRawPulse((uint16_t)width);
      g_haveSignal = true;
    }
    g_awaitFall = false;
  }
}

void sortU16(uint16_t* a, uint8_t n)
{
  for (uint8_t i = 1; i < n; ++i) {
    const uint16_t key = a[i];
    int j = (int)i - 1;
    while (j >= 0 && a[j] > key) {
      a[j + 1] = a[j];
      --j;
    }
    a[j + 1] = key;
  }
}

uint16_t trimmedMean(uint16_t* sorted, uint8_t n)
{
  if (n == 0) return 0;
  if (n < 4) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < n; ++i) sum += sorted[i];
    return (uint16_t)(sum / n);
  }
  const uint8_t trim = n / 4;
  const uint8_t first = trim;
  const uint8_t last = (uint8_t)(n - trim);
  uint32_t sum = 0;
  for (uint8_t i = first; i < last; ++i) sum += sorted[i];
  return (uint16_t)(sum / (last - first));
}

void resetFilterState()
{
  memset((void*)g_rawPulse, 0, sizeof(g_rawPulse));
  memset((void*)g_rawPeriod, 0, sizeof(g_rawPeriod));
  g_rawPulseIdx = 0;
  g_rawPulseCount = 0;
  g_rawPeriodIdx = 0;
  g_rawPeriodCount = 0;
  g_pulseUs = 0;
  g_periodUs = 0;
  g_minUs = 0;
  g_maxUs = 0;
  g_emaPulsexN = 0;
  g_emaPeriodxN = 0;
  g_emaPulseValid = false;
  g_emaPeriodValid = false;
  g_lastFilterTick = 0;
}

void runBalancingFilter()
{
  uint16_t pulseBuf[RAW_PULSE_DEPTH];
  uint16_t periodBuf[RAW_PERIOD_DEPTH];
  const uint8_t pn = g_rawPulseCount;
  const uint8_t rn = g_rawPeriodCount;
  const uint8_t pIdx = g_rawPulseIdx;
  const uint8_t rIdx = g_rawPeriodIdx;

  if (pn == 0) return;

  for (uint8_t i = 0; i < pn; ++i) {
    const uint8_t src =
        (uint8_t)((pIdx + RAW_PULSE_DEPTH - pn + i) % RAW_PULSE_DEPTH);
    pulseBuf[i] = g_rawPulse[src];
  }
  sortU16(pulseBuf, pn);
  const uint16_t balanced = trimmedMean(pulseBuf, pn);

  if (!g_emaPulseValid) {
    g_emaPulsexN = (uint32_t)balanced * EMA_SUM_W;
    g_emaPulseValid = true;
  } else {
    g_emaPulsexN = (g_emaPulsexN * EMA_HIST_W +
                    (uint32_t)balanced * EMA_SUM_W * EMA_NEW_W) /
                   EMA_SUM_W;
  }
  g_pulseUs = (uint16_t)((g_emaPulsexN + EMA_SUM_W / 2) / EMA_SUM_W);

  if (g_minUs == 0 || g_pulseUs < g_minUs) g_minUs = g_pulseUs;
  if (g_pulseUs > g_maxUs) g_maxUs = g_pulseUs;

  if (rn > 0) {
    for (uint8_t i = 0; i < rn; ++i) {
      const uint8_t src =
          (uint8_t)((rIdx + RAW_PERIOD_DEPTH - rn + i) % RAW_PERIOD_DEPTH);
      periodBuf[i] = g_rawPeriod[src];
    }
    sortU16(periodBuf, rn);
    const uint16_t pBalanced = trimmedMean(periodBuf, rn);
    if (!g_emaPeriodValid) {
      g_emaPeriodxN = (uint32_t)pBalanced * EMA_SUM_W;
      g_emaPeriodValid = true;
    } else {
      g_emaPeriodxN = (g_emaPeriodxN * EMA_HIST_W +
                      (uint32_t)pBalanced * EMA_SUM_W * EMA_NEW_W) /
                     EMA_SUM_W;
    }
    g_periodUs = (uint16_t)((g_emaPeriodxN + EMA_SUM_W / 2) / EMA_SUM_W);
  }
}

}  // namespace

bool v15PwmMeasureIsActive(void) { return g_active; }
bool v15PwmMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15PwmMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15PwmMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15PwmMeasureGetCurrentMa(void) { return g_currentMa; }

uint16_t v15PwmMeasureGetPulseUs(void) { return g_pulseUs; }
uint16_t v15PwmMeasureGetPeriodUs(void) { return g_periodUs; }

uint16_t v15PwmMeasureGetFreqHz(void)
{
  if (g_periodUs == 0) return 0;
  return (uint16_t)((1000000U + (g_periodUs / 2U)) / g_periodUs);
}

uint16_t v15PwmMeasureGetMinUs(void) { return g_minUs; }
uint16_t v15PwmMeasureGetMaxUs(void) { return g_maxUs; }

void v15PwmMeasureResetStats(void)
{
  g_minUs = g_pulseUs;
  g_maxUs = g_pulseUs;
}

void v15PwmMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive() && !v15AutoMeasureIsStartingChild()) return;

  takeOverAuxPort();

  g_awaitFall = false;
  g_havePrevRise = false;
  g_haveSignal = false;
  g_riseUs = 0;
  g_lastRiseUs = 0;
  g_lastEdge10ms = get_tmr10ms();
  resetFilterState();
  g_lastFilterTick = get_tmr10ms();

  g_overcurrentFault = false;
  g_ocBaselineValid = false;
  g_ocBaselineMa = 0;
  g_lastCurrentMa = 0;
  g_currentMa = 0;
  g_ocSettleUntil = get_tmr10ms() + OC_SETTLE_10MS;
  g_lastOcCheckTick = 0;

  if (v15AutoMeasureIsStartingChild() && v15AutoMeasureIsPowerOn()) {
    v15ExtPortEnsureSignalPathOpen();
  } else if (!v15ExtPortTryOpenSignalPath()) {
    restoreAuxPort();
    return;
  }

  gpio_init_int(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_BOTH, pwmEdgeIsr);

  g_active = true;
  g_powerOn = true;
  setPortPower(true);
}

void v15PwmMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_awaitFall = false;
  g_havePrevRise = false;
  g_overcurrentFault = false;

  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);

  if (v15AutoMeasureIsActive()) {
    g_auxTakenOver = false;
  } else {
    setExtSignalPath(false);
    setPortPower(false);
    restoreAuxPort();
    rotaryEncoderInit();
#if defined(MODULE_BATTERY_SENSOR)
    v15BatterySensorResumeAfterExtPort();
#endif
  }
}

void v15PwmMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) {
    return;
  }

  const tmr10ms_t now = get_tmr10ms();
  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastEdge10ms) >= PWM_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    g_awaitFall = false;
    g_havePrevRise = false;
    resetFilterState();
    return;
  }

  if ((tmr10ms_t)(now - g_lastFilterTick) >= FILTER_PERIOD_10MS) {
    g_lastFilterTick = now;
    if (g_haveSignal) {
      runBalancingFilter();
    }
  }
}

#else  // !RADIO_V15

bool v15PwmMeasureIsActive(void) { return false; }
bool v15PwmMeasureHasSignal(void) { return false; }
bool v15PwmMeasureIsPowerOn(void) { return false; }
bool v15PwmMeasureOvercurrentFault(void) { return false; }
int16_t v15PwmMeasureGetCurrentMa(void) { return 0; }
uint16_t v15PwmMeasureGetPulseUs(void) { return 0; }
uint16_t v15PwmMeasureGetPeriodUs(void) { return 0; }
uint16_t v15PwmMeasureGetFreqHz(void) { return 0; }
uint16_t v15PwmMeasureGetMinUs(void) { return 0; }
uint16_t v15PwmMeasureGetMaxUs(void) { return 0; }
void v15PwmMeasureResetStats(void) {}
void v15PwmMeasureStart(void) {}
void v15PwmMeasureStop(void) {}
void v15PwmMeasureTask(void) {}

#endif  // RADIO_V15
