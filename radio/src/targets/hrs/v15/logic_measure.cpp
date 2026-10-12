/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Pulse Scope on V15 J10 (PJ8): EXTI both-edge timestamps → ring buffer.
 */

#include "logic_measure.h"
#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"
#include "hal/watchdog_driver.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "auto_measure.h"
#include "stm32_gpio.h"
#include "timers_driver.h"

#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;
constexpr uint16_t DELTA_SAT_US = 60000;
constexpr uint16_t RATE_LIMIT_EDGES = 150;     // per 1 ms
constexpr uint32_t RATE_WINDOW_US = 1000;
constexpr tmr10ms_t RATE_PAUSE_10MS = 3;       // 30 ms cool-down

constexpr uint32_t kTimebaseUs[LOGIC_MEASURE_TB_COUNT] = {
    100u, 500u, 1000u, 2000u, 5000u, 10000u, 20000u, 50000u,
};

struct RingEdge {
  uint16_t deltaUs;
  uint8_t level;
};

volatile bool g_active = false;
volatile uint8_t g_mode = LOGIC_MEASURE_MODE_RUN;
volatile uint8_t g_timebase = LOGIC_MEASURE_TB_5MS;

volatile RingEdge g_ring[LOGIC_MEASURE_RING];
volatile uint16_t g_head = 0;
volatile uint16_t g_count = 0;
volatile uint32_t g_lastEdgeUs = 0;
volatile bool g_haveLastEdge = false;

volatile bool g_singleArmed = false;
volatile bool g_singleCapturing = false;
volatile uint32_t g_singleAccumUs = 0;

volatile uint32_t g_edgeCountWindow = 0;
volatile tmr10ms_t g_hzWindowStart = 0;
uint32_t g_edgesHz = 0;

volatile uint16_t g_lastHighUs = 0;
volatile uint16_t g_lastLowUs = 0;
volatile uint8_t g_lastLevel = 0;
volatile bool g_haveSignal = false;
volatile tmr10ms_t g_lastEdge10ms = 0;

volatile uint16_t g_rateEdges = 0;
volatile uint32_t g_rateWinStartUs = 0;
volatile bool g_rateLimited = false;
volatile bool g_extiPaused = false;
tmr10ms_t g_extiPauseUntil = 0;

// Frozen snapshot for Stop / completed Single
RingEdge g_snap[LOGIC_MEASURE_RING];
uint16_t g_snapCount = 0;
uint8_t g_snapStartLevel = 0;
bool g_useSnap = false;

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
  if (connect) gpio_set(CHIP_FUN_GPIO);
  else gpio_clear(CHIP_FUN_GPIO);
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
  if (on) gpio_set(SERVO_TESTER_PWR_GPIO);
  else gpio_clear(SERVO_TESTER_PWR_GPIO);
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

