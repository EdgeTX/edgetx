/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Soft-PWM servo output on V15 external measurement port (J10).
 * PJ8 has no timer AF, so TIM4 drives the pin via UPDATE/CC1 IRQs.
 *
 * Pulse width is latched only on UPDATE (frame start). Mid-frame CCR
 * writes from the UI caused missed/extended edges and felt "jerky".
 * TIM4 IRQ priority must stay above LTDC/DMA screen or LCD refresh
 * delays the edges by tensâ€“hundreds of Âµs (classic servo stutter).
 */

#include "servo_tester.h"
#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"
#include "dshot_tester.h"
#include "auto_measure.h"
#include "serial.h"
#include "stm32_gpio.h"
#undef UNUSED
#include "stm32_hal_ll.h"
#include "stm32_timer.h"
#include "timers_driver.h"

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr uint16_t DEFAULT_PULSE_MIN_US = 1000;
constexpr uint16_t DEFAULT_PULSE_MAX_US = 2000;
/** Auto sweep: Âµs advanced once per PWM frame (not per 10ms tick). */
constexpr uint16_t DEFAULT_AUTO_SPEED_US = 12;  // Âµs/PWM frame (@50 Hz â‰?600 Âµs/s)
constexpr tmr10ms_t OC_SETTLE_10MS = 20;  // 200ms after power-on before baseline

volatile bool g_active = false;
volatile uint16_t g_pulseUs = SERVO_TESTER_PULSE_DEFAULT_US;
volatile uint16_t g_periodUs = SERVO_TESTER_PERIOD_US;

uint16_t g_pulseMinUs = DEFAULT_PULSE_MIN_US;
uint16_t g_pulseMaxUs = DEFAULT_PULSE_MAX_US;
uint8_t g_mode = SERVO_TESTER_MODE_MANUAL;
uint8_t g_type = SERVO_TESTER_TYPE_STD;
uint8_t g_rate = SERVO_TESTER_RATE_50;
uint8_t g_stick = 0;
uint16_t g_autoSpeedUs = DEFAULT_AUTO_SPEED_US;
int8_t g_autoDir = 1;
tmr10ms_t g_lastOcCheckTick = 0;

int g_savedAuxMode = UART_MODE_NONE;
bool g_auxTakenOver = false;

bool g_powerOn = false;
bool g_overcurrentFault = false;
bool g_ocBaselineValid = false;
tmr10ms_t g_ocSettleUntil = 0;
int16_t g_ocBaselineMa = 0;
int16_t g_lastCurrentMa = 0;
int16_t g_currentMa = 0;

constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;  // 20ms â€?keep touch UI responsive

uint16_t clampPulse(uint16_t pulseUs)
{
  if (pulseUs < g_pulseMinUs) return g_pulseMinUs;
  if (pulseUs > g_pulseMaxUs) return g_pulseMaxUs;
  // High frame rates: pulse must stay inside the period
  if (g_periodUs > 200 && pulseUs >= g_periodUs - 100) {
    return (uint16_t)(g_periodUs - 100);
  }
  return pulseUs;
}

uint16_t clampRangeMax(uint16_t maxUs)
{
  if (maxUs < SERVO_TESTER_PULSE_ABS_MIN_US + 1)
    return SERVO_TESTER_PULSE_ABS_MIN_US + 1;
  if (maxUs > SERVO_TESTER_PULSE_ABS_MAX_US) return SERVO_TESTER_PULSE_ABS_MAX_US;
  return maxUs;
}

uint16_t clampPeriod(uint16_t periodUs)
{
  if (periodUs < SERVO_TESTER_PERIOD_MIN_US) return SERVO_TESTER_PERIOD_MIN_US;
  if (periodUs > SERVO_TESTER_PERIOD_MAX_US) return SERVO_TESTER_PERIOD_MAX_US;
  return periodUs;
}

/** Next-frame pulse (Âµs). Applied in UPDATE ISR only â€?never mid-pulse. */
uint16_t nextFramePulseUs()
{
  uint16_t pulse = g_pulseUs;
  if (pulse < 1) pulse = 1;
  if (pulse >= g_periodUs) pulse = g_periodUs - 1;
  return pulse;
}

/** One auto-sweep step per PWM frame (called from UPDATE ISR). */
void autoSweepFrame()
{
  if (g_mode != SERVO_TESTER_MODE_AUTO) return;
  if (!g_powerOn || g_overcurrentFault) return;

  int32_t next =
      (int32_t)g_pulseUs + (int32_t)g_autoDir * (int32_t)g_autoSpeedUs;
  if (next >= g_pulseMaxUs) {
    next = g_pulseMaxUs;
    g_autoDir = -1;
  } else if (next <= g_pulseMinUs) {
    next = g_pulseMinUs;
    g_autoDir = 1;
  }
  g_pulseUs = (uint16_t)next;
}

