/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * DJI RS Pro measure on V15 J10 (PJ8 half-duplex UART8).
 * SBUS-compatible framing with DJI channel scaling for display.
 * Auto-tries inverted then normal polarity until a valid frame locks.
 */

#include "dji_rs_measure.h"
#include "ext_port_safety.h"
#include "measure_trainer_feed.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#undef UNUSED
#include "hal/serial_driver.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
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

constexpr uint8_t DJI_RS_FRAME_SIZE = 25;
constexpr uint8_t DJI_RS_START_BYTE = 0x0F;
constexpr uint8_t DJI_RS_END_BYTE = 0x00;
constexpr uint8_t DJI_RS_FLAGS_IDX = 23;
constexpr uint8_t DJI_RS_FRAMELOST_BIT = 2;
constexpr uint8_t DJI_RS_FAILSAFE_BIT = 3;
constexpr uint8_t DJI_RS_CH17_BIT = 0;
constexpr uint8_t DJI_RS_CH18_BIT = 1;
constexpr uint8_t DJI_RS_CH_BITS = 11;
constexpr uint16_t DJI_RS_CH_MASK = (1u << DJI_RS_CH_BITS) - 1u;

// Half-duplex RX on J10 pin3 (UART8_TX / PJ8) ù?PJ9 is not on the jack.
static const stm32_usart_t djiRsMeasUsart = {
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

DEFINE_STM32_SERIAL_PORT(DjiRsMeas, djiRsMeasUsart, 64, 8);

volatile bool g_active = false;
volatile bool g_haveSignal = false;
volatile tmr10ms_t g_lastFrame10ms = 0;
volatile uint32_t g_lastFrameUs = 0;
volatile uint32_t g_framePeriodUs = 0;

volatile uint16_t g_chRaw[DJI_RS_MEASURE_MAX_CHANNELS] = {};
volatile uint8_t g_flagsRaw = 0;
volatile uint8_t g_polarityUi = DJI_RS_MEASURE_POL_SEARCHING;

uint16_t g_chUs[DJI_RS_MEASURE_MAX_CHANNELS] = {};
uint16_t g_chRawDisp[DJI_RS_MEASURE_MAX_CHANNELS] = {};
uint8_t g_flags = 0;
uint8_t g_polarity = DJI_RS_MEASURE_POL_SEARCHING;
uint16_t g_frameHz = 0;

void* g_uartCtx = nullptr;
volatile bool g_polarityLocked = false;
volatile uint8_t g_tryPolarity = ETX_Pol_Inverted;
tmr10ms_t g_polarityTryStart = 0;

uint8_t g_waveBytes[DJI_RS_MEASURE_WAVE_BYTES] = {};
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
  g_polarityUi = DJI_RS_MEASURE_POL_SEARCHING;

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
        DJI_RS_MEASURE_OC_DELTA_MA) {
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
      DJI_RS_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      DJI_RS_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

uint16_t rawToUs(uint16_t raw)
{
  // ELRS DJI RS Pro already stores microseconds in the 11-bit fields.
  return raw;
}

void remapWireToTxOrder(uint16_t* ch)
{
  // ExpressLRS SerialSBUS DJI mode:
  //   wire CH1ù?  = TX CH1ù?
  //   wire CH5ù?  = TX CH6ù?
  //   wire CH8ù?5 = TX CH9ù?6
  //   wire CH16   = TX CH5 (arm, digital 352/1696)
  uint16_t out[DJI_RS_MEASURE_MAX_CHANNELS];
  out[0] = ch[0];
  out[1] = ch[1];
  out[2] = ch[2];
  out[3] = ch[3];
  out[4] = ch[15];
  out[5] = ch[4];
  out[6] = ch[5];
  out[7] = ch[6];
  out[8] = ch[7];
  out[9] = ch[8];
  out[10] = ch[9];
  out[11] = ch[10];
  out[12] = ch[11];
  out[13] = ch[12];
  out[14] = ch[13];
  out[15] = ch[14];
  memcpy(ch, out, sizeof(out));
}

void processFrame(const uint8_t* sbus)
{
  if (sbus[0] != DJI_RS_START_BYTE || sbus[DJI_RS_FRAME_SIZE - 1] != DJI_RS_END_BYTE) {
    return;
  }

  const uint8_t flags = sbus[DJI_RS_FLAGS_IDX];
  const uint8_t* p = sbus + 1;

  uint32_t inputbits = 0;
  uint32_t inputbitsavailable = 0;
  uint16_t ch[DJI_RS_MEASURE_MAX_CHANNELS];

  for (uint8_t i = 0; i < DJI_RS_MEASURE_MAX_CHANNELS; ++i) {
    while (inputbitsavailable < DJI_RS_CH_BITS) {
      inputbits |= (uint32_t)(*p++) << inputbitsavailable;
      inputbitsavailable += 8;
    }
    ch[i] = (uint16_t)(inputbits & DJI_RS_CH_MASK);
    inputbitsavailable -= DJI_RS_CH_BITS;
    inputbits >>= DJI_RS_CH_BITS;
  }

  const uint32_t nowUs = timersGetUsTick();
  if (g_lastFrameUs != 0) {
    const uint32_t dt = nowUs - g_lastFrameUs;
    if (dt >= 4000U && dt <= 30000U) {
      g_framePeriodUs = dt;
    }
  }
  g_lastFrameUs = nowUs;
  g_lastFrame10ms = get_tmr10ms();

  for (uint8_t i = 0; i < DJI_RS_MEASURE_MAX_CHANNELS; ++i) {
    g_chRaw[i] = ch[i];
  }
  g_flagsRaw = flags;
  g_haveSignal = true;

  if ((flags & (1u << DJI_RS_FAILSAFE_BIT)) == 0) {
    uint16_t tx[DJI_RS_MEASURE_MAX_CHANNELS];
    for (uint8_t i = 0; i < DJI_RS_MEASURE_MAX_CHANNELS; ++i) tx[i] = ch[i];
    remapWireToTxOrder(tx);
    v15MeasureTrainerFeedPushDjiRs(tx, 8);
  }

  memcpy(g_waveBytes, sbus, DJI_RS_FRAME_SIZE);
  g_waveLen = DJI_RS_FRAME_SIZE;
  g_waveSeq++;

  if (!g_polarityLocked) {
    g_polarityLocked = true;
    g_polarityUi = (g_tryPolarity == ETX_Pol_Inverted) ? DJI_RS_MEASURE_POL_INVERTED
                                                       : DJI_RS_MEASURE_POL_NORMAL;
  }
}

void onDjiRsIdle(void*)
{
  if (!g_active || g_overcurrentFault || !g_powerOn || !g_uartCtx) return;
  if (!STM32SerialDriver.getBufferedBytes || !STM32SerialDriver.copyRxBuffer ||
      !STM32SerialDriver.clearRxBuffer) {
    return;
  }

  const int n = STM32SerialDriver.getBufferedBytes(g_uartCtx);
  if (n != DJI_RS_FRAME_SIZE) {
    STM32SerialDriver.clearRxBuffer(g_uartCtx);
    return;
  }

  uint8_t frame[DJI_RS_FRAME_SIZE];
  if (STM32SerialDriver.copyRxBuffer(g_uartCtx, frame, DJI_RS_FRAME_SIZE) < 0) {
    return;
  }

  processFrame(frame);
}

void stopUart()
{
  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }
}

