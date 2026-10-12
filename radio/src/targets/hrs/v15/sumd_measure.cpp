/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * SUMD measure on V15 J10 (PJ8 half-duplex UART8).
 * Auto-tries normal then inverted polarity until a valid frame locks.
 */

#include "sumd_measure.h"
#include "ext_port_safety.h"
#include "measure_trainer_feed.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "crc.h"
#include "edgetx.h"
#include "hal/gpio.h"
#undef UNUSED
#include "hal/serial_driver.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "auto_measure.h"
#include "stm32_gpio.h"
#include "stm32_serial_driver.h"
#include "timers_driver.h"

#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;
constexpr uint32_t SUMD_BAUDRATE = 115200u;

constexpr uint8_t SUMD_SYNC_BYTE = 0xA8;
constexpr uint8_t SUMD_STATUS_OK = 0x01;

// Half-duplex RX on J10 pin3 (UART8_TX / PJ8) ù?PJ9 is not on the jack.
static const stm32_usart_t sumdMeasUsart = {
    .USARTx = AUX_SERIAL_USART,
    .txGPIO = AUX_SERIAL_TX_GPIO,
    .rxGPIO = GPIO_UNDEF,
    .IRQn = AUX_SERIAL_USART_IRQn,
    .IRQ_Prio = 7,
    .txDMA = nullptr,
    .txDMA_Stream = 0,
    .txDMA_Channel = 0,
    .rxDMA = AUX_SERIAL_DMA_RX,
    .rxDMA_Stream = AUX_SERIAL_DMA_RX_STREAM,
    .rxDMA_Channel = AUX_SERIAL_DMA_RX_CHANNEL,
};

// 512 B RX ring; UART IDLE callback drains like CRSF/SBUS (not UI-polled only).
DEFINE_STM32_SERIAL_PORT(SumdMeas, sumdMeasUsart, 512, 8);

volatile bool g_active = false;
volatile bool g_haveSignal = false;
volatile tmr10ms_t g_lastFrame10ms = 0;

volatile uint16_t g_chRaw[SUMD_MEASURE_MAX_CHANNELS] = {};
volatile uint8_t g_statusRaw = 0;
volatile uint8_t g_channelCountRaw = 0;
volatile uint8_t g_polarityUi = SUMD_MEASURE_POL_SEARCHING;

uint16_t g_chUs[SUMD_MEASURE_MAX_CHANNELS] = {};
uint16_t g_chRawDisp[SUMD_MEASURE_MAX_CHANNELS] = {};
uint8_t g_status = 0;
uint8_t g_channelCount = 0;
uint8_t g_polarity = SUMD_MEASURE_POL_SEARCHING;
uint16_t g_frameHz = 0;
uint16_t g_frameCountWindow = 0;
tmr10ms_t g_hzWindowStart = 0;

void* g_uartCtx = nullptr;
volatile bool g_polarityLocked = false;
volatile uint8_t g_tryPolarity = ETX_Pol_Normal;
tmr10ms_t g_polarityTryStart = 0;
tmr10ms_t g_lostSince = 0;

uint8_t g_frameBuf[SUMD_MEASURE_WAVE_BYTES] = {};
uint8_t g_frameLen = 0;
uint8_t g_expectedLen = 0;
uint8_t g_feedDepth = 0;

