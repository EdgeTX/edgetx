/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * CRSF measure on V15 J10 (PJ8 half-duplex UART8).
 * Auto-tries baud × polarity until a valid CHANNELS (0x16) frame locks.
 */

#include "crsf_measure.h"
#include "ext_port_safety.h"
#include "measure_trainer_feed.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "crc.h"
#include "edgetx.h"
#include "hal/gpio.h"
#undef UNUSED
#include "hal/serial_driver.h"
#include "ppm_measure.h"
#include "pwm_measure.h"
#include "sbus_measure.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"
#include "auto_measure.h"
#include "stm32_gpio.h"
#include "stm32_serial_driver.h"
#include "telemetry/crossfire.h"
#include "timers_driver.h"

#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;

constexpr uint8_t CRSF_CH_BITS = 11;
constexpr uint16_t CRSF_CH_MASK = (1u << CRSF_CH_BITS) - 1u;
constexpr uint8_t CRSF_CHANNELS_LEN = 24;   // type + 22 payload + CRC
constexpr uint8_t CRSF_PARSE_MAX = 64;

// Common CRSF / ELRS rates (+ EdgeTX Crossfire 400k / 115k).
constexpr uint32_t BAUD_CANDIDATES[] = {400000u, 420000u, 115200u};
constexpr uint8_t BAUD_COUNT = sizeof(BAUD_CANDIDATES) / sizeof(BAUD_CANDIDATES[0]);

// Half-duplex RX on J10 pin3 (UART8_TX / PJ8).
static const stm32_usart_t crsfMeasUsart = {
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

DEFINE_STM32_SERIAL_PORT(CrsfMeas, crsfMeasUsart, 128, 8);

volatile bool g_active = false;
volatile bool g_haveSignal = false;
volatile tmr10ms_t g_lastFrame10ms = 0;
volatile uint32_t g_lastFrameUs = 0;
volatile uint32_t g_framePeriodUs = 0;

volatile uint16_t g_chRaw[CRSF_MEASURE_MAX_CHANNELS] = {};
volatile uint8_t g_polarityUi = CRSF_MEASURE_POL_SEARCHING;
volatile uint32_t g_baudUi = 0;

uint16_t g_chUs[CRSF_MEASURE_MAX_CHANNELS] = {};
uint16_t g_chRawDisp[CRSF_MEASURE_MAX_CHANNELS] = {};
uint8_t g_polarity = CRSF_MEASURE_POL_SEARCHING;
uint32_t g_baudrate = 0;
uint16_t g_frameHz = 0;

void* g_uartCtx = nullptr;
volatile bool g_configLocked = false;
volatile uint8_t g_tryPolarity = ETX_Pol_Normal;  // CRSF default idle-high
volatile uint8_t g_tryBaudIdx = 0;
tmr10ms_t g_configTryStart = 0;

uint8_t g_parseBuf[CRSF_PARSE_MAX] = {};
uint8_t g_parseLen = 0;

// Last CHANNELS frame snapshot for UI digital waveform (partial OK)
uint8_t g_waveBytes[CRSF_MEASURE_WAVE_BYTES] = {};
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
  g_configLocked = false;
  g_polarityUi = CRSF_MEASURE_POL_SEARCHING;
  g_baudUi = 0;

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
        CRSF_MEASURE_OC_DELTA_MA) {
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
      CRSF_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      CRSF_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

uint16_t rawToUs(uint16_t raw)
{
  return (uint16_t)(((uint32_t)raw * 5u) / 8u + 880u);
}

bool isSyncByte(uint8_t b)
{
  return b == MODULE_ADDRESS || b == RADIO_ADDRESS || b == UART_SYNC ||
         b == RECEIVER_ADDRESS;
}

void publishChannels(const uint8_t* payload)
{
  uint32_t inputbits = 0;
  uint32_t inputbitsavailable = 0;
  const uint8_t* p = payload;

  for (uint8_t i = 0; i < CRSF_MEASURE_MAX_CHANNELS; ++i) {
    while (inputbitsavailable < CRSF_CH_BITS) {
      inputbits |= (uint32_t)(*p++) << inputbitsavailable;
      inputbitsavailable += 8;
    }
    g_chRaw[i] = (uint16_t)(inputbits & CRSF_CH_MASK);
    inputbitsavailable -= CRSF_CH_BITS;
    inputbits >>= CRSF_CH_BITS;
  }

  const uint32_t nowUs = timersGetUsTick();
  if (g_lastFrameUs != 0) {
    const uint32_t dt = nowUs - g_lastFrameUs;
    if (dt >= 2000U && dt <= 50000U) {
      g_framePeriodUs = dt;
    }
  }
  g_lastFrameUs = nowUs;
  g_lastFrame10ms = get_tmr10ms();
  g_haveSignal = true;
  {
    uint16_t raw[8];
    for (uint8_t i = 0; i < 8; ++i) raw[i] = g_chRaw[i];
    v15MeasureTrainerFeedPushBusRaw(raw, 8);
  }

  if (!g_configLocked) {
    g_configLocked = true;
    g_baudUi = BAUD_CANDIDATES[g_tryBaudIdx];
    g_polarityUi = (g_tryPolarity == ETX_Pol_Inverted) ? CRSF_MEASURE_POL_INVERTED
                                                       : CRSF_MEASURE_POL_NORMAL;
  }
}

void consumeParseBuffer()
{
  while (g_parseLen >= 3) {
    if (!isSyncByte(g_parseBuf[0])) {
      memmove(g_parseBuf, g_parseBuf + 1, --g_parseLen);
      continue;
    }

    const uint8_t len = g_parseBuf[1];
    if (len < 2 || len > 60) {
      memmove(g_parseBuf, g_parseBuf + 1, --g_parseLen);
      continue;
    }

    const uint8_t total = (uint8_t)(len + 2);
    if (g_parseLen < total) return;

    const uint8_t type = g_parseBuf[2];
    const uint8_t crc = g_parseBuf[total - 1];
    const bool crcOk = (crc8(&g_parseBuf[2], (uint32_t)(len - 1)) == crc);

    if (type == CHANNELS_ID && len >= CRSF_CHANNELS_LEN && crcOk) {
      publishChannels(&g_parseBuf[3]);
      // Snapshot frame head for UI scope (truncated is fine)
      uint8_t n = total;
      if (n > CRSF_MEASURE_WAVE_BYTES) n = CRSF_MEASURE_WAVE_BYTES;
      memcpy(g_waveBytes, g_parseBuf, n);
      g_waveLen = n;
      g_waveSeq++;
    }

    // Consume one frame (or drop sync on bad CRC/type and resync)
    if (crcOk) {
      const uint8_t remain = (uint8_t)(g_parseLen - total);
      memmove(g_parseBuf, g_parseBuf + total, remain);
      g_parseLen = remain;
    } else {
      memmove(g_parseBuf, g_parseBuf + 1, --g_parseLen);
    }
  }
}

void onCrsfIdle(void*)
{
  if (!g_active || g_overcurrentFault || !g_powerOn || !g_uartCtx) return;
  if (!STM32SerialDriver.getByte) return;

  uint8_t b = 0;
  while (STM32SerialDriver.getByte(g_uartCtx, &b) > 0) {
    if (g_parseLen < CRSF_PARSE_MAX) {
      g_parseBuf[g_parseLen++] = b;
    } else {
      // Overflow: keep last half for resync
      memmove(g_parseBuf, g_parseBuf + (CRSF_PARSE_MAX / 2), CRSF_PARSE_MAX / 2);
      g_parseLen = CRSF_PARSE_MAX / 2;
      g_parseBuf[g_parseLen++] = b;
    }
  }

  consumeParseBuffer();
}

void stopUart()
{
  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }
  g_parseLen = 0;
}

