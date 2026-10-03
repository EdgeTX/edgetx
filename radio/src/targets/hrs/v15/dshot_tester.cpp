/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * DShot ESC output on V15 J10 (PJ8). Pin has no timer AF, so frames are
 * bit-banged with DWT timing from a 1 kHz TIM7 UPDATE ISR (TIM4 is servo).
 */

#include "dshot_tester.h"
#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "delays_driver.h"
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
#include "auto_measure.h"
#include "servo_tester.h"
#include "serial.h"
#include "stm32_gpio.h"
#undef UNUSED
#include "stm32_hal_ll.h"
#include "stm32_timer.h"
#include "timers_driver.h"

#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;
constexpr uint8_t DEFAULT_AUTO_MAX_PCT = 15;
constexpr uint16_t CMD_BURST_FRAMES = 40;  // ~40 ms @ 1 kHz
/** Auto: 1% step every N frames (~50 ms �?~20 %/s). */
constexpr uint16_t AUTO_STEP_FRAMES = 50;

volatile bool g_active = false;
volatile bool g_armed = false;
volatile uint8_t g_protocol = DSHOT_TESTER_PROTO_300;
volatile uint8_t g_mode = DSHOT_TESTER_MODE_MANUAL;
volatile uint8_t g_throttlePct = 0;
volatile uint8_t g_autoMaxPct = DEFAULT_AUTO_MAX_PCT;
volatile int8_t g_autoDir = 1;
volatile uint16_t g_autoFrameDiv = 0;
volatile uint16_t g_cmdValue = 0;
volatile uint16_t g_cmdFramesLeft = 0;
volatile uint16_t g_outputValue = 0;

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

struct DshotTiming {
  uint32_t period01us;
  uint32_t t0h01us;
  uint32_t t1h01us;
};

// Nominal DShot bit timings in 0.1 us units (Betaflight-compatible).
const DshotTiming kTimings[DSHOT_TESTER_PROTO_COUNT] = {
    {67, 25, 50},  // 150
    {33, 12, 25},  // 300
    {17, 6, 12},   // 600
    {8, 3, 6},     // 1200
};

uint8_t clampPct(uint8_t pct)
{
  if (pct > DSHOT_TESTER_THROTTLE_MAX) return DSHOT_TESTER_THROTTLE_MAX;
  return pct;
}

uint16_t throttlePercentToValue(uint8_t pct)
{
  pct = clampPct(pct);
  if (pct == 0) return DSHOT_TESTER_VALUE_STOP;
  // Map 1�?00% �?48�?047
  return (uint16_t)(DSHOT_TESTER_VALUE_THROTTLE_MIN +
                    ((uint32_t)(pct - 1) *
                     (DSHOT_TESTER_VALUE_THROTTLE_MAX -
                      DSHOT_TESTER_VALUE_THROTTLE_MIN)) /
                        99u);
}

uint16_t dshotEncode(uint16_t value11, bool telemetry)
{
  if (value11 > DSHOT_TESTER_VALUE_THROTTLE_MAX)
    value11 = DSHOT_TESTER_VALUE_THROTTLE_MAX;
  uint16_t packet = (uint16_t)((value11 << 1) | (telemetry ? 1u : 0u));
  uint16_t csum = 0;
  uint16_t data = packet;
  for (int i = 0; i < 3; ++i) {
    csum ^= data;
    data >>= 4;
  }
  csum &= 0x0F;
  return (uint16_t)((packet << 4) | csum);
}

uint16_t nextOutputValue()
{
  if (g_cmdFramesLeft > 0) {
    --g_cmdFramesLeft;
    return g_cmdValue;
  }
  if (!g_armed || g_overcurrentFault || !g_powerOn) {
    return DSHOT_TESTER_VALUE_STOP;
  }
  return throttlePercentToValue(g_throttlePct);
}

void autoSweepFrame()
{
  if (g_mode != DSHOT_TESTER_MODE_AUTO) return;
  if (!g_armed || g_overcurrentFault || !g_powerOn) return;
  if (g_cmdFramesLeft > 0) return;

  if (++g_autoFrameDiv < AUTO_STEP_FRAMES) return;
  g_autoFrameDiv = 0;

  int32_t next = (int32_t)g_throttlePct + g_autoDir;
  const uint8_t maxPct = g_autoMaxPct;
  if (next >= maxPct) {
    next = maxPct;
    g_autoDir = -1;
  } else if (next <= 0) {
    next = 0;
    g_autoDir = 1;
  }
  g_throttlePct = (uint8_t)next;
}

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

