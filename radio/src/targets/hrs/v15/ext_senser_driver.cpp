/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "timers_driver.h"

#include "board.h"
#include "hal/usb_driver.h"
#if !defined(BOOT)
#include "edgetx.h"
#include "audio.h"
#include "batsenser.h"
#include "hal/serial_driver.h"
#include "keys.h"
#include "cli.h"
#include "aux3_chat.h"
#include "chat_ui.h"
#endif

#include <string.h>

#if !defined(BOOT)
namespace {

enum : uint8_t {
  V15_AI_MODE_OFF = 0,
  V15_AI_MODE_ON = 1,
  V15_AI_MODE_PAIRING = 2,
  V15_AI_MODE_BOOT = 3,
};

enum V15AiPairingStage : uint8_t {
  PAIR_STAGE_IDLE = 0,
  PAIR_STAGE_WAIT_POWER_ON,
  PAIR_STAGE_WAIT_BOOTCMD_ON,
  PAIR_STAGE_WAIT_BOOTCMD_OFF,
  PAIR_STAGE_MONITOR,
  PAIR_STAGE_WAIT_BOOT_POWER_ON,
};

static uint8_t s_v15AiMode = V15_AI_MODE_OFF;
static V15AiPairingStage s_pairingStage = PAIR_STAGE_IDLE;
static tmr10ms_t s_stageDeadline = 0;

static constexpr uint8_t V15_AI_LOG_QUEUE_SIZE = 8;
static constexpr uint16_t V15_AI_LOG_LINE_LEN = 250;
static char s_logQueue[V15_AI_LOG_QUEUE_SIZE][V15_AI_LOG_LINE_LEN] = {{0}};
static uint8_t s_logHead = 0;
static uint8_t s_logTail = 0;

static char s_uartLine[V15_CHAT_UART_LINE_BUFFER_SIZE] = {0};
static uint16_t s_uartLineLen = 0;
static uint8_t s_aux3LogParseState = 0;  // processAux3RxToLogs FSM
static bool s_usbBridgeEnabled = false;
static bool s_exitWaitReleaseDirect = false;
static tmr10ms_t s_listeningExitPressStart = 0;
static bool s_listeningExitLongPressReady = false;
static tmr10ms_t s_bootCmdPulseStart = 0;
static bool s_bootCmdPulseActive = false;

// ISR-safe ring buffer for USB��AUX3 direction
// Written from USB ISR context, drained from main loop
static constexpr uint32_t USB_TO_AUX3_BUF_SIZE = 4096;
static uint8_t s_usbToAux3Buf[USB_TO_AUX3_BUF_SIZE];
static volatile uint32_t s_usbToAux3Head = 0;
static volatile uint32_t s_usbToAux3Tail = 0;
alignas(4) static uint8_t s_usbToAux3TxChunk[128];

// AUX3->USB buffered path, paced by usbSerialFreeSpace() to avoid CDC TX overwrite.
static constexpr uint32_t AUX3_TO_USB_BUF_SIZE = 4096;
static uint8_t s_aux3ToUsbBuf[AUX3_TO_USB_BUF_SIZE];
static uint32_t s_aux3ToUsbHead = 0;
static uint32_t s_aux3ToUsbTail = 0;

static inline void pushAiLog(const char *text)
{
  if (!text || !text[0]) {
    return;
  }

  strncpy(s_logQueue[s_logTail], text, V15_AI_LOG_LINE_LEN - 1);
  s_logQueue[s_logTail][V15_AI_LOG_LINE_LEN - 1] = '\0';
  s_logTail = (s_logTail + 1) % V15_AI_LOG_QUEUE_SIZE;
  if (s_logTail == s_logHead) {
    s_logHead = (s_logHead + 1) % V15_AI_LOG_QUEUE_SIZE;
  }
}

static inline void applyPowerOff()
{
  gpio_set(EXP_ESP32C3_PWR_GPIO);
  pushAiLog("AI power off");
}

static inline void applyPowerOn()
{
  gpio_clear(EXP_ESP32C3_PWR_GPIO);
  pushAiLog("AI power on");
}

static inline void applyCmdOn()
{
  gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 1);
  pushAiLog("AI cmd key on");
}

static inline void applyCmdOff()
{
  gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
  pushAiLog("AI cmd key off");
}

