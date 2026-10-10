/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Auto Measure state machine for V15 EXT port.
 */

#include "auto_measure.h"
#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"
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
#include "logic_measure.h"
#include "stm32_gpio.h"
#include "timers_driver.h"

#include <stdio.h>
#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t PROBE_WAIT_MS_10 = 500;    // wait up to 5 s for first edges
constexpr tmr10ms_t PROBE_SAMPLE_MS_10 = 6;    // ~60 ms pulse-width sample window
constexpr tmr10ms_t TRY_LOCK_MS_10 = 40;      // 400 ms wait for child LOCK
constexpr tmr10ms_t TRY_SERIAL_MS_10 = 55;    // 550 ms per UART proto (IDLE + short polarity)
constexpr tmr10ms_t TRY_STABLE_MS_10 = 8;     // 80 ms stable lock before commit
constexpr tmr10ms_t LOST_MS_10 = 40;           // 400 ms without signal in LOCKED (UI attached)
constexpr tmr10ms_t HANDOFF_LOST_MS_10 = 100;  // 1.0 s without signal in child UI -> Auto search
constexpr tmr10ms_t LINK_HOLD_MS_10 = 15;      // brief settle after power-on
constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;
constexpr uint8_t EDGE_CAP = 96;
constexpr uint8_t PROBE_MIN_EDGES = 4;         // activity before starting sample
constexpr uint32_t PROBE_EDGE_STORM = 200;     // stop EXTI early (protect WDT)

volatile bool g_session = false;
volatile bool g_startingChild = false;
volatile bool g_probing = false;
volatile bool g_sampleActive = false;
volatile bool g_probeDone = false;

uint8_t g_state = AUTO_MEASURE_STATE_LINK;
uint8_t g_proto = AUTO_MEASURE_PROTO_NONE;
uint8_t g_tryProto = AUTO_MEASURE_PROTO_NONE;
uint8_t g_tryOrderIdx = 0;

tmr10ms_t g_stateSince = 0;
tmr10ms_t g_lostSince = 0;
tmr10ms_t g_sampleSince = 0;
bool g_lostTiming = false;
bool g_sampleStarted = false;

char g_status[48] = "Connecting...";

// ---- pulse probe capture ----
volatile uint16_t g_highUs[EDGE_CAP];
volatile uint16_t g_periodUs[EDGE_CAP];
volatile uint8_t g_highN = 0;
volatile uint8_t g_periodN = 0;
volatile bool g_awaitFall = false;
volatile bool g_haveRise = false;
volatile uint32_t g_riseUs = 0;
volatile uint32_t g_lastRiseUs = 0;
volatile uint32_t g_edgeCount = 0;

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
bool g_uiAttached = false;
bool g_handoff = false;
bool g_tryLockPending = false;
tmr10ms_t g_tryLockSince = 0;

// Preferred try order after pulse classify (filled by classify).
uint8_t g_tryList[12];
uint8_t g_tryListN = 0;

void setStatus(const char* s)
{
  strncpy(g_status, s, sizeof(g_status) - 1);
  g_status[sizeof(g_status) - 1] = '\0';
}

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
  if (g_savedAuxMode != UART_MODE_NONE) serialInit(SP_AUX1, UART_MODE_NONE);
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
  g_probing = false;
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
  setExtSignalPath(false);
  setPortPower(false);
  setStatus("FAULT - overcurrent");
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
    if ((int32_t)ima - (int32_t)g_lastCurrentMa >= 1000) {
      tripOvercurrent();
      return;
    }
    g_lastCurrentMa = ima;
    if ((int32_t)(now - g_ocSettleUntil) >= 0) {
      g_ocBaselineMa = ima;
      g_ocBaselineValid = true;
    }
    return;
  }
  if ((int32_t)ima - (int32_t)g_lastCurrentMa >= 1000 ||
      (int32_t)ima - (int32_t)g_ocBaselineMa >= 1000) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