void sendDshotFrame(uint16_t packet, uint8_t proto)
{
#if defined(SIMU)
  (void)packet;
  (void)proto;
  return;
#else
  if (proto >= DSHOT_TESTER_PROTO_COUNT) proto = DSHOT_TESTER_PROTO_300;
  const DshotTiming t = kTimings[proto];

  // Busy-wait bitbang; TIM7 IRQ is already high priority.
  for (int i = 15; i >= 0; --i) {
    const bool one = (packet >> i) & 1;
    const uint32_t high = one ? t.t1h01us : t.t0h01us;
    gpio_set(SERVO_TESTER_PWM_GPIO);
    delay_01us(high);
    gpio_clear(SERVO_TESTER_PWM_GPIO);
    if (t.period01us > high) {
      delay_01us(t.period01us - high);
    }
  }
#endif
}

void frameTimerStart()
{
#if defined(SIMU)
  return;
#else
  stm32_timer_enable_clock(DSHOT_TESTER_TIMER);

  LL_TIM_DisableCounter(DSHOT_TESTER_TIMER);
  LL_TIM_DisableIT_UPDATE(DSHOT_TESTER_TIMER);
  LL_TIM_ClearFlag_UPDATE(DSHOT_TESTER_TIMER);

  const uint32_t timClk = DSHOT_TESTER_TIMER_FREQ;
  LL_TIM_SetPrescaler(DSHOT_TESTER_TIMER, (timClk / 1000000U) - 1U);
  LL_TIM_EnableARRPreload(DSHOT_TESTER_TIMER);
  // 1000 us -> 1 kHz frame rate
  LL_TIM_SetAutoReload(DSHOT_TESTER_TIMER, 1000U - 1U);
  LL_TIM_SetCounter(DSHOT_TESTER_TIMER, 0);

  NVIC_SetPriority(DSHOT_TESTER_TIMER_IRQn, 1);
  NVIC_EnableIRQ(DSHOT_TESTER_TIMER_IRQn);

  LL_TIM_EnableIT_UPDATE(DSHOT_TESTER_TIMER);
  LL_TIM_EnableCounter(DSHOT_TESTER_TIMER);
  LL_TIM_GenerateEvent_UPDATE(DSHOT_TESTER_TIMER);
#endif
}

void frameTimerStop()
{
#if defined(SIMU)
  return;
#else
  NVIC_DisableIRQ(DSHOT_TESTER_TIMER_IRQn);
  LL_TIM_DisableCounter(DSHOT_TESTER_TIMER);
  LL_TIM_DisableIT_UPDATE(DSHOT_TESTER_TIMER);
  LL_TIM_ClearFlag_UPDATE(DSHOT_TESTER_TIMER);
  stm32_timer_disable_clock(DSHOT_TESTER_TIMER);
  gpio_clear(SERVO_TESTER_PWM_GPIO);
#endif
}

void tripOvercurrent()
{
  if (g_overcurrentFault) return;
  g_overcurrentFault = true;
  g_powerOn = false;
  g_armed = false;
  g_throttlePct = 0;
  g_cmdFramesLeft = 0;
  frameTimerStop();
  setExtSignalPath(false);
  setPortPower(false);
}

void checkOvercurrent()
{
  if (!g_powerOn || g_overcurrentFault) return;

#if !defined(MODULE_BATTERY_SENSOR) || defined(SIMU)
  return;
#else
  const tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - g_lastOcCheckTick) < OC_CHECK_PERIOD_10MS) return;
  g_lastOcCheckTick = now;

  int16_t raw = 0;
  if (!csd203DriverEnsureReady() || !csd203DriverReadCurrent(raw)) return;

  const int16_t ima = (raw < 0) ? static_cast<int16_t>(-raw) : raw;
  g_currentMa = ima;

  if (!g_ocBaselineValid) {
    if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
        DSHOT_TESTER_OC_DELTA_MA) {
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
      DSHOT_TESTER_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      DSHOT_TESTER_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

}  // namespace

#if !defined(SIMU)
extern "C" void DSHOT_TESTER_TIMER_IRQHandler()
{
  if (LL_TIM_IsActiveFlag_UPDATE(DSHOT_TESTER_TIMER)) {
    LL_TIM_ClearFlag_UPDATE(DSHOT_TESTER_TIMER);

    if (!g_active) return;

    autoSweepFrame();
    const uint16_t value = nextOutputValue();
    g_outputValue = value;
    const uint16_t packet = dshotEncode(value, false);

    if (g_powerOn && !g_overcurrentFault) {
      sendDshotFrame(packet, g_protocol);
    } else {
      gpio_clear(SERVO_TESTER_PWM_GPIO);
    }
  }
}
#endif

bool v15DshotTesterIsActive(void) { return g_active; }
bool v15DshotTesterIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15DshotTesterOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15DshotTesterGetCurrentMa(void) { return g_currentMa; }

