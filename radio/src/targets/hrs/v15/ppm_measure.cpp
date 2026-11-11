/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * CPPM/PPM frame measure on V15 J10 (PJ8). Rising-edge intervals
 * (trainer-style): sync gap 4ù?0 ms, channels 0.8ù?.2 ms.
 */

#include "ppm_measure.h"
#include "ext_port_safety.h"
#include "measure_trainer_feed.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"
#include "pwm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
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
/** Strong EMA + short moving average to kill ±µs edge jitter. */
constexpr uint32_t EMA_HIST_W = 9;
constexpr uint32_t EMA_NEW_W = 1;
constexpr uint32_t EMA_SUM_W = 10;
constexpr uint8_t AVG_DEPTH = 6;

volatile bool g_active = false;
volatile bool g_haveSignal = false;
volatile bool g_havePrevEdge = false;
volatile uint32_t g_lastEdgeUs = 0;
volatile uint32_t g_frameStartUs = 0;
volatile uint8_t g_decodeCh = PPM_MEASURE_MAX_CHANNELS;  // idle until sync
volatile tmr10ms_t g_lastEdge10ms = 0;

volatile uint16_t g_chDecode[PPM_MEASURE_MAX_CHANNELS] = {};
volatile uint16_t g_chPub[PPM_MEASURE_MAX_CHANNELS] = {};
volatile uint8_t g_chCountRaw = 0;
volatile uint16_t g_frameUsRaw = 0;

uint16_t g_chUs[PPM_MEASURE_MAX_CHANNELS] = {};
uint8_t g_chCount = 0;
uint16_t g_frameUs = 0;

// Last published frame for UI CPPM waveform reconstruction
uint16_t g_waveChUs[PPM_MEASURE_MAX_CHANNELS] = {};
uint8_t g_waveChCount = 0;
uint16_t g_waveFrameUs = 0;
volatile uint32_t g_waveSeq = 0;

uint16_t g_chAvgBuf[PPM_MEASURE_MAX_CHANNELS][AVG_DEPTH] = {};
uint8_t g_chAvgIdx[PPM_MEASURE_MAX_CHANNELS] = {};
uint8_t g_chAvgCount[PPM_MEASURE_MAX_CHANNELS] = {};
uint16_t g_frameAvgBuf[AVG_DEPTH] = {};
uint8_t g_frameAvgIdx = 0;
uint8_t g_frameAvgCount = 0;

uint32_t g_emaChxN[PPM_MEASURE_MAX_CHANNELS] = {};
bool g_emaChValid[PPM_MEASURE_MAX_CHANNELS] = {};
uint32_t g_emaFramexN = 0;
bool g_emaFrameValid = false;

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