void stopChildMeasure()
{
  if (v15PwmMeasureIsActive()) v15PwmMeasureStop();
  if (v15PpmMeasureIsActive()) v15PpmMeasureStop();
  if (v15SbusMeasureIsActive()) v15SbusMeasureStop();
  if (v15CrsfMeasureIsActive()) v15CrsfMeasureStop();
  if (v15SumdMeasureIsActive()) v15SumdMeasureStop();
  if (v15DjiRsMeasureIsActive()) v15DjiRsMeasureStop();
  if (v15MavlinkMeasureIsActive()) v15MavlinkMeasureStop();
  if (v15LogicMeasureIsActive()) v15LogicMeasureStop();
  g_proto = AUTO_MEASURE_PROTO_NONE;
  g_tryProto = AUTO_MEASURE_PROTO_NONE;

  if (g_session && !g_overcurrentFault) {
    g_auxTakenOver = false;
    takeOverAuxPort();
    if (g_powerOn) {
      v15ExtPortEnsureSignalPathOpen();
      setPortPower(true);
    } else if (!v15ExtPortTryOpenSignalPath()) {
      g_powerOn = false;
      setPortPower(false);
      setStatus(v15ExtPortBlockedMessage());
      return;
    } else {
      g_powerOn = true;
      setPortPower(true);
    }
  }
}

void startChild(uint8_t proto)
{
  stopChildMeasure();
  g_startingChild = true;
  switch (proto) {
    case AUTO_MEASURE_PROTO_PWM: v15PwmMeasureStart(); break;
    case AUTO_MEASURE_PROTO_PPM: v15PpmMeasureStart(); break;
    case AUTO_MEASURE_PROTO_SBUS: v15SbusMeasureStart(); break;
    case AUTO_MEASURE_PROTO_CRSF: v15CrsfMeasureStart(); break;
    case AUTO_MEASURE_PROTO_SUMD: v15SumdMeasureStart(); break;
    case AUTO_MEASURE_PROTO_DJI_RS: v15DjiRsMeasureStart(); break;
    case AUTO_MEASURE_PROTO_MAVLINK: v15MavlinkMeasureStart(); break;
    default: break;
  }
  g_startingChild = false;
  g_tryProto = proto;
}

bool childHasSignal(uint8_t proto)
{
  switch (proto) {
    case AUTO_MEASURE_PROTO_PWM: {
      // Reject UART/CRSF bit edges that briefly look like a servo pulse.
      if (!v15PwmMeasureHasSignal()) return false;
      const uint16_t pulse = v15PwmMeasureGetPulseUs();
      const uint16_t period = v15PwmMeasureGetPeriodUs();
      return pulse >= 900 && pulse <= 2100 && period >= 8000 && period <= 25000;
    }
    case AUTO_MEASURE_PROTO_PPM: {
      // CRSF/UART noise can publish 1-2 fake "channels". Require a real
      // multi-channel CPPM frame before Auto locks PPM.
      if (!v15PpmMeasureHasSignal()) return false;
      return v15PpmMeasureGetChannelCount() >= 6;
    }
    case AUTO_MEASURE_PROTO_SBUS: return v15SbusMeasureHasSignal();
    case AUTO_MEASURE_PROTO_CRSF: return v15CrsfMeasureHasSignal();
    case AUTO_MEASURE_PROTO_SUMD: return v15SumdMeasureHasSignal();
    case AUTO_MEASURE_PROTO_DJI_RS: return v15DjiRsMeasureHasSignal();
    case AUTO_MEASURE_PROTO_MAVLINK: return v15MavlinkMeasureHasSignal();
    default: return false;
  }
}