void v15DshotTesterStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive()) return;

  takeOverAuxPort();

  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_OUT, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_clear(SERVO_TESTER_PWM_GPIO);

  g_armed = false;
  g_throttlePct = 0;
  g_cmdFramesLeft = 0;
  g_cmdValue = 0;
  g_outputValue = 0;
  g_autoDir = 1;
  g_autoFrameDiv = 0;
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
  frameTimerStart();
  setPortPower(true);
}

void v15DshotTesterStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_armed = false;
  g_throttlePct = 0;
  g_cmdFramesLeft = 0;
  g_overcurrentFault = false;
  frameTimerStop();

  setExtSignalPath(false);
  setPortPower(false);
  restoreAuxPort();
  setPortPower(false);

#if defined(MODULE_BATTERY_SENSOR)
  v15BatterySensorResumeAfterExtPort();
#endif
}

void v15DshotTesterSetProtocol(uint8_t proto)
{
  if (proto >= DSHOT_TESTER_PROTO_COUNT) proto = DSHOT_TESTER_PROTO_300;
  g_protocol = proto;
}

uint8_t v15DshotTesterGetProtocol(void) { return g_protocol; }

void v15DshotTesterSetMode(uint8_t mode)
{
  if (mode > DSHOT_TESTER_MODE_STICK) mode = DSHOT_TESTER_MODE_MANUAL;
  g_mode = mode;
  g_autoDir = 1;
}

uint8_t v15DshotTesterGetMode(void) { return g_mode; }

void v15DshotTesterSetArmed(bool armed)
{
  g_armed = armed;
  if (!armed) {
    g_throttlePct = 0;
    g_cmdFramesLeft = 0;
  }
}

bool v15DshotTesterIsArmed(void) { return g_armed; }

void v15DshotTesterSetThrottlePercent(uint8_t pct)
{
  g_throttlePct = clampPct(pct);
}

uint8_t v15DshotTesterGetThrottlePercent(void) { return g_throttlePct; }

uint16_t v15DshotTesterGetOutputValue(void) { return g_outputValue; }

void v15DshotTesterSendCommand(uint16_t cmd)
{
  if (cmd == 0 || cmd > 47) return;
  if (!g_active || g_overcurrentFault || !g_powerOn) return;
  g_cmdValue = cmd;
  g_cmdFramesLeft = CMD_BURST_FRAMES;
}

void v15DshotTesterSetAutoMaxPercent(uint8_t pct)
{
  if (pct < 1) pct = 1;
  if (pct > 50) pct = 50;  // hard safety cap for auto sweep
  g_autoMaxPct = pct;
}

uint8_t v15DshotTesterGetAutoMaxPercent(void) { return g_autoMaxPct; }

void v15DshotTesterTask(void)
{
  if (!g_active) return;
  checkOvercurrent();
}

void v15DshotTesterMixerHook(void)
{
  if (!g_active) return;
  if (g_overcurrentFault || !g_powerOn) return;
  if (g_mode != DSHOT_TESTER_MODE_STICK) return;
  if (!g_armed) return;

  // Throttle stick only (stick index 2 on Mode 2 = throttle)
  const int32_t thr = getValue(MIXSRC_FIRST_STICK + 2);
  int32_t pct = ((thr + RESX) * 100) / (2 * RESX);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  g_throttlePct = (uint8_t)pct;
}

#else  // !RADIO_V15

static uint8_t g_mode = DSHOT_TESTER_MODE_MANUAL;
static uint8_t g_protocol = DSHOT_TESTER_PROTO_300;
static uint8_t g_throttlePct = 0;
static bool g_armed = false;

bool v15DshotTesterIsActive(void) { return false; }
bool v15DshotTesterIsPowerOn(void) { return false; }
bool v15DshotTesterOvercurrentFault(void) { return false; }
int16_t v15DshotTesterGetCurrentMa(void) { return 0; }
void v15DshotTesterStart(void) {}
void v15DshotTesterStop(void) {}
void v15DshotTesterSetProtocol(uint8_t p) { g_protocol = p; }
uint8_t v15DshotTesterGetProtocol(void) { return g_protocol; }
void v15DshotTesterSetMode(uint8_t m) { g_mode = m; }
uint8_t v15DshotTesterGetMode(void) { return g_mode; }
void v15DshotTesterSetArmed(bool b) { g_armed = b; }
bool v15DshotTesterIsArmed(void) { return g_armed; }
void v15DshotTesterSetThrottlePercent(uint8_t v) { g_throttlePct = v; }
uint8_t v15DshotTesterGetThrottlePercent(void) { return g_throttlePct; }
uint16_t v15DshotTesterGetOutputValue(void) { return 0; }
void v15DshotTesterSendCommand(uint16_t) {}
void v15DshotTesterSetAutoMaxPercent(uint8_t) {}
uint8_t v15DshotTesterGetAutoMaxPercent(void) { return 15; }
void v15DshotTesterTask(void) {}
void v15DshotTesterMixerHook(void) {}

#endif  // RADIO_V15