uint8_t g_waveBytes[SUMD_MEASURE_WAVE_BYTES] = {};
uint8_t g_waveLen = 0;
volatile uint32_t g_waveSeq = 0;

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
  g_polarityLocked = false;
  g_polarityUi = SUMD_MEASURE_POL_SEARCHING;

  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }

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
        SUMD_MEASURE_OC_DELTA_MA) {
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
      SUMD_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      SUMD_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

uint16_t rawToUs(uint16_t raw)
{
  return (uint16_t)(raw >> 3);
}

void resetParser()
{
  g_frameLen = 0;
  g_expectedLen = 0;
}

bool processFrame(const uint8_t* frame, uint8_t len)
{
  if (len < 5 || frame[0] != SUMD_SYNC_BYTE) return false;

  const uint8_t status = frame[1];
  const uint8_t n = frame[2];
  if (n < 1 || n > SUMD_MEASURE_MAX_CHANNELS) return false;

  const uint8_t expected = (uint8_t)(5 + n * 2);
  if (len < expected) return false;

  const uint16_t crcRx =
      (uint16_t)(((uint16_t)frame[expected - 2] << 8) | frame[expected - 1]);
  const uint16_t crcCalc =
      crc16(CRC_1021, frame, (uint32_t)(expected - 2), 0);
  if (crcRx != crcCalc) return false;

  const uint8_t* p = frame + 3;
  for (uint8_t i = 0; i < SUMD_MEASURE_MAX_CHANNELS; ++i) {
    if (i < n) {
      g_chRaw[i] = (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
      p += 2;
    } else {
      g_chRaw[i] = 0;
    }
  }

  g_lastFrame10ms = get_tmr10ms();
  g_frameCountWindow++;

  g_statusRaw = status;
  g_channelCountRaw = n;
  g_haveSignal = true;
  g_lostSince = 0;

  if (n >= 8) {
    uint16_t us[8];
    for (uint8_t i = 0; i < 8; ++i) us[i] = (uint16_t)(g_chRaw[i] >> 3);
    // Match UI: restore TX order (ELRS SUMD swaps wire CH5/CH8).
    const uint16_t tmp = us[4];
    us[4] = us[7];
    us[7] = tmp;
    v15MeasureTrainerFeedPushUs(us, 8);
  }

  uint8_t wn = expected;
  if (wn > SUMD_MEASURE_WAVE_BYTES) wn = SUMD_MEASURE_WAVE_BYTES;
  memcpy(g_waveBytes, frame, wn);
  g_waveLen = wn;
  g_waveSeq++;

  if (!g_polarityLocked) {
    g_polarityLocked = true;
    g_polarityUi = (g_tryPolarity == ETX_Pol_Inverted) ? SUMD_MEASURE_POL_INVERTED
                                                       : SUMD_MEASURE_POL_NORMAL;
  }
  return true;
}

void feedByte(uint8_t b);

void resyncFromBuffer(uint8_t fromIdx)
{
  uint8_t tmp[SUMD_MEASURE_WAVE_BYTES];
  const uint8_t n = g_frameLen;
  if (fromIdx >= n) {
    resetParser();
    return;
  }
  memcpy(tmp, g_frameBuf, n);
  resetParser();
  for (uint8_t i = fromIdx; i < n; ++i) {
    feedByte(tmp[i]);
  }
}

void feedByte(uint8_t b)
{
  // resyncFromBuffer re-enters feedByte; bound depth on pathological streams.
  if (g_feedDepth >= 64) {
    resetParser();
    return;
  }
  ++g_feedDepth;

  if (g_frameLen == 0) {
    if (b != SUMD_SYNC_BYTE) {
      --g_feedDepth;
      return;
    }
    g_frameBuf[g_frameLen++] = b;
    --g_feedDepth;
    return;
  }

  if (g_frameLen >= SUMD_MEASURE_WAVE_BYTES) {
    resyncFromBuffer(1);
    feedByte(b);
    --g_feedDepth;
    return;
  }

  g_frameBuf[g_frameLen++] = b;

  if (g_frameLen == 3) {
    const uint8_t nch = g_frameBuf[2];
    if (nch < 1 || nch > SUMD_MEASURE_MAX_CHANNELS) {
      resyncFromBuffer(1);
      --g_feedDepth;
      return;
    }
    g_expectedLen = (uint8_t)(5 + nch * 2);
    if (g_expectedLen > SUMD_MEASURE_WAVE_BYTES) {
      resyncFromBuffer(1);
      --g_feedDepth;
      return;
    }
  }

  if (g_expectedLen > 0 && g_frameLen >= g_expectedLen) {
    if (processFrame(g_frameBuf, g_expectedLen)) {
      resetParser();
    } else {
      // Bad CRC / false sync on 0xA8 inside payload ù?slide forward.
      resyncFromBuffer(1);
    }
  }

  --g_feedDepth;
}

void pollUartBytes()
{
  if (!g_active || g_overcurrentFault || !g_powerOn || !g_uartCtx) return;
  if (!STM32SerialDriver.getByte) return;

  uint8_t b = 0;
  while (STM32SerialDriver.getByte(g_uartCtx, &b) > 0) {
    feedByte(b);
  }
}

// Same path as CRSF/SBUS: drain on USART IDLE so parsing is not gated by LVGL.
void onSumdIdle(void*)
{
  pollUartBytes();
}

void stopUart()
{
  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }
  resetParser();
}

bool startUart(uint8_t polarity)
{
  stopUart();

  etx_serial_init params = {
      .baudrate = SUMD_BAUDRATE,
      .encoding = ETX_Encoding_8N1,
      .direction = ETX_Dir_RX,
      .polarity = polarity,
  };

  g_uartCtx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(SumdMeas), &params);
  if (!g_uartCtx) return false;

  if (STM32SerialDriver.setIdleCb) {
    STM32SerialDriver.setIdleCb(g_uartCtx, onSumdIdle, nullptr);
  }
  return true;
}