void tripOvercurrent()
{
  if (g_overcurrentFault) return;
  g_overcurrentFault = true;
  g_powerOn = false;
  g_haveSignal = false;
  g_mode = LOGIC_MEASURE_MODE_STOP;
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
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
        LOGIC_MEASURE_OC_DELTA_MA) {
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
          LOGIC_MEASURE_OC_DELTA_MA ||
      static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
          LOGIC_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

void logicEdgeIsr();
volatile bool g_freezePending = false;

void enableExti()
{
  g_extiPaused = false;
  // PJ8 EXTI8 steals rotary B (PH8); poll encoder until Init restores EXTI.
  gpio_init_int(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_BOTH, logicEdgeIsr);
  rotaryEncoderStartPolling();
}

void disableExti()
{
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
}

void clearRing()
{
  g_head = 0;
  g_count = 0;
  g_haveLastEdge = false;
  g_lastEdgeUs = 0;
  g_singleAccumUs = 0;
}

void freezeFromRing()
{
  const uint16_t n = g_count;
  const uint16_t head = g_head;
  g_snapCount = n;
  for (uint16_t i = 0; i < n; ++i) {
    const uint16_t idx =
        (uint16_t)((head + LOGIC_MEASURE_RING - n + i) % LOGIC_MEASURE_RING);
    g_snap[i].deltaUs = g_ring[idx].deltaUs;
    g_snap[i].level = g_ring[idx].level;
  }
  if (n == 0) {
    g_snapStartLevel = gpio_read(SERVO_TESTER_PWM_GPIO) ? 1 : 0;
  } else {
    g_snapStartLevel = g_snap[0].level ? 0 : 1;
  }
  g_useSnap = true;
  g_freezePending = false;
}

void logicEdgeIsr()
{
  if (!g_active || g_overcurrentFault || !g_powerOn || g_extiPaused) return;
  if (g_mode == LOGIC_MEASURE_MODE_STOP) return;
  if (g_mode == LOGIC_MEASURE_MODE_SINGLE && !g_singleArmed && !g_singleCapturing)
    return;

  const uint32_t now = timersGetUsTick();
  const uint8_t level = gpio_read(SERVO_TESTER_PWM_GPIO) ? 1 : 0;

  // Rate limit window
  if (g_rateWinStartUs == 0 || (now - g_rateWinStartUs) > RATE_WINDOW_US) {
    g_rateWinStartUs = now;
    g_rateEdges = 0;
  }
  ++g_rateEdges;
  if (g_rateEdges > RATE_LIMIT_EDGES) {
    g_rateLimited = true;
    g_extiPaused = true;
    g_extiPauseUntil = get_tmr10ms();
    gpio_int_disable(SERVO_TESTER_PWM_GPIO);
    return;
  }

  uint16_t delta = 0;
  if (g_haveLastEdge) {
    uint32_t d = now - g_lastEdgeUs;
    if (d > DELTA_SAT_US) d = DELTA_SAT_US;
    delta = (uint16_t)d;
  }

  if (g_mode == LOGIC_MEASURE_MODE_SINGLE) {
    if (!g_singleCapturing) {
      g_singleCapturing = true;
      g_singleArmed = false;
      clearRing();
      delta = 0;
      g_singleAccumUs = 0;
    } else {
      g_singleAccumUs += delta;
    }
  }

  // Pulse width stats from previous segment
  if (g_haveLastEdge && delta > 0) {
    if (g_lastLevel) g_lastHighUs = delta;
    else g_lastLowUs = delta;
  }

  g_ring[g_head].deltaUs = delta;
  g_ring[g_head].level = level;
  g_head = (uint16_t)((g_head + 1) % LOGIC_MEASURE_RING);
  if (g_count < LOGIC_MEASURE_RING) ++g_count;

  g_lastEdgeUs = now;
  g_haveLastEdge = true;
  g_lastLevel = level;
  g_haveSignal = true;
  g_lastEdge10ms = get_tmr10ms();
  ++g_edgeCountWindow;

  if (g_mode == LOGIC_MEASURE_MODE_RUN) {
    g_useSnap = false;
  }

  if (g_mode == LOGIC_MEASURE_MODE_SINGLE && g_singleCapturing) {
    const uint32_t win = kTimebaseUs[g_timebase];
    if (g_singleAccumUs >= win || g_count >= LOGIC_MEASURE_RING) {
      g_mode = LOGIC_MEASURE_MODE_STOP;
      g_singleCapturing = false;
      g_singleArmed = false;
      g_freezePending = true;
      gpio_int_disable(SERVO_TESTER_PWM_GPIO);
    }
  }
}

void buildWindowFrom(const RingEdge* src, uint16_t n, uint8_t inferredStart,
                     LogicMeasureEdge* out, uint16_t maxN, uint8_t* startLevel,
                     uint16_t* outCount)
{
  *outCount = 0;
  if (n == 0 || maxN == 0) {
    *startLevel = gpio_read(SERVO_TESTER_PWM_GPIO) ? 1 : 0;
    return;
  }

  const uint32_t win = kTimebaseUs[g_timebase];

  // Walk from newest backward until time >= win
  uint32_t sum = 0;
  uint16_t startIdx = 0;
  for (int i = (int)n - 1; i >= 0; --i) {
    sum += src[i].deltaUs;
    startIdx = (uint16_t)i;
    if (sum >= win) break;
  }

  // startLevel: level before edge at startIdx
  uint8_t sl = inferredStart;
  if (startIdx > 0) {
    sl = src[startIdx - 1].level;
  } else {
    sl = src[0].level ? 0 : 1;
  }
  *startLevel = sl;

  // Re-sum from startIdx with t=0 at window left (trim leading if sum>win)
  uint32_t lead = 0;
  for (uint16_t i = startIdx; i < n; ++i) lead += src[i].deltaUs;
  uint32_t skip = (lead > win) ? (lead - win) : 0;

  uint32_t t = 0;
  uint16_t w = 0;
  bool skipping = skip > 0;
  for (uint16_t i = startIdx; i < n && w < maxN; ++i) {
    uint32_t d = src[i].deltaUs;
    if (skipping) {
      if (d <= skip) {
        skip -= d;
        continue;
      }
      d -= skip;
      skip = 0;
      skipping = false;
    }
    t += d;
    if (t > 0xFFFF) t = 0xFFFF;
    out[w].tUs = (uint16_t)t;
    out[w].level = src[i].level;
    ++w;
  }
  *outCount = w;
}

}  // namespace