bool startUart(uint8_t polarity)
{
  stopUart();

  etx_serial_init params = {
      .baudrate = SBUS_BAUDRATE,
      .encoding = ETX_Encoding_8E2,
      .direction = ETX_Dir_RX,
      .polarity = polarity,
  };

  g_uartCtx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(DjiRsMeas), &params);
  if (!g_uartCtx) return false;

  if (STM32SerialDriver.setIdleCb) {
    STM32SerialDriver.setIdleCb(g_uartCtx, onDjiRsIdle, nullptr);
  }
  return true;
}

void beginPolaritySearch(uint8_t firstPolarity)
{
  g_polarityLocked = false;
  g_tryPolarity = firstPolarity;
  g_polarityTryStart = get_tmr10ms();
  g_polarityUi = DJI_RS_MEASURE_POL_SEARCHING;
  g_haveSignal = false;
  g_lastFrameUs = 0;
  startUart(g_tryPolarity);
}

void copyLiveToDisplay()
{
  uint16_t raw[DJI_RS_MEASURE_MAX_CHANNELS];
  uint8_t flags;
  uint8_t pol;
  uint32_t periodUs;

  for (uint8_t i = 0; i < DJI_RS_MEASURE_MAX_CHANNELS; ++i) raw[i] = g_chRaw[i];
  flags = g_flagsRaw;
  pol = g_polarityUi;
  periodUs = g_framePeriodUs;

  remapWireToTxOrder(raw);

  for (uint8_t i = 0; i < DJI_RS_MEASURE_MAX_CHANNELS; ++i) {
    g_chRawDisp[i] = raw[i];
    g_chUs[i] = rawToUs(raw[i]);
  }
  g_flags = flags;
  g_polarity = pol;
  if (periodUs > 0) {
    g_frameHz = (uint16_t)((1000000U + (periodUs / 2U)) / periodUs);
  }
}

}  // namespace

bool v15DjiRsMeasureIsActive(void) { return g_active; }
bool v15DjiRsMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15DjiRsMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15DjiRsMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15DjiRsMeasureGetCurrentMa(void) { return g_currentMa; }