void beginPolaritySearch(uint8_t firstPolarity)
{
  g_polarityLocked = false;
  g_tryPolarity = firstPolarity;
  g_polarityTryStart = get_tmr10ms();
  g_polarityUi = SUMD_MEASURE_POL_SEARCHING;
  g_haveSignal = false;
  g_lostSince = 0;
  g_frameCountWindow = 0;
  g_hzWindowStart = get_tmr10ms();
  g_frameHz = 0;
  resetParser();
  startUart(g_tryPolarity);
}

void updateFrameHz(tmr10ms_t now)
{
  // 500 ms window ù?stable Hz (ELRS SUMD is fixed ~100 Hz).
  constexpr tmr10ms_t HZ_WINDOW_10MS = 50;
  const tmr10ms_t elapsed = (tmr10ms_t)(now - g_hzWindowStart);
  if (elapsed < HZ_WINDOW_10MS) return;
  if (elapsed == 0) return;
  g_frameHz = (uint16_t)(((uint32_t)g_frameCountWindow * 100U) / (uint32_t)elapsed);
  g_frameCountWindow = 0;
  g_hzWindowStart = now;
}

void copyLiveToDisplay()
{
  uint16_t raw[SUMD_MEASURE_MAX_CHANNELS];
  uint8_t status;
  uint8_t count;
  uint8_t pol;

  for (uint8_t i = 0; i < SUMD_MEASURE_MAX_CHANNELS; ++i) raw[i] = g_chRaw[i];
  status = g_statusRaw;
  count = g_channelCountRaw;
  pol = g_polarityUi;

  // ExpressLRS SUMD swaps wire CH5?CH8 so AUX1/arm is not on SUMD CH5.
  // Restore TX logical order for the channel bars (see ELRS SerialSUMD.cpp).
  if (count >= 8) {
    const uint16_t tmp = raw[4];
    raw[4] = raw[7];
    raw[7] = tmp;
  }

  for (uint8_t i = 0; i < SUMD_MEASURE_MAX_CHANNELS; ++i) {
    g_chRawDisp[i] = raw[i];
    g_chUs[i] = rawToUs(raw[i]);
  }
  g_status = (status == SUMD_STATUS_OK) ? 1u : 0u;
  g_channelCount = count;
  g_polarity = pol;
}

}  // namespace

bool v15SumdMeasureIsActive(void) { return g_active; }
bool v15SumdMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15SumdMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15SumdMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15SumdMeasureGetCurrentMa(void) { return g_currentMa; }

uint8_t v15SumdMeasureGetPolarity(void) { return g_polarity; }
uint8_t v15SumdMeasureGetStatus(void) { return g_status; }

uint8_t v15SumdMeasureGetChannelCount(void)
{
  return v15SumdMeasureHasSignal() ? g_channelCount : 0;
}

uint16_t v15SumdMeasureGetChannelUs(uint8_t ch)
{
  if (ch >= SUMD_MEASURE_MAX_CHANNELS) return 0;
  return g_chUs[ch];
}

uint16_t v15SumdMeasureGetChannelRaw(uint8_t ch)
{
  if (ch >= SUMD_MEASURE_MAX_CHANNELS) return 0;
  return g_chRawDisp[ch];
}

uint16_t v15SumdMeasureGetFrameHz(void) { return g_frameHz; }

uint32_t v15SumdMeasureGetWaveSeq(void) { return g_waveSeq; }