bool v15LogicMeasureIsActive(void) { return g_active; }
bool v15LogicMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15LogicMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15LogicMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15LogicMeasureGetCurrentMa(void) { return g_currentMa; }

bool v15LogicMeasureGetLevel(void)
{
  if (!g_active) return false;
  return gpio_read(SERVO_TESTER_PWM_GPIO) != 0;
}

bool v15LogicMeasureIsRateLimited(void) { return g_rateLimited; }
uint32_t v15LogicMeasureGetEdgesHz(void) { return g_edgesHz; }
uint16_t v15LogicMeasureGetLastHighUs(void) { return g_lastHighUs; }
uint16_t v15LogicMeasureGetLastLowUs(void) { return g_lastLowUs; }

uint8_t v15LogicMeasureGetDutyPercent(void)
{
  const uint32_t hi = g_lastHighUs;
  const uint32_t lo = g_lastLowUs;
  const uint32_t p = hi + lo;
  if (p == 0) return 0;
  return (uint8_t)((hi * 100u + p / 2u) / p);
}

uint16_t v15LogicMeasureGetPeriodUs(void)
{
  const uint32_t p = (uint32_t)g_lastHighUs + (uint32_t)g_lastLowUs;
  if (p > 0xFFFF) return 0xFFFF;
  return (uint16_t)p;
}

void v15LogicMeasureSetTimebase(uint8_t tb)
{
  if (tb >= LOGIC_MEASURE_TB_COUNT) tb = LOGIC_MEASURE_TB_5MS;
  g_timebase = tb;
}

uint8_t v15LogicMeasureGetTimebase(void) { return g_timebase; }

uint32_t v15LogicMeasureGetTimebaseUs(void)
{
  return kTimebaseUs[g_timebase < LOGIC_MEASURE_TB_COUNT ? g_timebase
                                                         : LOGIC_MEASURE_TB_5MS];
}

void v15LogicMeasureRun(void)
{
  if (!g_active || g_overcurrentFault) return;
  g_mode = LOGIC_MEASURE_MODE_RUN;
  g_singleArmed = false;
  g_singleCapturing = false;
  g_useSnap = false;
  g_rateLimited = false;
  clearRing();
  enableExti();
}

void v15LogicMeasureStopCapture(void)
{
  if (!g_active) return;
  freezeFromRing();
  g_mode = LOGIC_MEASURE_MODE_STOP;
  g_singleArmed = false;
  g_singleCapturing = false;
  disableExti();
}

void v15LogicMeasureSingle(void)
{
  if (!g_active || g_overcurrentFault) return;
  g_mode = LOGIC_MEASURE_MODE_SINGLE;
  g_singleArmed = true;
  g_singleCapturing = false;
  g_useSnap = false;
  g_rateLimited = false;
  clearRing();
  enableExti();
}

uint8_t v15LogicMeasureGetMode(void) { return g_mode; }

uint16_t v15LogicMeasureCopyEdges(LogicMeasureEdge* out, uint16_t maxN,
                                  uint8_t* startLevel)
{
  if (!out || !startLevel || maxN == 0) return 0;

  if (g_useSnap || g_mode == LOGIC_MEASURE_MODE_STOP) {
    uint16_t n = 0;
    buildWindowFrom(g_snap, g_snapCount, g_snapStartLevel, out, maxN,
                    startLevel, &n);
    return n;
  }

  // Live: copy ring to local then window
  RingEdge tmp[LOGIC_MEASURE_RING];
  const uint16_t n = g_count;
  const uint16_t head = g_head;
  for (uint16_t i = 0; i < n; ++i) {
    const uint16_t idx =
        (uint16_t)((head + LOGIC_MEASURE_RING - n + i) % LOGIC_MEASURE_RING);
    tmp[i].deltaUs = g_ring[idx].deltaUs;
    tmp[i].level = g_ring[idx].level;
  }
  uint8_t inferred = (n > 0) ? (uint8_t)(tmp[0].level ? 0 : 1) : 0;
  uint16_t outN = 0;
  buildWindowFrom(tmp, n, inferred, out, maxN, startLevel, &outN);
  return outN;
}