void applyPeriod()
{
#if !defined(SIMU)
  if (!g_active) return;
  // ARR preload: new period takes effect at next UPDATE (glitch-free)
  LL_TIM_SetAutoReload(SERVO_TESTER_TIMER, g_periodUs - 1U);
#endif
}

void setExtSignalPath(bool connect)
{
#if defined(CHIP_FUN_GPIO)
  // PJ7-Select: HIGH connects MCU UART/PWM path to J10; LOW disconnects
  gpio_init(CHIP_FUN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (connect) {
    gpio_set(CHIP_FUN_GPIO);
  } else {
    gpio_clear(CHIP_FUN_GPIO);
  }
#endif
#if defined(CHIP_CS1_GPIO)
  // Keep HC138 on unused Y7 so cell mux switches stay off during servo test
  if (connect) {
    gpio_write(CHIP_CS1_GPIO, 1);
    gpio_write(CHIP_CS2_GPIO, 1);
    gpio_write(CHIP_CS3_GPIO, 1);
  }
#endif
}

void setPortPower(bool on)
{
  // PD7-BAT: HIGH enables +5V to J10 via Q13/Q12/Q14 (1=ENABLE, 0=DISABLE)
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
  // Restore AUX function only; servo-tester always leaves port power OFF.
  serialInit(SP_AUX1, g_savedAuxMode);
  setPortPower(false);
}

void pwmTimerStart()
{
#if defined(SIMU)
  return;
#else
  stm32_timer_enable_clock(SERVO_TESTER_TIMER);

  LL_TIM_DisableCounter(SERVO_TESTER_TIMER);
  LL_TIM_DisableIT_UPDATE(SERVO_TESTER_TIMER);
  LL_TIM_DisableIT_CC1(SERVO_TESTER_TIMER);
  LL_TIM_ClearFlag_UPDATE(SERVO_TESTER_TIMER);
  LL_TIM_ClearFlag_CC1(SERVO_TESTER_TIMER);

  const uint32_t timClk = SERVO_TESTER_TIMER_FREQ;
  LL_TIM_SetPrescaler(SERVO_TESTER_TIMER, (timClk / 1000000U) - 1U);
  LL_TIM_EnableARRPreload(SERVO_TESTER_TIMER);
  LL_TIM_SetAutoReload(SERVO_TESTER_TIMER, g_periodUs - 1U);
  LL_TIM_SetCounter(SERVO_TESTER_TIMER, 0);

  // Frozen OC + CC1 IRQ: no pin AF; edges are bit-banged in the ISR
  LL_TIM_OC_SetMode(SERVO_TESTER_TIMER, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FROZEN);
  LL_TIM_OC_DisablePreload(SERVO_TESTER_TIMER, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_SetCompareCH1(SERVO_TESTER_TIMER, nextFramePulseUs());
  LL_TIM_CC_EnableChannel(SERVO_TESTER_TIMER, LL_TIM_CHANNEL_CH1);

  // Must preempt LTDC (prio 4) / screen DMA (prio 6) â€?soft edges are
  // otherwise delayed every LCD refresh and the servo chatters.
  NVIC_SetPriority(SERVO_TESTER_TIMER_IRQn, 1);
  NVIC_EnableIRQ(SERVO_TESTER_TIMER_IRQn);

  LL_TIM_EnableIT_UPDATE(SERVO_TESTER_TIMER);
  LL_TIM_EnableIT_CC1(SERVO_TESTER_TIMER);
  LL_TIM_EnableCounter(SERVO_TESTER_TIMER);
  LL_TIM_GenerateEvent_UPDATE(SERVO_TESTER_TIMER);
#endif
}

void pwmTimerStop()
{
#if defined(SIMU)
  return;
#else
  NVIC_DisableIRQ(SERVO_TESTER_TIMER_IRQn);
  LL_TIM_DisableCounter(SERVO_TESTER_TIMER);
  LL_TIM_DisableIT_UPDATE(SERVO_TESTER_TIMER);
  LL_TIM_DisableIT_CC1(SERVO_TESTER_TIMER);
  LL_TIM_ClearFlag_UPDATE(SERVO_TESTER_TIMER);
  LL_TIM_ClearFlag_CC1(SERVO_TESTER_TIMER);
  stm32_timer_disable_clock(SERVO_TESTER_TIMER);

  gpio_clear(SERVO_TESTER_PWM_GPIO);
#endif
}

void tripOvercurrent()
{
  if (g_overcurrentFault) return;

  g_overcurrentFault = true;
  g_powerOn = false;
  pwmTimerStop();
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

  // Magnitude: shunt polarity may make raw negative under load
  const int16_t ima = (raw < 0) ? static_cast<int16_t>(-raw) : raw;
  g_currentMa = ima;

  if (!g_ocBaselineValid) {
    // Still trip on a sudden jump while settling (e.g. immediate short)
    if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
        SERVO_TESTER_OC_DELTA_MA) {
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

  // Instantaneous rise vs previous sample
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
      SERVO_TESTER_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }

  // Rise vs post-power baseline (stall / sustained fault)
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      SERVO_TESTER_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }

  g_lastCurrentMa = ima;
#endif
}

}  // namespace