void childTask(uint8_t proto)
{
  switch (proto) {
    case AUTO_MEASURE_PROTO_PWM: v15PwmMeasureTask(); break;
    case AUTO_MEASURE_PROTO_PPM: v15PpmMeasureTask(); break;
    case AUTO_MEASURE_PROTO_SBUS: v15SbusMeasureTask(); break;
    case AUTO_MEASURE_PROTO_CRSF: v15CrsfMeasureTask(); break;
    case AUTO_MEASURE_PROTO_SUMD: v15SumdMeasureTask(); break;
    case AUTO_MEASURE_PROTO_DJI_RS: v15DjiRsMeasureTask(); break;
    case AUTO_MEASURE_PROTO_MAVLINK: v15MavlinkMeasureTask(); break;
    default: break;
  }
}

const char* protoName(uint8_t p)
{
  switch (p) {
    case AUTO_MEASURE_PROTO_PWM: return "PWM";
    case AUTO_MEASURE_PROTO_PPM: return "PPM";
    case AUTO_MEASURE_PROTO_SBUS: return "SBUS";
    case AUTO_MEASURE_PROTO_CRSF: return "CRSF";
    case AUTO_MEASURE_PROTO_SUMD: return "SUMD";
    case AUTO_MEASURE_PROTO_DJI_RS: return "DJI RS";
    case AUTO_MEASURE_PROTO_MAVLINK: return "MAVLink";
    default: return "-";
  }
}

void probeFinishFromIsr()
{
  g_probeDone = true;
  g_probing = false;
  g_sampleActive = false;
  // Dense UART/DShot would otherwise keep EXTI starving the watchdog.
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
}

void pushHigh(uint16_t w)
{
  if (g_highN < EDGE_CAP) g_highUs[g_highN++] = w;
}

void pushPeriod(uint16_t p)
{
  if (g_periodN < EDGE_CAP) g_periodUs[g_periodN++] = p;
}

void probeEdgeIsr()
{
  if (!g_probing || g_overcurrentFault || g_probeDone) return;
  const uint32_t now = timersGetUsTick();
  g_edgeCount++;

  // Activity detect only - Task arms sampling after a few edges.
  if (!g_sampleActive) {
    if (g_edgeCount >= PROBE_EDGE_STORM) probeFinishFromIsr();
    return;
  }

  if (gpio_read(SERVO_TESTER_PWM_GPIO)) {
    if (g_haveRise) {
      const uint32_t period = now - g_lastRiseUs;
      if (period > 0 && period < 100000u) pushPeriod((uint16_t)period);
    }
    g_riseUs = now;
    g_lastRiseUs = now;
    g_haveRise = true;
    g_awaitFall = true;
  } else if (g_awaitFall) {
    const uint32_t width = now - g_riseUs;
    if (width > 0 && width < 60000u) pushHigh((uint16_t)width);
    g_awaitFall = false;
  }

  if (g_highN >= EDGE_CAP || g_edgeCount >= PROBE_EDGE_STORM) {
    probeFinishFromIsr();
  }
}

void stopProbe()
{
  g_probing = false;
  g_sampleActive = false;
  g_probeDone = false;
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
}

void startProbe()
{
  g_highN = 0;
  g_periodN = 0;
  g_awaitFall = false;
  g_haveRise = false;
  g_riseUs = 0;
  g_lastRiseUs = 0;
  g_edgeCount = 0;
  g_sampleActive = false;
  g_probeDone = false;
  g_sampleStarted = false;
  g_probing = true;
  gpio_init_int(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_BOTH, probeEdgeIsr);
  setStatus("Waiting for pulses...");
}

void beginProbeSample()
{
  // Restart a clean ~60 ms width capture after activity was seen.
  gpio_int_disable(SERVO_TESTER_PWM_GPIO);
  g_highN = 0;
  g_periodN = 0;
  g_awaitFall = false;
  g_haveRise = false;
  g_riseUs = 0;
  g_lastRiseUs = 0;
  g_edgeCount = 0;
  g_probeDone = false;
  g_sampleSince = get_tmr10ms();
  g_sampleStarted = true;
  g_sampleActive = true;
  g_probing = true;
  gpio_init_int(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_BOTH, probeEdgeIsr);
  setStatus("Sampling widths ~60ms...");
}