uint8_t v15DjiRsMeasureGetPolarity(void) { return g_polarity; }
uint8_t v15DjiRsMeasureGetFlags(void) { return g_flags; }
bool v15DjiRsMeasureFailsafe(void)
{
  return (g_flags & (1u << DJI_RS_FAILSAFE_BIT)) != 0;
}
bool v15DjiRsMeasureFrameLost(void)
{
  return (g_flags & (1u << DJI_RS_FRAMELOST_BIT)) != 0;
}
bool v15DjiRsMeasureCh17(void) { return (g_flags & (1u << DJI_RS_CH17_BIT)) != 0; }
bool v15DjiRsMeasureCh18(void) { return (g_flags & (1u << DJI_RS_CH18_BIT)) != 0; }

uint8_t v15DjiRsMeasureGetChannelCount(void)
{
  return v15DjiRsMeasureHasSignal() ? DJI_RS_MEASURE_MAX_CHANNELS : 0;
}

uint16_t v15DjiRsMeasureGetChannelUs(uint8_t ch)
{
  if (ch >= DJI_RS_MEASURE_MAX_CHANNELS) return 0;
  return g_chUs[ch];
}

uint16_t v15DjiRsMeasureGetChannelRaw(uint8_t ch)
{
  if (ch >= DJI_RS_MEASURE_MAX_CHANNELS) return 0;
  return g_chRawDisp[ch];
}

uint16_t v15DjiRsMeasureGetFrameHz(void) { return g_frameHz; }

uint32_t v15DjiRsMeasureGetWaveSeq(void) { return g_waveSeq; }

uint8_t v15DjiRsMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen)
{
  if (!dst || maxLen == 0) return 0;
  uint8_t n = g_waveLen;
  if (n > maxLen) n = maxLen;
  if (n > DJI_RS_MEASURE_WAVE_BYTES) n = DJI_RS_MEASURE_WAVE_BYTES;
  if (n) memcpy(dst, g_waveBytes, n);
  return n;
}

void v15DjiRsMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
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
  g_flagsRaw = 0;
  g_flags = 0;
  g_frameHz = 0;
  g_framePeriodUs = 0;
  g_lastFrameUs = 0;
  g_lastFrame10ms = get_tmr10ms();

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

  beginPolaritySearch(ETX_Pol_Inverted);
}

void v15DjiRsMeasureStop(void)
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

void v15DjiRsMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  const tmr10ms_t now = get_tmr10ms();

  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastFrame10ms) >= DJI_RS_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    g_frameHz = 0;
    beginPolaritySearch(g_tryPolarity);
    return;
  }

  if (!g_polarityLocked && g_uartCtx) {
    if ((tmr10ms_t)(now - g_polarityTryStart) >= DJI_RS_MEASURE_POLARITY_TRY_10MS) {
      g_tryPolarity = (g_tryPolarity == ETX_Pol_Inverted) ? ETX_Pol_Normal
                                                          : ETX_Pol_Inverted;
      g_polarityTryStart = now;
      startUart(g_tryPolarity);
    }
  }

  if (g_haveSignal) {
    copyLiveToDisplay();
  }
}

#else  // !RADIO_V15

bool v15DjiRsMeasureIsActive(void) { return false; }
bool v15DjiRsMeasureHasSignal(void) { return false; }
bool v15DjiRsMeasureIsPowerOn(void) { return false; }
bool v15DjiRsMeasureOvercurrentFault(void) { return false; }
int16_t v15DjiRsMeasureGetCurrentMa(void) { return 0; }
uint8_t v15DjiRsMeasureGetPolarity(void) { return DJI_RS_MEASURE_POL_SEARCHING; }
uint8_t v15DjiRsMeasureGetFlags(void) { return 0; }
bool v15DjiRsMeasureFailsafe(void) { return false; }
bool v15DjiRsMeasureFrameLost(void) { return false; }
bool v15DjiRsMeasureCh17(void) { return false; }
bool v15DjiRsMeasureCh18(void) { return false; }
uint8_t v15DjiRsMeasureGetChannelCount(void) { return 0; }
uint16_t v15DjiRsMeasureGetChannelUs(uint8_t) { return 0; }
uint16_t v15DjiRsMeasureGetChannelRaw(uint8_t) { return 0; }
uint16_t v15DjiRsMeasureGetFrameHz(void) { return 0; }
uint32_t v15DjiRsMeasureGetWaveSeq(void) { return 0; }
uint8_t v15DjiRsMeasureCopyWaveBytes(uint8_t*, uint8_t) { return 0; }
void v15DjiRsMeasureStart(void) {}
void v15DjiRsMeasureStop(void) {}
void v15DjiRsMeasureTask(void) {}

#endif  // RADIO_V15