uint8_t v15SumdMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen)
{
  if (!dst || maxLen == 0) return 0;
  uint8_t n = g_waveLen;
  if (n > maxLen) n = maxLen;
  if (n > SUMD_MEASURE_WAVE_BYTES) n = SUMD_MEASURE_WAVE_BYTES;
  if (n) memcpy(dst, g_waveBytes, n);
  return n;
}

void v15SumdMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15MavlinkMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive() && !v15AutoMeasureIsStartingChild()) return;

  takeOverAuxPort();

  memset((void*)g_chRaw, 0, sizeof(g_chRaw));
  memset(g_chUs, 0, sizeof(g_chUs));
  memset(g_chRawDisp, 0, sizeof(g_chRawDisp));
  memset(g_waveBytes, 0, sizeof(g_waveBytes));
  g_waveLen = 0;
  g_waveSeq = 0;
  g_statusRaw = 0;
  g_status = 0;
  g_channelCountRaw = 0;
  g_channelCount = 0;
  g_frameHz = 0;
  g_frameCountWindow = 0;
  g_hzWindowStart = get_tmr10ms();
  g_lostSince = 0;
  g_lastFrame10ms = get_tmr10ms();
  resetParser();

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

  beginPolaritySearch(ETX_Pol_Normal);
}

void v15SumdMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_polarityLocked = false;
  g_overcurrentFault = false;

  stopUart();
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);

  if (v15AutoMeasureIsActive()) {
    g_auxTakenOver = false;
  } else {
    setExtSignalPath(false);
    setPortPower(false);
    restoreAuxPort();
#if defined(MODULE_BATTERY_SENSOR)
    v15BatterySensorResumeAfterExtPort();
#endif
  }
}

void v15SumdMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  pollUartBytes();

  const tmr10ms_t now = get_tmr10ms();
  updateFrameHz(now);

  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastFrame10ms) >= SUMD_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    if (g_polarityLocked) {
      // Keep locked UART/polarity ù?brief gaps must not tear down the port
      // (that causes UI SEARCHING flash and more byte loss).
      g_lostSince = now;
    } else {
      beginPolaritySearch(g_tryPolarity);
      return;
    }
  }

  // Only re-hunt polarity after a sustained loss while previously locked.
  constexpr tmr10ms_t RELOCK_10MS = 100;  // 1 s
  if (!g_haveSignal && g_polarityLocked && g_lostSince != 0 &&
      (tmr10ms_t)(now - g_lostSince) >= RELOCK_10MS) {
    beginPolaritySearch(g_tryPolarity);
    return;
  }

  if (!g_polarityLocked && g_uartCtx) {
    if ((tmr10ms_t)(now - g_polarityTryStart) >= SUMD_MEASURE_POLARITY_TRY_10MS) {
      g_tryPolarity = (g_tryPolarity == ETX_Pol_Normal) ? ETX_Pol_Inverted
                                                        : ETX_Pol_Normal;
      g_polarityTryStart = now;
      resetParser();
      startUart(g_tryPolarity);
    }
  }

  if (g_haveSignal) {
    copyLiveToDisplay();
  }
}

#else  // !RADIO_V15

bool v15SumdMeasureIsActive(void) { return false; }
bool v15SumdMeasureHasSignal(void) { return false; }
bool v15SumdMeasureIsPowerOn(void) { return false; }
bool v15SumdMeasureOvercurrentFault(void) { return false; }
int16_t v15SumdMeasureGetCurrentMa(void) { return 0; }
uint8_t v15SumdMeasureGetPolarity(void) { return SUMD_MEASURE_POL_SEARCHING; }
uint8_t v15SumdMeasureGetStatus(void) { return 0; }
uint8_t v15SumdMeasureGetChannelCount(void) { return 0; }
uint16_t v15SumdMeasureGetChannelUs(uint8_t) { return 0; }
uint16_t v15SumdMeasureGetChannelRaw(uint8_t) { return 0; }
uint16_t v15SumdMeasureGetFrameHz(void) { return 0; }
uint32_t v15SumdMeasureGetWaveSeq(void) { return 0; }
uint8_t v15SumdMeasureCopyWaveBytes(uint8_t*, uint8_t) { return 0; }
void v15SumdMeasureStart(void) {}
void v15SumdMeasureStop(void) {}
void v15SumdMeasureTask(void) {}

#endif  // RADIO_V15