uint16_t medianCopy(volatile uint16_t* src, uint8_t n)
{
  if (n == 0) return 0;
  uint16_t tmp[EDGE_CAP];
  uint8_t m = n;
  if (m > EDGE_CAP) m = EDGE_CAP;
  for (uint8_t i = 0; i < m; ++i) tmp[i] = src[i];
  for (uint8_t i = 1; i < m; ++i) {
    uint16_t key = tmp[i];
    int j = (int)i - 1;
    while (j >= 0 && tmp[j] > key) {
      tmp[j + 1] = tmp[j];
      --j;
    }
    tmp[j + 1] = key;
  }
  return tmp[m / 2];
}

uint8_t countInRange(volatile uint16_t* src, uint8_t n, uint16_t lo, uint16_t hi)
{
  uint8_t c = 0;
  for (uint8_t i = 0; i < n; ++i) {
    const uint16_t v = src[i];
    if (v >= lo && v <= hi) ++c;
  }
  return c;
}

void pushTry(uint8_t proto)
{
  for (uint8_t i = 0; i < g_tryListN; ++i) {
    if (g_tryList[i] == proto) return;
  }
  if (g_tryListN < 12) g_tryList[g_tryListN++] = proto;
}

void pushUartFamily()
{
  pushTry(AUTO_MEASURE_PROTO_CRSF);
  // SBUS before DJI: same wire framing; DJI is detected via channel scaling
  // after SBUS locks (see TRY state).
  pushTry(AUTO_MEASURE_PROTO_SBUS);
  pushTry(AUTO_MEASURE_PROTO_SUMD);
  pushTry(AUTO_MEASURE_PROTO_DJI_RS);
  pushTry(AUTO_MEASURE_PROTO_MAVLINK);
}

void classifyFromProbe()
{
  g_tryListN = 0;
  const uint16_t medHigh = medianCopy(g_highUs, g_highN);
  const uint16_t medPeriod = medianCopy(g_periodUs, g_periodN);
  const uint8_t uartishBits = countInRange(g_highUs, g_highN, 1, 100);
  const uint8_t servoHighs = countInRange(g_highUs, g_highN, 800, 2200);
  const uint8_t longGaps = countInRange(g_periodUs, g_periodN, 4000, 50000);
  const uint32_t edges = g_edgeCount;
  const uint8_t n = (g_highN > 0) ? g_highN : 1;

  // Classification from ~60 ms pulse-width sample (not a fixed try order).
  const bool lookUart =
      (uartishBits * 2 >= n) || (edges >= 80) ||
      (medHigh > 0 && medHigh < 200);
  const bool lookPwm =
      (servoHighs * 2 >= n) && (medHigh >= 800) && (medHigh <= 2200) &&
      (medPeriod >= 8000) && (medPeriod <= 25000) && (edges < 60);
  const bool lookPpm =
      (servoHighs * 2 >= n) && (longGaps >= 2) && (edges >= 8) && (edges < 60) &&
      !lookUart;

  if (lookUart && !lookPwm && !lookPpm) {
    pushUartFamily();
  } else if (lookPwm) {
    pushTry(AUTO_MEASURE_PROTO_PWM);
    if (lookPpm) pushTry(AUTO_MEASURE_PROTO_PPM);
    pushUartFamily();
  } else if (lookPpm) {
    pushTry(AUTO_MEASURE_PROTO_PPM);
    pushTry(AUTO_MEASURE_PROTO_PWM);
    pushUartFamily();
  } else if (medHigh >= 800 && medHigh <= 2200) {
    pushTry(AUTO_MEASURE_PROTO_PWM);
    pushTry(AUTO_MEASURE_PROTO_PPM);
    pushUartFamily();
  } else {
    if (medHigh > 0 && medHigh < 200) pushUartFamily();
    pushTry(AUTO_MEASURE_PROTO_PWM);
    pushTry(AUTO_MEASURE_PROTO_PPM);
    pushUartFamily();
  }

  char buf[48];
  snprintf(buf, sizeof(buf), "H=%uus P=%uus n=%u", (unsigned)medHigh,
           (unsigned)medPeriod, (unsigned)g_highN);
  setStatus(buf);
}