#if !defined(SIMU)
extern "C" void SERVO_TESTER_TIMER_IRQHandler()
{
  // Frame start: optional auto step, latch pulse, then drive rising edge.
  if (LL_TIM_IsActiveFlag_UPDATE(SERVO_TESTER_TIMER)) {
    LL_TIM_ClearFlag_UPDATE(SERVO_TESTER_TIMER);

    autoSweepFrame();

    const uint16_t pulse = nextFramePulseUs();
    LL_TIM_OC_SetCompareCH1(SERVO_TESTER_TIMER, pulse);

    if (g_active && g_powerOn && !g_overcurrentFault) {
      // If we entered late past the compare point, skip a truncated pulse
      // rather than emitting a near-zero glitch that makes the servo jump.
      if (LL_TIM_GetCounter(SERVO_TESTER_TIMER) < pulse) {
        gpio_set(SERVO_TESTER_PWM_GPIO);
      } else {
        gpio_clear(SERVO_TESTER_PWM_GPIO);
      }
    } else {
      gpio_clear(SERVO_TESTER_PWM_GPIO);
    }
  }

  // Falling edge at pulse width
  if (LL_TIM_IsActiveFlag_CC1(SERVO_TESTER_TIMER)) {
    LL_TIM_ClearFlag_CC1(SERVO_TESTER_TIMER);
    gpio_clear(SERVO_TESTER_PWM_GPIO);
  }
}
#endif

bool v15ServoTesterIsActive(void)
{
  return g_active;
}

bool v15ServoTesterIsPowerOn(void)
{
  return g_powerOn && !g_overcurrentFault;
}

bool v15ServoTesterOvercurrentFault(void)
{
  return g_overcurrentFault;
}

int16_t v15ServoTesterGetCurrentMa(void)
{
  return g_currentMa;
}

void v15ServoTesterStart(void)
{
  if (g_active) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive()) return;

  takeOverAuxPort();

  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
  gpio_clear(SERVO_TESTER_PWM_GPIO);

  g_pulseUs = clampPulse(g_pulseUs);
  g_overcurrentFault = false;
  g_ocBaselineValid = false;
  g_ocBaselineMa = 0;
  g_lastCurrentMa = 0;
  g_currentMa = 0;
  g_ocSettleUntil = get_tmr10ms() + OC_SETTLE_10MS;
  g_lastOcCheckTick = 0;

  if (!v15ExtPortTryOpenSignalPath()) {
    restoreAuxPort();
    return;
  }

  g_active = true;
  g_powerOn = true;
  pwmTimerStart();
  setPortPower(true);
}

void v15ServoTesterStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_overcurrentFault = false;
  pwmTimerStop();

  // Disconnect external signal path and cut +5V first
  setExtSignalPath(false);
  setPortPower(false);

  restoreAuxPort();
  setPortPower(false);

#if defined(MODULE_BATTERY_SENSOR)
  v15BatterySensorResumeAfterExtPort();
#endif
}

void v15ServoTesterSetPulseUs(uint16_t pulseUs)
{
  // Only store the request; TIM4 UPDATE ISR latches CCR1 each frame so
  // knob/auto/stick updates never shorten/extend the pulse mid-way.
  g_pulseUs = clampPulse(pulseUs);
}

uint16_t v15ServoTesterGetPulseUs(void)
{
  return g_pulseUs;
}

void v15ServoTesterSetPulseRange(uint16_t minUs, uint16_t maxUs)
{
  if (minUs < SERVO_TESTER_PULSE_ABS_MIN_US) minUs = SERVO_TESTER_PULSE_ABS_MIN_US;
  maxUs = clampRangeMax(maxUs);
  if (minUs >= maxUs) {
    minUs = maxUs > SERVO_TESTER_PULSE_ABS_MIN_US + 1
                ? maxUs - 1
                : SERVO_TESTER_PULSE_ABS_MIN_US;
  }
  g_pulseMinUs = minUs;
  g_pulseMaxUs = maxUs;
  v15ServoTesterSetPulseUs(g_pulseUs);
}