void v15LogicMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15AutoMeasureIsActive() && !v15AutoMeasureIsStartingChild()) return;

  takeOverAuxPort();

  g_mode = LOGIC_MEASURE_MODE_RUN;
  g_timebase = LOGIC_MEASURE_TB_5MS;
  g_singleArmed = false;
  g_singleCapturing = false;
  g_useSnap = false;
  g_snapCount = 0;
  g_rateLimited = false;
  g_extiPaused = false;
  g_haveSignal = false;
  g_lastHighUs = 0;
  g_lastLowUs = 0;
  g_lastLevel = 0;
  g_edgesHz = 0;
  g_edgeCountWindow = 0;
  g_hzWindowStart = get_tmr10ms();
  g_lastEdge10ms = get_tmr10ms();
  clearRing();

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

  g_active = true;
  g_powerOn = true;
  setPortPower(true);
  enableExti();
}

void v15LogicMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_mode = LOGIC_MEASURE_MODE_STOP;
  g_overcurrentFault = false;
  g_singleArmed = false;
  g_singleCapturing = false;

  disableExti();
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

void v15LogicMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  WDG_RESET();

  if (g_freezePending) {
    freezeFromRing();
  }

  const tmr10ms_t now = get_tmr10ms();

  if (g_extiPaused && (tmr10ms_t)(now - g_extiPauseUntil) >= RATE_PAUSE_10MS) {
    g_extiPaused = false;
    g_rateEdges = 0;
    g_rateWinStartUs = 0;
    if (g_mode != LOGIC_MEASURE_MODE_STOP) {
      enableExti();
    }
  }

  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastEdge10ms) >= LOGIC_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
  }

  if ((tmr10ms_t)(now - g_hzWindowStart) >= 100) {
    g_edgesHz = g_edgeCountWindow;
    g_edgeCountWindow = 0;
    g_hzWindowStart = now;
  }
}

#else  // !RADIO_V15 || SIMU

bool v15LogicMeasureIsActive(void) { return false; }
bool v15LogicMeasureHasSignal(void) { return false; }
bool v15LogicMeasureIsPowerOn(void) { return false; }
bool v15LogicMeasureOvercurrentFault(void) { return false; }
int16_t v15LogicMeasureGetCurrentMa(void) { return 0; }
bool v15LogicMeasureGetLevel(void) { return false; }
bool v15LogicMeasureIsRateLimited(void) { return false; }
uint32_t v15LogicMeasureGetEdgesHz(void) { return 0; }
uint16_t v15LogicMeasureGetLastHighUs(void) { return 0; }
uint16_t v15LogicMeasureGetLastLowUs(void) { return 0; }
uint8_t v15LogicMeasureGetDutyPercent(void) { return 0; }
uint16_t v15LogicMeasureGetPeriodUs(void) { return 0; }
void v15LogicMeasureSetTimebase(uint8_t) {}
uint8_t v15LogicMeasureGetTimebase(void) { return LOGIC_MEASURE_TB_5MS; }
uint32_t v15LogicMeasureGetTimebaseUs(void) { return 5000u; }
void v15LogicMeasureRun(void) {}
void v15LogicMeasureStopCapture(void) {}
void v15LogicMeasureSingle(void) {}
uint8_t v15LogicMeasureGetMode(void) { return LOGIC_MEASURE_MODE_STOP; }
uint16_t v15LogicMeasureCopyEdges(LogicMeasureEdge*, uint16_t, uint8_t* sl)
{
  if (sl) *sl = 0;
  return 0;
}
void v15LogicMeasureStart(void) {}
void v15LogicMeasureStop(void) {}
void v15LogicMeasureTask(void) {}

#endif