void tripOvercurrent()
{
  if (g_overcurrentFault) return;

  g_overcurrentFault = true;
  g_powerOn = false;
  g_haveSignal = false;
  g_havePrevEdge = false;
  g_decodeCh = PPM_MEASURE_MAX_CHANNELS;

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
        PPM_MEASURE_OC_DELTA_MA) {
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
      PPM_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      PPM_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

uint16_t emaUpdate(uint32_t& state, bool& valid, uint16_t sample)
{
  if (!valid) {
    state = (uint32_t)sample * EMA_SUM_W;
    valid = true;
  } else {
    state = (state * EMA_HIST_W + (uint32_t)sample * EMA_SUM_W * EMA_NEW_W) /
            EMA_SUM_W;
  }
  return (uint16_t)((state + EMA_SUM_W / 2) / EMA_SUM_W);
}

uint16_t avgPush(uint16_t* buf, uint8_t& idx, uint8_t& count, uint16_t sample)
{
  buf[idx] = sample;
  idx = (uint8_t)((idx + 1) % AVG_DEPTH);
  if (count < AVG_DEPTH) ++count;

  uint32_t sum = 0;
  for (uint8_t i = 0; i < count; ++i) sum += buf[i];
  return (uint16_t)((sum + count / 2) / count);
}

void resetChannelFilter()
{
  memset(g_chAvgBuf, 0, sizeof(g_chAvgBuf));
  memset(g_chAvgIdx, 0, sizeof(g_chAvgIdx));
  memset(g_chAvgCount, 0, sizeof(g_chAvgCount));
  memset(g_frameAvgBuf, 0, sizeof(g_frameAvgBuf));
  g_frameAvgIdx = 0;
  g_frameAvgCount = 0;
  memset(g_emaChxN, 0, sizeof(g_emaChxN));
  memset(g_emaChValid, 0, sizeof(g_emaChValid));
  g_emaFramexN = 0;
  g_emaFrameValid = false;
}

void publishFrame(uint8_t count, uint16_t frameUs)
{
  for (uint8_t i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) {
    if (i < count && g_chDecode[i] != 0) {
      const uint16_t averaged =
          avgPush(g_chAvgBuf[i], g_chAvgIdx[i], g_chAvgCount[i], g_chDecode[i]);
      g_chPub[i] = emaUpdate(g_emaChxN[i], g_emaChValid[i], averaged);
    } else {
      g_chPub[i] = 0;
      g_chAvgIdx[i] = 0;
      g_chAvgCount[i] = 0;
      memset(g_chAvgBuf[i], 0, sizeof(g_chAvgBuf[i]));
      g_emaChxN[i] = 0;
      g_emaChValid[i] = false;
    }
  }
  const uint16_t frameAvg =
      avgPush(g_frameAvgBuf, g_frameAvgIdx, g_frameAvgCount, frameUs);
  g_chCountRaw = count;
  g_frameUsRaw = emaUpdate(g_emaFramexN, g_emaFrameValid, frameAvg);
  g_haveSignal = (count > 0);

  if (count >= 8) {
    uint16_t us[8];
    for (uint8_t i = 0; i < 8; ++i) us[i] = g_chDecode[i];
    v15MeasureTrainerFeedPushUs(us, 8);
  }

  // Snapshot raw channel intervals for digital CPPM wave preview
  for (uint8_t i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) {
    g_waveChUs[i] = (i < count) ? g_chDecode[i] : 0;
  }
  g_waveChCount = count;
  g_waveFrameUs = frameUs;
  g_waveSeq++;
}

void ppmEdgeIsr()
{
  if (!g_active || g_overcurrentFault || !g_powerOn) return;

  // Rising-edge CPPM (trainer-style interval decode)
  if (!gpio_read(SERVO_TESTER_PWM_GPIO)) return;

  const uint32_t now = timersGetUsTick();
  g_lastEdge10ms = get_tmr10ms();

  if (!g_havePrevEdge) {
    g_lastEdgeUs = now;
    g_havePrevEdge = true;
    return;
  }

  uint32_t dt = now - g_lastEdgeUs;
  g_lastEdgeUs = now;
  dt += PPM_MEASURE_PULSE_OFFSET_US;

  // Sync / blanking gap
  if (dt >= PPM_MEASURE_SYNC_MIN_US && dt <= PPM_MEASURE_SYNC_MAX_US) {
    if (g_decodeCh > 0 && g_decodeCh <= PPM_MEASURE_MAX_CHANNELS &&
        g_frameStartUs != 0) {
      const uint32_t frameUs = now - g_frameStartUs;
      if (frameUs >= 10000U && frameUs <= 40000U) {
        publishFrame(g_decodeCh, (uint16_t)frameUs);
      }
    }
    g_decodeCh = 0;
    g_frameStartUs = now;
    return;
  }

  if (g_decodeCh >= PPM_MEASURE_MAX_CHANNELS) return;

  if (dt < PPM_MEASURE_CH_MIN_US || dt > PPM_MEASURE_CH_MAX_US) {
    g_decodeCh = PPM_MEASURE_MAX_CHANNELS;  // resync
    return;
  }

  uint16_t pulse = (uint16_t)dt;
  if (g_decodeCh == 0) {
    pulse = (uint16_t)(pulse + PPM_MEASURE_CH1_EXTRA_OFFSET_US);
  }
  g_chDecode[g_decodeCh++] = pulse;
}

void copyLiveToDisplay()
{
  // Snapshot volatile filtered results for UI (~20 Hz task)
  uint16_t tmp[PPM_MEASURE_MAX_CHANNELS];
  uint8_t count;
  uint16_t frameUs;
  for (uint8_t i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) tmp[i] = g_chPub[i];
  count = g_chCountRaw;
  frameUs = g_frameUsRaw;

  for (uint8_t i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) g_chUs[i] = tmp[i];
  g_chCount = count;
  g_frameUs = frameUs;
}

}  // namespace