bool startUart(uint32_t baudrate, uint8_t polarity)
{
  stopUart();

  etx_serial_init params = {
      .baudrate = baudrate,
      .encoding = ETX_Encoding_8N1,
      .direction = ETX_Dir_RX,
      .polarity = polarity,
  };

  g_uartCtx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(CrsfMeas), &params);
  if (!g_uartCtx) return false;

  if (STM32SerialDriver.setIdleCb) {
    STM32SerialDriver.setIdleCb(g_uartCtx, onCrsfIdle, nullptr);
  }
  return true;
}

void beginConfigSearch(uint8_t baudIdx, uint8_t polarity)
{
  g_configLocked = false;
  g_tryBaudIdx = baudIdx % BAUD_COUNT;
  g_tryPolarity = polarity;
  g_configTryStart = get_tmr10ms();
  g_polarityUi = CRSF_MEASURE_POL_SEARCHING;
  g_baudUi = 0;
  g_haveSignal = false;
  g_lastFrameUs = 0;
  g_parseLen = 0;
  startUart(BAUD_CANDIDATES[g_tryBaudIdx], g_tryPolarity);
}

void advanceConfigSearch()
{
  // Cycle: for each baud, Normal then Inverted
  if (g_tryPolarity == ETX_Pol_Normal) {
    g_tryPolarity = ETX_Pol_Inverted;
  } else {
    g_tryPolarity = ETX_Pol_Normal;
    g_tryBaudIdx = (uint8_t)((g_tryBaudIdx + 1) % BAUD_COUNT);
  }
  g_configTryStart = get_tmr10ms();
  g_parseLen = 0;
  startUart(BAUD_CANDIDATES[g_tryBaudIdx], g_tryPolarity);
}