void enterLink()
{
  stopProbe();
  stopChildMeasure();
  g_state = AUTO_MEASURE_STATE_LINK;
  g_stateSince = get_tmr10ms();
  g_lostTiming = false;
  g_tryLockPending = false;
  g_sampleStarted = false;
  g_proto = AUTO_MEASURE_PROTO_NONE;
  g_tryOrderIdx = 0;
  g_tryListN = 0;

  if (!g_overcurrentFault) {
    if (!g_powerOn) {
      if (!v15ExtPortTryOpenSignalPath()) {
        g_powerOn = false;
        setPortPower(false);
        setStatus(v15ExtPortBlockedMessage());
        return;
      }
      g_powerOn = true;
      setPortPower(true);
      g_ocSettleUntil = get_tmr10ms() + OC_SETTLE_10MS;
      g_ocBaselineValid = false;
    } else {
      v15ExtPortEnsureSignalPathOpen();
      setPortPower(true);
    }
  }
  setStatus("Connecting - EXT power ON");
}

void enterProbe()
{
  stopChildMeasure();
  g_state = AUTO_MEASURE_STATE_PROBE;
  g_stateSince = get_tmr10ms();
  g_sampleStarted = false;
  startProbe();
}

void enterTryNext()
{
  stopProbe();
  g_tryLockPending = false;
  if (g_tryOrderIdx >= g_tryListN) {
    enterLink();
    return;
  }
  const uint8_t proto = g_tryList[g_tryOrderIdx++];
  g_state = AUTO_MEASURE_STATE_TRY;
  g_stateSince = get_tmr10ms();
  startChild(proto);
  char buf[48];
  snprintf(buf, sizeof(buf), "Trying %s...", protoName(proto));
  setStatus(buf);
}

void enterLocked(uint8_t proto)
{
  g_state = AUTO_MEASURE_STATE_LOCKED;
  g_proto = proto;
  g_tryProto = proto;
  g_stateSince = get_tmr10ms();
  g_lostTiming = false;
  g_tryLockPending = false;
  char buf[48];
  snprintf(buf, sizeof(buf), "LOCKED ÃÂ· %s", protoName(proto));
  setStatus(buf);
}

}  // namespace

bool v15AutoMeasureIsActive(void) { return g_session; }
bool v15AutoMeasureIsStartingChild(void) { return g_startingChild; }
bool v15AutoMeasureIsHandoff(void) { return g_handoff; }
bool v15AutoMeasureIsUiAttached(void) { return g_uiAttached; }
uint8_t v15AutoMeasureGetState(void) { return g_state; }
uint8_t v15AutoMeasureGetProtocol(void) { return g_proto; }
uint8_t v15AutoMeasureGetTryProtocol(void) { return g_tryProto; }
const char* v15AutoMeasureGetStatusText(void) { return g_status; }
bool v15AutoMeasureOvercurrentFault(void) { return g_overcurrentFault; }
bool v15AutoMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
int16_t v15AutoMeasureGetCurrentMa(void)
{
  if (g_state == AUTO_MEASURE_STATE_LOCKED || g_state == AUTO_MEASURE_STATE_TRY) {
    switch (g_tryProto) {
      case AUTO_MEASURE_PROTO_PWM: return v15PwmMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_PPM: return v15PpmMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_SBUS: return v15SbusMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_CRSF: return v15CrsfMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_SUMD: return v15SumdMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_DJI_RS: return v15DjiRsMeasureGetCurrentMa();
      case AUTO_MEASURE_PROTO_MAVLINK: return v15MavlinkMeasureGetCurrentMa();
      default: break;
    }
  }
  return g_currentMa;
}