static void processAux3RxToLogs()
{
  enum ParseState : uint8_t {
    WaitLevel = 0,
    WaitOpenParen,
    SkipParenContent,
    WaitPayloadStart,
    CollectPayload,
  };

  // Bound work per tick — ESP boot floods can otherwise starve the WDT.
  constexpr uint32_t kMaxBytesPerCall = 256;

  if (!v15ChatUartIsReady()) {
    return;
  }

  ParseState parseState = static_cast<ParseState>(s_aux3LogParseState);
  uint8_t byte = 0;
  uint32_t n = 0;
  while (n < kMaxBytesPerCall && v15ChatUartReadByte(&byte)) {
    ++n;
    if (byte == '\r' || byte == '\n') {
      if (parseState == CollectPayload && s_uartLineLen > 0) {
        s_uartLine[s_uartLineLen] = '\0';
        char line[V15_AI_LOG_LINE_LEN];
        strncpy(line, "AUX3: ", sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';
        strncat(line, s_uartLine, sizeof(line) - strlen(line) - 1);
        pushAiLog(line);
      }
      parseState = WaitLevel;
      s_uartLineLen = 0;
      s_uartLine[0] = '\0';
      continue;
    }

    switch (parseState) {
      case WaitLevel:
        if (byte == 'W' || byte == 'I') {
          parseState = WaitOpenParen;
        }
        break;

      case WaitOpenParen:
        if (byte == ' ') {
          break;
        }
        if (byte == '(') {
          parseState = SkipParenContent;
          break;
        }
        parseState = (byte == 'W' || byte == 'I') ? WaitOpenParen : WaitLevel;
        break;

      case SkipParenContent:
        if (byte == ')') {
          parseState = WaitPayloadStart;
        }
        break;

      case WaitPayloadStart:
        if (byte == ' ') {
          break;
        }
        s_uartLineLen = 0;
        s_uartLine[0] = '\0';
        parseState = CollectPayload;
        // fall through
      case CollectPayload:
        if (s_uartLineLen < (sizeof(s_uartLine) - 1)) {
          s_uartLine[s_uartLineLen++] = static_cast<char>(byte);
        }
        // Overflow: drop further payload bytes (no O(n) memmove per byte)
        break;
    }
  }
  s_aux3LogParseState = parseState;
}

// USB RX callback: called from USB ISR context.
// Must NOT call UART TX directly (DMA ops are not ISR-safe).
// Bytes are buffered and drained from the main loop.
static void usbToAux3BridgeRx(uint8_t *data, uint32_t len)
{
  if (!data || len == 0 || s_v15AiMode != V15_AI_MODE_BOOT) {
    return;
  }

  for (uint32_t i = 0; i < len; ++i) {
    uint32_t next = (s_usbToAux3Head + 1) % USB_TO_AUX3_BUF_SIZE;
    if (next != s_usbToAux3Tail) {  // buffer not full
      s_usbToAux3Buf[s_usbToAux3Head] = data[i];
      s_usbToAux3Head = next;
    }
  }
}

// Baud rate passthrough: when the PC tool changes CDC line coding,
// apply the same baud rate to the AUX3 UART so esptool can switch speeds.
static void bridgeBaudrateChanged(uint32_t baudrate)
{
  if (s_v15AiMode == V15_AI_MODE_BOOT && baudrate > 0) {
    v15ChatUartSetBaudrate(baudrate);
  }
}

// Main-loop task: flush buffered USB��AUX3 bytes, and forward AUX3��USB.
static void processAux3ToUsbBridge()
{
  if (!s_usbBridgeEnabled || !UsbSerialPort.uart ||
      !UsbSerialPort.uart->sendByte) {
    return;
  }

  // USB reconnect/deinit can clear CDC callbacks; re-arm to keep passthrough alive.
  if (UsbSerialPort.uart->setReceiveCb) {
    UsbSerialPort.uart->setReceiveCb(nullptr, usbToAux3BridgeRx);
  }
  if (UsbSerialPort.uart->setBaudrateCb) {
    UsbSerialPort.uart->setBaudrateCb(nullptr, bridgeBaudrateChanged);
  }

  // USB→AUX3: drain ISR-filled ring buffer and send as chunks.
  // Chunked TX reduces per-byte overhead and improves esptool stability.
  // Cap chunks/tick so a full USB flood cannot stall menus (WDT risk).
  constexpr uint32_t kMaxTxChunksPerCall = 4;
  uint32_t txChunks = 0;
  while (txChunks < kMaxTxChunksPerCall && s_usbToAux3Tail != s_usbToAux3Head) {
    v15ChatUartWaitTxCompleted();

    uint32_t n = 0;
    while (n < sizeof(s_usbToAux3TxChunk) && s_usbToAux3Tail != s_usbToAux3Head) {
      s_usbToAux3TxChunk[n++] = s_usbToAux3Buf[s_usbToAux3Tail];
      s_usbToAux3Tail = (s_usbToAux3Tail + 1) % USB_TO_AUX3_BUF_SIZE;
    }
    if (n > 0) {
      v15ChatUartSendBuffer(s_usbToAux3TxChunk, n);
      ++txChunks;
    }
  }

  // AUX3→USB stage 1: collect bytes from UART RX DMA into a local ring.
  constexpr uint32_t kMaxRxBytesPerCall = 512;
  uint8_t byte = 0;
  uint32_t nRx = 0;
  while (nRx < kMaxRxBytesPerCall && v15ChatUartReadByte(&byte)) {
    ++nRx;
    uint32_t next = (s_aux3ToUsbHead + 1) % AUX3_TO_USB_BUF_SIZE;
    if (next == s_aux3ToUsbTail) {
      break;  // ring full, keep remaining UART bytes for next cycle
    }
    s_aux3ToUsbBuf[s_aux3ToUsbHead] = byte;
    s_aux3ToUsbHead = next;
  }

  // AUX3��USB stage 2: only send what USB CDC TX ring can safely accept.
  uint32_t free = usbSerialFreeSpace();
  while (free > 0 && s_aux3ToUsbTail != s_aux3ToUsbHead) {
    UsbSerialPort.uart->sendByte(nullptr, s_aux3ToUsbBuf[s_aux3ToUsbTail]);
    s_aux3ToUsbTail = (s_aux3ToUsbTail + 1) % AUX3_TO_USB_BUF_SIZE;
    --free;
  }
}

static void setUsbBridgeEnabled(bool enabled)
{
  if (enabled == s_usbBridgeEnabled) {
    return;
  }

  if (!UsbSerialPort.uart || !UsbSerialPort.uart->setReceiveCb) {
    return;
  }

  if (enabled) {
    // Reset the USB��AUX3 ring buffer
    s_usbToAux3Head = 0;
    s_usbToAux3Tail = 0;
    s_aux3ToUsbHead = 0;
    s_aux3ToUsbTail = 0;

#if defined(CLI)
    // Ensure no CLI/debug traffic is emitted to USB during raw passthrough.
    // Any extra byte on this link can break esptool stub protocol.
    cliSetSerialDriver(nullptr, nullptr);
#endif

    // Override USB receive callback to capture PC��AUX3 data
    UsbSerialPort.uart->setReceiveCb(nullptr, usbToAux3BridgeRx);

    // Override baud-rate callback so esptool speed changes reach AUX3 UART
    if (UsbSerialPort.uart->setBaudrateCb) {
      UsbSerialPort.uart->setBaudrateCb(nullptr, bridgeBaudrateChanged);
    }

    pushAiLog("AI usb bridge on");
  }
  else {
    // Restore CLI ownership of USB serial port
    UsbSerialPort.uart->setReceiveCb(nullptr, nullptr);
    if (UsbSerialPort.uart->setBaudrateCb) {
      UsbSerialPort.uart->setBaudrateCb(nullptr, nullptr);
    }
#if defined(CLI)
    cliSetSerialDriver(nullptr, UsbSerialPort.uart);
#endif
    pushAiLog("AI usb bridge off");
  }

  s_usbBridgeEnabled = enabled;
}

static inline bool deadlineReached()
{
  return static_cast<int32_t>(get_tmr10ms() - s_stageDeadline) >= 0;
}

static void processPairingStateMachine()
{
  if (s_v15AiMode != V15_AI_MODE_PAIRING && s_v15AiMode != V15_AI_MODE_BOOT) {
    return;
  }

  switch (s_pairingStage) {
    case PAIR_STAGE_WAIT_POWER_ON:
      if (deadlineReached()) {
        applyPowerOn();
        s_pairingStage = PAIR_STAGE_WAIT_BOOTCMD_ON;
        s_stageDeadline = get_tmr10ms() + 300;  // 3s
      }
      break;

    case PAIR_STAGE_WAIT_BOOTCMD_ON:
      if (deadlineReached()) {
        applyCmdOn();
        s_pairingStage = PAIR_STAGE_WAIT_BOOTCMD_OFF;
        s_stageDeadline = get_tmr10ms() + 50;  // 0.5s
      }
      break;

    case PAIR_STAGE_WAIT_BOOTCMD_OFF:
      if (deadlineReached()) {
        applyCmdOff();
        s_pairingStage = PAIR_STAGE_MONITOR;
      }
      break;

    case PAIR_STAGE_WAIT_BOOT_POWER_ON:
      if (deadlineReached()) {
        applyPowerOn();
        setUsbBridgeEnabled(true);
        s_pairingStage = PAIR_STAGE_MONITOR;
      }
      break;

    case PAIR_STAGE_MONITOR:
    case PAIR_STAGE_IDLE:
    default:
      break;
  }
}

}  // namespace
#endif