uint16_t v15ServoTesterGetPulseMinUs(void)
{
  return g_pulseMinUs;
}

uint16_t v15ServoTesterGetPulseMaxUs(void)
{
  return g_pulseMaxUs;
}

void v15ServoTesterSetPeriodUs(uint16_t periodUs)
{
  g_periodUs = clampPeriod(periodUs);
  // Keep working range inside the new period
  if (g_pulseMaxUs >= g_periodUs - 100) {
    g_pulseMaxUs = (uint16_t)(g_periodUs - 100);
    if (g_pulseMaxUs <= g_pulseMinUs) {
      g_pulseMinUs = (g_pulseMaxUs > SERVO_TESTER_PULSE_ABS_MIN_US + 1)
                         ? (uint16_t)(g_pulseMaxUs - 1)
                         : SERVO_TESTER_PULSE_ABS_MIN_US;
    }
  }
  v15ServoTesterSetPulseUs(g_pulseUs);
  applyPeriod();
}

uint16_t v15ServoTesterGetPeriodUs(void) { return g_periodUs; }

uint16_t v15ServoTesterGetFrameHz(void)
{
  if (g_periodUs == 0) return 0;
  return (uint16_t)((1000000U + (g_periodUs / 2U)) / g_periodUs);
}

void v15ServoTesterSetType(uint8_t type)
{
  if (type >= SERVO_TESTER_TYPE_COUNT) type = SERVO_TESTER_TYPE_STD;
  g_type = type;
  switch (type) {
    case SERVO_TESTER_TYPE_WIDE:
      if (g_rate == SERVO_TESTER_RATE_560) v15ServoTesterSetRate(SERVO_TESTER_RATE_50);
      v15ServoTesterSetPulseRange(500, 2500);
      v15ServoTesterSetPulseUs(SERVO_TESTER_PULSE_DEFAULT_US);
      break;
    case SERVO_TESTER_TYPE_DIGI:
      if (g_rate == SERVO_TESTER_RATE_560) v15ServoTesterSetRate(SERVO_TESTER_RATE_50);
      v15ServoTesterSetPulseRange(900, 2100);
      v15ServoTesterSetPulseUs(SERVO_TESTER_PULSE_DEFAULT_US);
      break;
    case SERVO_TESTER_TYPE_HELI760:
      // KST HLS / similar: 760Âµs center, 560 Hz narrow-band
      v15ServoTesterSetPulseRange(450, 1050);
      v15ServoTesterSetRate(SERVO_TESTER_RATE_560);
      v15ServoTesterSetPulseUs(760);
      break;
    case SERVO_TESTER_TYPE_STD:
    default:
      if (g_rate == SERVO_TESTER_RATE_560) v15ServoTesterSetRate(SERVO_TESTER_RATE_50);
      v15ServoTesterSetPulseRange(1000, 2000);
      v15ServoTesterSetPulseUs(SERVO_TESTER_PULSE_DEFAULT_US);
      break;
  }
}

uint8_t v15ServoTesterGetType(void) { return g_type; }

void v15ServoTesterSetRate(uint8_t rate)
{
  if (rate >= SERVO_TESTER_RATE_COUNT) rate = SERVO_TESTER_RATE_50;
  g_rate = rate;
  switch (rate) {
    case SERVO_TESTER_RATE_100:
      v15ServoTesterSetPeriodUs(10000);
      break;
    case SERVO_TESTER_RATE_200:
      v15ServoTesterSetPeriodUs(5000);
      break;
    case SERVO_TESTER_RATE_560:
      // 1000000 / 560 â‰?1786 Âµs â€?pair with Heli 760 pulse window
      v15ServoTesterSetPeriodUs(1786);
      if (g_type != SERVO_TESTER_TYPE_HELI760) {
        g_type = SERVO_TESTER_TYPE_HELI760;
        v15ServoTesterSetPulseRange(450, 1050);
        v15ServoTesterSetPulseUs(760);
      }
      break;
    case SERVO_TESTER_RATE_50:
    default:
      v15ServoTesterSetPeriodUs(20000);
      break;
  }
}

uint8_t v15ServoTesterGetRate(void) { return g_rate; }

void v15ServoTesterSetMode(uint8_t mode)
{
  if (mode > SERVO_TESTER_MODE_STICK) mode = SERVO_TESTER_MODE_MANUAL;
  g_mode = mode;
  g_autoDir = 1;
}