void v15AutoMeasureStart(void)
{
  if (g_session) return;
  if (v15ServoTesterIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;

  takeOverAuxPort();
  g_uiAttached = true;
  g_handoff = false;
  g_tryLockPending = false;
  g_overcurrentFault = false;
  g_ocBaselineValid = false;
  g_currentMa = 0;
  g_lastCurrentMa = 0;
  g_lastOcCheckTick = 0;
  g_powerOn = false;
  // Probe + open before claiming session; fail closed if jack > 5 V.
  g_session = true;
  enterLink();
  if (!g_powerOn) {
    g_session = false;
    g_uiAttached = false;
    setExtSignalPath(false);
    setPortPower(false);
    restoreAuxPort();
#if defined(MODULE_BATTERY_SENSOR)
    v15BatterySensorResumeAfterExtPort();
#endif
  }
}

void v15AutoMeasureStop(void)
{
  if (!g_session) return;
  // Clear session first so stopChildMeasure() does not reclaim the port.
  g_session = false;
  g_uiAttached = false;
  g_handoff = false;
  g_tryLockPending = false;
  stopProbe();
  stopChildMeasure();
  g_powerOn = false;
  g_probing = false;
  g_overcurrentFault = false;
  g_state = AUTO_MEASURE_STATE_LINK;
  g_proto = AUTO_MEASURE_PROTO_NONE;

  setExtSignalPath(false);
  setPortPower(false);
  restoreAuxPort();
  rotaryEncoderInit();

#if defined(MODULE_BATTERY_SENSOR)
  v15BatterySensorResumeAfterExtPort();
#endif
}

void v15AutoMeasureBeginHandoff(void)
{
  g_uiAttached = false;
  g_handoff = true;
  g_lostTiming = false;
}

void v15AutoMeasureReattachUi(void)
{
  if (!g_session) return;
  g_uiAttached = true;
  g_handoff = false;
}

void v15AutoMeasureResumeSearch(void)
{
  if (!g_session) return;
  g_handoff = false;
  g_uiAttached = false;
  g_tryLockPending = false;
  g_lostTiming = false;
  g_proto = AUTO_MEASURE_PROTO_NONE;
  g_tryProto = AUTO_MEASURE_PROTO_NONE;
  takeOverAuxPort();
  enterLink();
  if (!g_powerOn) {
    setStatus(v15ExtPortBlockedMessage());
    return;
  }
  setStatus("Signal lost - searching...");
}

bool v15AutoMeasureWatchHandoffLoss(bool hasSignal)
{
  if (!g_session || !g_handoff) return false;
  const tmr10ms_t now = get_tmr10ms();
  if (hasSignal) {
    g_lostTiming = false;
    return false;
  }
  if (!g_lostTiming) {
    g_lostTiming = true;
    g_lostSince = now;
    return false;
  }
  // Longer than in-page LOCKED timeout so brief glitches / cable swaps settle.
  return (tmr10ms_t)(now - g_lostSince) >= HANDOFF_LOST_MS_10;
}

void v15AutoMeasureTask(void)
{
  if (!g_session || !g_uiAttached) return;

  checkOvercurrent();
  if (g_overcurrentFault) {
    stopProbe();
    stopChildMeasure();
    return;
  }

  const tmr10ms_t now = get_tmr10ms();

  switch (g_state) {
    case AUTO_MEASURE_STATE_LINK:
      if ((tmr10ms_t)(now - g_stateSince) >= LINK_HOLD_MS_10) {
        enterProbe();
      }
      break;

    case AUTO_MEASURE_STATE_PROBE:
      if (!g_sampleStarted) {
        if (g_edgeCount >= PROBE_MIN_EDGES || g_probeDone) {
          beginProbeSample();
        } else if ((tmr10ms_t)(now - g_stateSince) >= PROBE_WAIT_MS_10) {
          g_stateSince = now;
          setStatus("Waiting for pulses...");
        }
      } else if (g_probeDone ||
                 (tmr10ms_t)(now - g_sampleSince) >= PROBE_SAMPLE_MS_10) {
        stopProbe();
        if (g_highN == 0 && g_periodN == 0) {
          enterProbe();
        } else {
          classifyFromProbe();
          g_tryOrderIdx = 0;
          enterTryNext();
        }
      }
      break;

    case AUTO_MEASURE_STATE_TRY:
      childTask(g_tryProto);
      if (childHasSignal(g_tryProto)) {
        if (!g_tryLockPending) {
          g_tryLockPending = true;
          g_tryLockSince = now;
        } else if ((tmr10ms_t)(now - g_tryLockSince) >= TRY_STABLE_MS_10) {
          // DJI RS Pro is SBUS framing with ÃÂµs packed in the 11-bit slots.
          // SBUS locks first; reclassify before handoff.
          if (g_tryProto == AUTO_MEASURE_PROTO_SBUS &&
              v15SbusMeasureLooksLikeDjiRs()) {
            stopChildMeasure();
            g_startingChild = true;
            v15DjiRsMeasureStart();
            g_startingChild = false;
            g_tryProto = AUTO_MEASURE_PROTO_DJI_RS;
            enterLocked(AUTO_MEASURE_PROTO_DJI_RS);
          } else {
            enterLocked(g_tryProto);
          }
        }
      } else {
        g_tryLockPending = false;
        const tmr10ms_t tryLimit =
            (g_tryProto == AUTO_MEASURE_PROTO_CRSF ||
             g_tryProto == AUTO_MEASURE_PROTO_SBUS ||
             g_tryProto == AUTO_MEASURE_PROTO_SUMD ||
             g_tryProto == AUTO_MEASURE_PROTO_DJI_RS ||
             g_tryProto == AUTO_MEASURE_PROTO_MAVLINK)
                ? TRY_SERIAL_MS_10
                : TRY_LOCK_MS_10;
        if ((tmr10ms_t)(now - g_stateSince) >= tryLimit) {
          enterTryNext();
        }
      }
      break;

    case AUTO_MEASURE_STATE_LOCKED:
      childTask(g_proto);
      if (childHasSignal(g_proto)) {
        g_lostTiming = false;
      } else {
        if (!g_lostTiming) {
          g_lostTiming = true;
          g_lostSince = now;
          setStatus("Signal lost - re-linking...");
        } else if ((tmr10ms_t)(now - g_lostSince) >= LOST_MS_10) {
          enterLink();
        }
      }
      break;

    default:
      enterLink();
      break;
  }
}

#else  // SIMU / non-V15

bool v15AutoMeasureIsActive(void) { return false; }
bool v15AutoMeasureIsStartingChild(void) { return false; }
bool v15AutoMeasureIsHandoff(void) { return false; }
bool v15AutoMeasureIsUiAttached(void) { return false; }
uint8_t v15AutoMeasureGetState(void) { return AUTO_MEASURE_STATE_LINK; }
uint8_t v15AutoMeasureGetProtocol(void) { return AUTO_MEASURE_PROTO_NONE; }
uint8_t v15AutoMeasureGetTryProtocol(void) { return AUTO_MEASURE_PROTO_NONE; }
const char* v15AutoMeasureGetStatusText(void) { return "N/A"; }
bool v15AutoMeasureOvercurrentFault(void) { return false; }
bool v15AutoMeasureIsPowerOn(void) { return false; }
int16_t v15AutoMeasureGetCurrentMa(void) { return 0; }
void v15AutoMeasureStart(void) {}
void v15AutoMeasureStop(void) {}
void v15AutoMeasureBeginHandoff(void) {}
void v15AutoMeasureReattachUi(void) {}
void v15AutoMeasureResumeSearch(void) {}
bool v15AutoMeasureWatchHandoffLoss(bool) { return false; }
void v15AutoMeasureTask(void) {}

#endif