void hr_exesenserInit()
{
  gpio_init(EXP_ESP32C3_PAEN, GPIO_IN_PU, GPIO_PIN_SPEED_LOW); // Audio PAEN
  gpio_init(EXP_ESP32C3_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#if defined(BOOT)
  gpio_set(EXP_ESP32C3_PWR_GPIO);    // 1 -> power off in bootloader
#else
  gpio_set(EXP_ESP32C3_PWR_GPIO);    // 1 -> power off in bootloader
  //gpio_clear(EXP_ESP32C3_PWR_GPIO);  // 0 -> power on in firmware
#endif
  gpio_init(EXP_ESP32C3_BOOTCMD_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(EXP_ESP32C3_BOOTCMD_GPIO);
  //gpio_set(EXP_ESP32C3_BOOTCMD_GPIO); //0->esp32c3 en   1->esp32c3 boot

#if defined(AUDIO_SELECT_GPIO)
  gpio_init(AUDIO_SELECT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  //gpio_clear(AUDIO_SELECT_GPIO);
  gpio_set(AUDIO_SELECT_GPIO); //0->Ai  1->Edgetx
#endif
#if defined(EXT_UART_POWER)

  gpio_init(XIAOZHI_UART_TX_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(XIAOZHI_UART_RX_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(XIAOZHI_UART_TX_GPIO );
  gpio_clear(XIAOZHI_UART_RX_GPIO );

  gpio_init(EXT_UART_POWER, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  //gpio_set(EXT_UART_POWER);
  gpio_clear(EXT_UART_POWER );  //EXT POWER  1=ENANLE 0=DISABLE 
#endif
#if defined(CHIP_CS1_GPIO)
  gpio_init(CHIP_CS1_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(CHIP_CS2_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(CHIP_CS3_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(CHIP_CS1_GPIO);
  gpio_set(CHIP_CS2_GPIO);
  gpio_set(CHIP_CS3_GPIO);      //SELECT VBAT

 gpio_init(CHIP_FUN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
 gpio_set(CHIP_FUN_GPIO);       //SELECT UART
 //gpio_clear(CHIP_FUN_GPIO);       //SELECT VBAT SENSER
#endif

#if !defined(BOOT)
  v15BatterySensorInit();
#endif
}

#if !defined(BOOT) && defined(AUDIO_MUTE_GPIO)
// Match audio_dac_driver mute polarity (INVERTED_MUTE_PIN on V15).
static inline void v15AudioAmpSetMuted(bool muted)
{
#if defined(INVERTED_MUTE_PIN)
  muted = !muted;
#endif
  gpio_write(AUDIO_MUTE_GPIO, muted);
}
#endif

void hr_exesenserTask()
{
#if defined(AUDIO_SELECT_GPIO) && defined(EXP_ESP32C3_PAEN)
  static bool aiRouteEnabled = false;
#if !defined(BOOT) && defined(AUDIO_MUTE_GPIO)
  // After mute enable, amp may be off; wait AUDIO_UNMUTE_DELAY before AI select.
  static bool aiUnmutePending = false;
  static tmr10ms_t aiUnmuteStart = 0;
#endif
  const bool hasAiVoice = gpio_read(EXP_ESP32C3_PAEN);
  bool systemAudioActive = false;

#if !defined(BOOT) && defined(AUDIO)
  systemAudioActive =
      !audioQueue.isEmpty() ||
      (audioQueue.buffersFifo.getNextFilledBuffer() != nullptr);
#endif

#if !defined(BOOT)
  const bool enableAiRoute =
      (s_v15AiMode != V15_AI_MODE_OFF) && hasAiVoice && !systemAudioActive;
#else
  const bool enableAiRoute = hasAiVoice && !systemAudioActive;
#endif

  if (!enableAiRoute) {
#if !defined(BOOT) && defined(AUDIO_MUTE_GPIO)
    aiUnmutePending = false;
#endif
    if (aiRouteEnabled) {
      aiRouteEnabled = false;
      gpio_set(AUDIO_SELECT_GPIO);  // 1 -> EdgeTX route
    }
  } else if (!aiRouteEnabled) {
#if !defined(BOOT) && defined(AUDIO_MUTE_GPIO)
    // Special function: keep amp muted — do not steal the speaker for AI.
    if (isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
      aiUnmutePending = false;
    } else if (!aiUnmutePending) {
      // Unmute amp first, then switch route after AUDIO_UNMUTE_DELAY.
      v15AudioAmpSetMuted(false);
#if defined(AUDIO_UNMUTE_DELAY)
      audioQueue.lastAudioPlayTime = 0;
#endif
      aiUnmutePending = true;
      aiUnmuteStart = get_tmr10ms();
    } else {
      const tmr10ms_t unmuteDelayTicks =
#if defined(AUDIO_UNMUTE_DELAY)
          (AUDIO_UNMUTE_DELAY / 10);
#else
          0;
#endif
      if ((tmr10ms_t)(get_tmr10ms() - aiUnmuteStart) >= unmuteDelayTicks) {
        gpio_clear(AUDIO_SELECT_GPIO);  // 0 -> AI route
        aiRouteEnabled = true;
        aiUnmutePending = false;
      }
    }
#else
    gpio_clear(AUDIO_SELECT_GPIO);  // 0 -> AI route
    aiRouteEnabled = true;
#endif
  } else {
#if !defined(BOOT) && defined(AUDIO_MUTE_GPIO)
    // Hold amp open while AI speaks; otherwise EdgeTX auto-mute remutes
    // because the DAC queue is empty during XiaoZhi playback.
    if (!isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP) &&
        g_eeGeneral.audioMuteEnable) {
      v15AudioAmpSetMuted(false);
#if defined(AUDIO_UNMUTE_DELAY)
      audioQueue.lastAudioPlayTime = 0;
#endif
    }
#endif
  }
#endif

#if defined(EXP_ESP32C3_BOOTCMD_GPIO)
#if !defined(BOOT)
  processPairingStateMachine();
  if (s_v15AiMode == V15_AI_MODE_BOOT) {
    processAux3ToUsbBridge();
  }
  if (s_v15AiMode == V15_AI_MODE_PAIRING) {
    processAux3RxToLogs();
  }

  if (s_v15AiMode != V15_AI_MODE_PAIRING && s_v15AiMode != V15_AI_MODE_BOOT) {
    // Control BOOTCMD_GPIO directly in REST/ANSWERING; require 2s hold in LISTENING.
    const uint8_t chatState = v15ChatUiGetVisualState();
    if (chatState == V15_CHAT_UI_STATE_LISTENING ||
        chatState == V15_CHAT_UI_STATE_ANSWERING) {
      s_listeningExitPressStart = 0;
      s_listeningExitLongPressReady = false;
      s_bootCmdPulseStart = 0;
      s_bootCmdPulseActive = false;
      const bool rtnPressed = keysGetState(KEY_EXIT);
      if (s_exitWaitReleaseDirect) {
        if (!rtnPressed) {
          s_exitWaitReleaseDirect = false;
        }
        //gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
      }
      else if (rtnPressed) {
        gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 1);
      }
      else {
        gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
      }
    }
    else if (chatState == V15_CHAT_UI_STATE_REST) {
      const bool rtnPressed = keysGetState(KEY_EXIT);

      if (rtnPressed) {
        if (s_listeningExitPressStart == 0) {
          s_listeningExitPressStart = get_tmr10ms();
        }
        else if (!s_listeningExitLongPressReady &&
                 static_cast<int32_t>(get_tmr10ms() - s_listeningExitPressStart) >=
                     100) {
          s_listeningExitLongPressReady = true;
          s_exitWaitReleaseDirect = true;
        }
      }
      else {
        s_listeningExitPressStart = 0;
        s_listeningExitLongPressReady = false;
        s_bootCmdPulseStart = 0;
        s_bootCmdPulseActive = false;
        gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
      }

      if (rtnPressed && s_listeningExitLongPressReady && !s_bootCmdPulseActive) {
        gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 1);
        s_bootCmdPulseActive = true;
        s_bootCmdPulseStart = get_tmr10ms();
        s_listeningExitLongPressReady = false;
        s_listeningExitPressStart = get_tmr10ms();
      }

      if (s_bootCmdPulseActive &&
          static_cast<int32_t>(get_tmr10ms() - s_bootCmdPulseStart) >= 20) {
        gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
        s_bootCmdPulseActive = false;
      }
    }
    else {
      s_exitWaitReleaseDirect = false;
      s_listeningExitPressStart = 0;
      s_listeningExitLongPressReady = false;
      s_bootCmdPulseStart = 0;
      s_bootCmdPulseActive = false;
      gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 0);
    }
  }
#else
  const bool rtnPressed = !gpio_read(GPIO_PIN(GPIOD, 5));
  gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, rtnPressed ? 1 : 0);
#endif
#endif
}

#if !defined(BOOT)
uint8_t v15AiModeGet()
{
  return s_v15AiMode;
}

void v15AiModeApply(uint8_t mode)
{
  if (mode > V15_AI_MODE_BOOT) {
    mode = V15_AI_MODE_OFF;
  }

  const uint8_t prevMode = s_v15AiMode;

  if (s_v15AiMode == V15_AI_MODE_BOOT && mode != V15_AI_MODE_BOOT) {
    setUsbBridgeEnabled(false);
  }

  s_v15AiMode = mode;
  s_pairingStage = PAIR_STAGE_IDLE;
  s_uartLineLen = 0;
  s_uartLine[0] = '\0';
  s_aux3LogParseState = 0;
  s_logHead = 0;
  s_logTail = 0;

  // Always flush RX + chat parser on mode change so Pairing boot spam cannot
  // race into AI On / Off UI paths (chat overlay / terminal teardown).
  if (v15ChatUartIsReady()) {
    v15ChatUartClearRxBuffer();
  }

  if (mode == V15_AI_MODE_OFF) {
    applyPowerOff();
    // Close pairing/boot terminal if Radio Setup left it open on MainWindow.
    v15AiTerminalUiClose();
  }
  else if (mode == V15_AI_MODE_ON) {
    applyPowerOn();
    // Leaving Pairing/Boot → On: drop terminal so main view + chat overlay
    // do not fight the same UART flood UI path.
    if (prevMode == V15_AI_MODE_PAIRING || prevMode == V15_AI_MODE_BOOT) {
      v15AiTerminalUiClose();
    }
  }
  else {
    if (mode == V15_AI_MODE_PAIRING) {
      applyPowerOff();
      s_pairingStage = PAIR_STAGE_WAIT_POWER_ON;
      s_stageDeadline = get_tmr10ms() + 100;  // 1s
    }
    else {
      applyPowerOff();
      gpio_write(EXP_ESP32C3_BOOTCMD_GPIO, 1);
      pushAiLog("AI cmd key off");
      s_pairingStage = PAIR_STAGE_WAIT_BOOT_POWER_ON;
      s_stageDeadline = get_tmr10ms() + 100;  // 1s
    }
  }
}

bool v15AiTerminalFetchLine(char *buffer, uint32_t bufferLen)
{
  if (!buffer || bufferLen == 0 || s_logHead == s_logTail) {
    return false;
  }

  strncpy(buffer, s_logQueue[s_logHead], bufferLen - 1);
  buffer[bufferLen - 1] = '\0';
  s_logHead = (s_logHead + 1) % V15_AI_LOG_QUEUE_SIZE;
  return true;
}

bool v15AiTerminalIsActive()
{
  return s_v15AiMode == V15_AI_MODE_PAIRING || s_v15AiMode == V15_AI_MODE_BOOT;
}

#if !defined(MODULE_XIAOZHI_CHAT)
// radio_setup provides the real singleton when MODULE_XIAOZHI_CHAT is on.
void v15AiTerminalUiClose() {}
void v15AiTerminalUiOpen() {}
#endif
#endif