bool v15PpmMeasureIsActive(void) { return g_active; }
bool v15PpmMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15PpmMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15PpmMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15PpmMeasureGetCurrentMa(void) { return g_currentMa; }

uint8_t v15PpmMeasureGetChannelCount(void) { return g_chCount; }

uint16_t v15PpmMeasureGetChannelUs(uint8_t ch)
{
  if (ch >= PPM_MEASURE_MAX_CHANNELS) return 0;
  return g_chUs[ch];
}

uint16_t v15PpmMeasureGetFrameUs(void) { return g_frameUs; }

uint16_t v15PpmMeasureGetFrameHz(void)
{
  if (g_frameUs == 0) return 0;
  return (uint16_t)((1000000U + (g_frameUs / 2U)) / g_frameUs);
}

uint32_t v15PpmMeasureGetWaveSeq(void) { return g_waveSeq; }

uint8_t v15PpmMeasureCopyWaveChannels(uint16_t* dst, uint8_t maxCh)
{
  if (!dst || maxCh == 0) return 0;
  uint8_t n = g_waveChCount;
  if (n > maxCh) n = maxCh;
  if (n > PPM_MEASURE_MAX_CHANNELS) n = PPM_MEASURE_MAX_CHANNELS;
  for (uint8_t i = 0; i < n; ++i) dst[i] = g_waveChUs[i];
  return n;
}

uint16_t v15PpmMeasureGetWaveFrameUs(void) { return g_waveFrameUs; }

void v15PpmMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive() && !v15AutoMeasureIsStartingChild()) return;

  takeOverAuxPort();

  g_haveSignal = false;
  g_havePrevEdge = false;
  g_decodeCh = PPM_MEASURE_MAX_CHANNELS;
  g_lastEdgeUs = 0;
  g_frameStartUs = 0;
  g_chCountRaw = 0;
  g_frameUsRaw = 0;
  g_chCount = 0;
  g_frameUs = 0;
  memset((void*)g_chDecode, 0, sizeof(g_chDecode));
  memset((void*)g_chPub, 0, sizeof(g_chPub));
  memset(g_chUs, 0, sizeof(g_chUs));
  memset(g_waveChUs, 0, sizeof(g_waveChUs));
  g_waveChCount = 0;
  g_waveFrameUs = 0;
  g_waveSeq = 0;
  resetChannelFilter();
  g_lastEdge10ms = get_tmr10ms();

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

  gpio_init_int(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_BOTH, ppmEdgeIsr);

  g_active = true;
  g_powerOn = true;
  setPortPower(true);
}

void v15PpmMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_havePrevEdge = false;
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

void v15PpmMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  const tmr10ms_t now = get_tmr10ms();
  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastEdge10ms) >= PPM_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    g_havePrevEdge = false;
    g_decodeCh = PPM_MEASURE_MAX_CHANNELS;
    g_chCount = 0;
    g_frameUs = 0;
    resetChannelFilter();
    return;
  }

  copyLiveToDisplay();
}

#else  // !RADIO_V15

bool v15PpmMeasureIsActive(void) { return false; }
bool v15PpmMeasureHasSignal(void) { return false; }
bool v15PpmMeasureIsPowerOn(void) { return false; }
bool v15PpmMeasureOvercurrentFault(void) { return false; }
int16_t v15PpmMeasureGetCurrentMa(void) { return 0; }
uint8_t v15PpmMeasureGetChannelCount(void) { return 0; }
uint16_t v15PpmMeasureGetChannelUs(uint8_t) { return 0; }
uint16_t v15PpmMeasureGetFrameUs(void) { return 0; }
uint16_t v15PpmMeasureGetFrameHz(void) { return 0; }
uint32_t v15PpmMeasureGetWaveSeq(void) { return 0; }
uint8_t v15PpmMeasureCopyWaveChannels(uint16_t*, uint8_t) { return 0; }
uint16_t v15PpmMeasureGetWaveFrameUs(void) { return 0; }
void v15PpmMeasureStart(void) {}
void v15PpmMeasureStop(void) {}
void v15PpmMeasureTask(void) {}

#endif  // RADIO_V15