void copyLiveToDisplay()
{
  uint16_t raw[CRSF_MEASURE_MAX_CHANNELS];
  uint8_t pol;
  uint32_t baud;
  uint32_t periodUs;

  for (uint8_t i = 0; i < CRSF_MEASURE_MAX_CHANNELS; ++i) raw[i] = g_chRaw[i];
  pol = g_polarityUi;
  baud = g_baudUi;
  periodUs = g_framePeriodUs;

  for (uint8_t i = 0; i < CRSF_MEASURE_MAX_CHANNELS; ++i) {
    g_chRawDisp[i] = raw[i];
    g_chUs[i] = rawToUs(raw[i]);
  }
  g_polarity = pol;
  g_baudrate = baud;
  if (periodUs > 0) {
    g_frameHz = (uint16_t)((1000000U + (periodUs / 2U)) / periodUs);
  }
}

}  // namespace

bool v15CrsfMeasureIsActive(void) { return g_active; }
bool v15CrsfMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15CrsfMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15CrsfMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15CrsfMeasureGetCurrentMa(void) { return g_currentMa; }

uint8_t v15CrsfMeasureGetPolarity(void) { return g_polarity; }
uint32_t v15CrsfMeasureGetBaudrate(void) { return g_baudrate; }

uint8_t v15CrsfMeasureGetChannelCount(void)
{
  return v15CrsfMeasureHasSignal() ? CRSF_MEASURE_MAX_CHANNELS : 0;
}

uint16_t v15CrsfMeasureGetChannelUs(uint8_t ch)
{
  if (ch >= CRSF_MEASURE_MAX_CHANNELS) return 0;
  return g_chUs[ch];
}

uint16_t v15CrsfMeasureGetChannelRaw(uint8_t ch)
{
  if (ch >= CRSF_MEASURE_MAX_CHANNELS) return 0;
  return g_chRawDisp[ch];
}

uint16_t v15CrsfMeasureGetFrameHz(void) { return g_frameHz; }

uint32_t v15CrsfMeasureGetWaveSeq(void) { return g_waveSeq; }

uint8_t v15CrsfMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen)
{
  if (!dst || maxLen == 0) return 0;
  uint8_t n = g_waveLen;
  if (n > maxLen) n = maxLen;
  if (n > CRSF_MEASURE_WAVE_BYTES) n = CRSF_MEASURE_WAVE_BYTES;
  if (n) memcpy(dst, g_waveBytes, n);
  return n;
}

void v15CrsfMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
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
  g_frameHz = 0;
  g_framePeriodUs = 0;
  g_baudrate = 0;
  g_lastFrameUs = 0;
  g_lastFrame10ms = get_tmr10ms();
  g_parseLen = 0;

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

  beginConfigSearch(0, ETX_Pol_Normal);
}

void v15CrsfMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_configLocked = false;
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

void v15CrsfMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  const tmr10ms_t now = get_tmr10ms();

  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastFrame10ms) >= CRSF_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    g_frameHz = 0;
    beginConfigSearch(g_tryBaudIdx, g_tryPolarity);
    return;
  }

  if (!g_configLocked && g_uartCtx) {
    if ((tmr10ms_t)(now - g_configTryStart) >= CRSF_MEASURE_CONFIG_TRY_10MS) {
      advanceConfigSearch();
    }
  }

  if (g_haveSignal) {
    copyLiveToDisplay();
  }
}

#else  // !RADIO_V15

bool v15CrsfMeasureIsActive(void) { return false; }
bool v15CrsfMeasureHasSignal(void) { return false; }
bool v15CrsfMeasureIsPowerOn(void) { return false; }
bool v15CrsfMeasureOvercurrentFault(void) { return false; }
int16_t v15CrsfMeasureGetCurrentMa(void) { return 0; }
uint8_t v15CrsfMeasureGetPolarity(void) { return CRSF_MEASURE_POL_SEARCHING; }
uint32_t v15CrsfMeasureGetBaudrate(void) { return 0; }
uint8_t v15CrsfMeasureGetChannelCount(void) { return 0; }
uint16_t v15CrsfMeasureGetChannelUs(uint8_t) { return 0; }
uint16_t v15CrsfMeasureGetChannelRaw(uint8_t) { return 0; }
uint16_t v15CrsfMeasureGetFrameHz(void) { return 0; }
uint32_t v15CrsfMeasureGetWaveSeq(void) { return 0; }
uint8_t v15CrsfMeasureCopyWaveBytes(uint8_t*, uint8_t) { return 0; }
void v15CrsfMeasureStart(void) {}
void v15CrsfMeasureStop(void) {}
void v15CrsfMeasureTask(void) {}

#endif  // RADIO_V15