uint8_t v15ServoTesterGetMode(void)
{
  return g_mode;
}

void v15ServoTesterSetStick(uint8_t stick)
{
  // 0=Ail, 1=Ele, 2=Rud
  if (stick > 2) stick = 0;
  g_stick = stick;
}

uint8_t v15ServoTesterGetStick(void)
{
  return g_stick;
}

void v15ServoTesterSetAutoSpeed(uint16_t usPerFrame)
{
  // Âµs per PWM frame (50 Hz â†?12 Âµs/frame â‰?600 Âµs/s default)
  if (usPerFrame < 1) usPerFrame = 1;
  if (usPerFrame > 50) usPerFrame = 50;
  g_autoSpeedUs = usPerFrame;
}

uint16_t v15ServoTesterGetAutoSpeed(void)
{
  return g_autoSpeedUs;
}

void v15ServoTesterTask(void)
{
  if (!g_active) return;

  // OC only here â€?stick pulse is updated in v15ServoTesterMixerHook()
  // so LVGL waveform redraw cannot stall the servo output.
  checkOvercurrent();
}

void v15ServoTesterMixerHook(void)
{
  if (!g_active) return;
  if (g_overcurrentFault || !g_powerOn) return;
  if (g_mode != SERVO_TESTER_MODE_STICK) return;

  // Raw sticks (Ail+Ele+Rud), not CH mixes â€?Slow/expo on channels would
  // step or lag; ADC sticks match how a receiver feels when you move the TX.
  const int32_t v1 = getValue(MIXSRC_FIRST_STICK + 0);
  const int32_t v2 = getValue(MIXSRC_FIRST_STICK + 1);
  const int32_t v4 = getValue(MIXSRC_FIRST_STICK + 3);

  int32_t sum = v1 + v2 + v4;
  if (sum > RESX) sum = RESX;
  if (sum < -RESX) sum = -RESX;

  const int32_t pmin = g_pulseMinUs;
  const int32_t pmax = g_pulseMaxUs;
  const int32_t span = pmax - pmin;
  int32_t pulse = pmin + ((sum + RESX) * span) / (2 * RESX);
  if (pulse < pmin) pulse = pmin;
  if (pulse > pmax) pulse = pmax;
  v15ServoTesterSetPulseUs((uint16_t)pulse);
}

#else  // !RADIO_V15

uint8_t g_mode = SERVO_TESTER_MODE_MANUAL;
uint8_t g_type = SERVO_TESTER_TYPE_STD;
uint8_t g_rate = SERVO_TESTER_RATE_50;
uint16_t g_pulseUs = SERVO_TESTER_PULSE_DEFAULT_US;

bool v15ServoTesterIsActive(void) { return false; }
bool v15ServoTesterIsPowerOn(void) { return false; }
bool v15ServoTesterOvercurrentFault(void) { return false; }
int16_t v15ServoTesterGetCurrentMa(void) { return 0; }
void v15ServoTesterStart(void) {}
void v15ServoTesterStop(void) {}
void v15ServoTesterSetPulseUs(uint16_t v) { g_pulseUs = v; }
uint16_t v15ServoTesterGetPulseUs(void) { return g_pulseUs; }
void v15ServoTesterSetPulseRange(uint16_t, uint16_t) {}
uint16_t v15ServoTesterGetPulseMinUs(void) { return 1000; }
uint16_t v15ServoTesterGetPulseMaxUs(void) { return 2000; }
void v15ServoTesterSetPeriodUs(uint16_t) {}
uint16_t v15ServoTesterGetPeriodUs(void) { return SERVO_TESTER_PERIOD_US; }
uint16_t v15ServoTesterGetFrameHz(void) { return 50; }
void v15ServoTesterSetType(uint8_t type) { g_type = type; }
uint8_t v15ServoTesterGetType(void) { return g_type; }
void v15ServoTesterSetRate(uint8_t rate) { g_rate = rate; }
uint8_t v15ServoTesterGetRate(void) { return g_rate; }
void v15ServoTesterSetMode(uint8_t mode) { g_mode = mode; }
uint8_t v15ServoTesterGetMode(void) { return g_mode; }
void v15ServoTesterSetStick(uint8_t) {}
uint8_t v15ServoTesterGetStick(void) { return 0; }
void v15ServoTesterSetAutoSpeed(uint16_t) {}
uint16_t v15ServoTesterGetAutoSpeed(void) { return 20; }
void v15ServoTesterTask(void) {}
void v15ServoTesterMixerHook(void) {}

#endif  // RADIO_V15
