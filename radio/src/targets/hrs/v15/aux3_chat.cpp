#include "aux3_chat.h"

#if defined(MODULE_XIAOZHI_CHAT)

#include "stm32_gpio.h"
#include "board.h"
#include "debug.h"
#include "hal/serial_port.h"
#include "stm32_serial_driver.h"
#include "stm32_usart_driver.h"

#include <string.h>

namespace {

constexpr uint32_t V15_CHAT_UART_RX_BUFFER_SIZE = 512;
constexpr uint32_t V15_CHAT_UART_TX_BUFFER_SIZE = 256;

static const stm32_usart_t v15ChatUSART = {
  .USARTx = XIAOZHI_UART_USART,
  .txGPIO = XIAOZHI_UART_TX_GPIO,
  .rxGPIO = XIAOZHI_UART_RX_GPIO,
  .IRQn = XIAOZHI_UART_USART_IRQn,
  .IRQ_Prio = 7,
  .txDMA = XIAOZHI_UART_DMA_TX,
  .txDMA_Stream = XIAOZHI_UART_DMA_TX_STREAM,
  .txDMA_Channel = XIAOZHI_UART_DMA_TX_CHANNEL,
  .rxDMA = XIAOZHI_UART_DMA_RX,
  .rxDMA_Stream = XIAOZHI_UART_DMA_RX_STREAM,
  .rxDMA_Channel = XIAOZHI_UART_DMA_RX_CHANNEL,
  .set_input = nullptr,
  .txDMA_IRQn = static_cast<IRQn_Type>(-1),
  .txDMA_IRQ_Prio = 0,
};

DEFINE_STM32_SERIAL_PORT(V15Chat, v15ChatUSART,
                         V15_CHAT_UART_RX_BUFFER_SIZE,
                         V15_CHAT_UART_TX_BUFFER_SIZE);

static const etx_serial_port_t v15ChatPort = {
  .name = "V15-XiaoZhi-UART",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(V15Chat),
  .set_pwr = nullptr,
};

static void *s_chatCtx = nullptr;
static uint32_t s_baudrate = V15_CHAT_UART_DEFAULT_BAUDRATE;
static bool s_hasNewLine = false;
static bool s_wifiScanWaitHit = false;
static char s_latestLine[V15_CHAT_UART_LINE_BUFFER_SIZE] = {0};
static char s_lineAssembly[V15_CHAT_UART_LINE_BUFFER_SIZE] = {0};
static uint32_t s_lineLen = 0;

static inline const etx_serial_driver_t *driver()
{
  return v15ChatPort.uart;
}

static inline bool isDeviceStateLine(const char* text)
{
  // Device FSM lines only — do NOT include WifiStation here.
  return text && (strstr(text, "STATE:") || strstr(text, "State:") ||
                  strstr(text, "state:"));
}

static inline bool isWifiScanWaitLine(const char* text)
{
  return text && strstr(text, "WifiStation") && strstr(text, "Wait for next scan");
}

static inline void saveCompletedLine()
{
  if (s_lineLen == 0) {
    return;
  }

  s_lineAssembly[s_lineLen] = '\0';

  if (isDeviceStateLine(s_lineAssembly)) {
    // Always keep the latest device STATE (idle/listening/speaking).
    // Never let WifiStation logs overwrite this — that left the icon stuck
    // after goodbye / idle when STATE:idle was followed by wifi spam.
    strncpy(s_latestLine, s_lineAssembly, sizeof(s_latestLine) - 1);
    s_latestLine[sizeof(s_latestLine) - 1] = '\0';
    s_hasNewLine = true;
  } else if (isWifiScanWaitLine(s_lineAssembly)) {
    s_wifiScanWaitHit = true;
  }

  s_lineLen = 0;
  s_lineAssembly[0] = '\0';
}

}  // namespace

void v15ChatUartInit(uint32_t baudrate)
{
  if (s_chatCtx) {
    if (baudrate != s_baudrate) {
      v15ChatUartSetBaudrate(baudrate);
    }
    return;
  }

  etx_serial_init params = {
    .baudrate = baudrate,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
  };

  s_chatCtx = driver()->init(v15ChatPort.hw_def, &params);
  if (s_chatCtx) {
    s_baudrate = baudrate;
    v15ChatUartClearRxBuffer();
    TRACE("V15 AUX3 UART ready @ %lu baud", static_cast<unsigned long>(baudrate));
  }
  else {
    TRACE("V15 AUX3 UART init failed");
  }
}

void v15ChatUartDeinit()
{
  if (!s_chatCtx) {
    return;
  }

  driver()->deinit(s_chatCtx);
  s_chatCtx = nullptr;
  s_hasNewLine = false;
  s_lineLen = 0;
  s_latestLine[0] = '\0';
  s_lineAssembly[0] = '\0';
}

bool v15ChatUartIsReady()
{
  return s_chatCtx != nullptr;
}

uint32_t v15ChatUartGetBaudrate()
{
  if (!s_chatCtx || !driver()->getBaudrate) {
    return s_baudrate;
  }

  return driver()->getBaudrate(s_chatCtx);
}

void v15ChatUartSetBaudrate(uint32_t baudrate)
{
  s_baudrate = baudrate;
  if (s_chatCtx && driver()->setBaudrate) {
    driver()->setBaudrate(s_chatCtx, baudrate);
  }
}

void v15ChatUartClearRxBuffer()
{
  if (s_chatCtx && driver()->clearRxBuffer) {
    driver()->clearRxBuffer(s_chatCtx);
  }
  v15ChatUartResetParser();
}

uint32_t v15ChatUartAvailable()
{
  if (!s_chatCtx || !driver()->getBufferedBytes) {
    return 0;
  }

  int count = driver()->getBufferedBytes(s_chatCtx);
  return (count > 0) ? static_cast<uint32_t>(count) : 0;
}

bool v15ChatUartReadByte(uint8_t *data)
{
  if (!s_chatCtx || !data || !driver()->getByte) {
    return false;
  }

  return driver()->getByte(s_chatCtx, data) > 0;
}

uint32_t v15ChatUartRead(uint8_t *data, uint32_t maxLen)
{
  if (!data || maxLen == 0) {
    return 0;
  }

  uint32_t count = 0;
  while (count < maxLen && v15ChatUartReadByte(&data[count])) {
    ++count;
  }
  return count;
}

void v15ChatUartSendByte(uint8_t byte)
{
  if (!s_chatCtx || !driver()->sendByte) {
    return;
  }

  driver()->sendByte(s_chatCtx, byte);
}

void v15ChatUartSendBuffer(const uint8_t *data, uint32_t len)
{
  if (!s_chatCtx || !data || len == 0) {
    return;
  }

  if (driver()->sendBuffer) {
    driver()->sendBuffer(s_chatCtx, data, len);
  }
  else if (driver()->sendByte) {
    for (uint32_t i = 0; i < len; ++i) {
      driver()->sendByte(s_chatCtx, data[i]);
    }
  }
}

void v15ChatUartSendString(const char *text)
{
  if (!text) {
    return;
  }

  v15ChatUartSendBuffer(reinterpret_cast<const uint8_t *>(text), strlen(text));
}

void v15ChatUartWaitTxCompleted()
{
  if (!s_chatCtx || !driver()->waitForTxCompleted) {
    return;
  }

  driver()->waitForTxCompleted(s_chatCtx);
}

enum V15ChatParseState : uint8_t {
  V15_CHAT_WAIT_LEVEL = 0,
  V15_CHAT_WAIT_OPEN_PAREN,
  V15_CHAT_SKIP_PAREN_CONTENT,
  V15_CHAT_WAIT_PAYLOAD_START,
  V15_CHAT_COLLECT_PAYLOAD,
};

static V15ChatParseState s_parseState = V15_CHAT_WAIT_LEVEL;

void v15ChatUartResetParser()
{
  s_parseState = V15_CHAT_WAIT_LEVEL;
  s_lineLen = 0;
  s_lineAssembly[0] = '\0';
  s_hasNewLine = false;
  s_wifiScanWaitHit = false;
  s_latestLine[0] = '\0';
}

bool v15ChatUartPollLine()
{
  // Bound work per call — residual ESP boot/pairing floods must not blow the
  // 10ms timer / menus tick (watchdog → EMERGENCY MODE).
  constexpr uint32_t kMaxBytesPerCall = 256;

  uint8_t byte = 0;
  bool updated = false;
  uint32_t n = 0;

  while (n < kMaxBytesPerCall && v15ChatUartReadByte(&byte)) {
    ++n;

    if (byte == '\r' || byte == '\n') {
      if (s_parseState == V15_CHAT_COLLECT_PAYLOAD && s_lineLen > 0) {
        s_lineAssembly[s_lineLen] = '\0';
        saveCompletedLine();
        updated = s_hasNewLine || updated;
      }
      s_parseState = V15_CHAT_WAIT_LEVEL;
      s_lineLen = 0;
      s_lineAssembly[0] = '\0';
      // Keep draining within the byte budget so a following STATE: idle is not
      // stuck behind an earlier line (ESP-IDF logs are usually \n-terminated).
      continue;
    }

    switch (s_parseState) {
      case V15_CHAT_WAIT_LEVEL:
        if (byte == 'W' || byte == 'I') {
          s_parseState = V15_CHAT_WAIT_OPEN_PAREN;
        }
        break;

      case V15_CHAT_WAIT_OPEN_PAREN:
        if (byte == ' ') {
          break;
        }
        if (byte == '(') {
          s_parseState = V15_CHAT_SKIP_PAREN_CONTENT;
          break;
        }
        s_parseState = (byte == 'W' || byte == 'I') ? V15_CHAT_WAIT_OPEN_PAREN
                                                    : V15_CHAT_WAIT_LEVEL;
        break;

      case V15_CHAT_SKIP_PAREN_CONTENT:
        if (byte == ')') {
          s_parseState = V15_CHAT_WAIT_PAYLOAD_START;
        }
        break;

      case V15_CHAT_WAIT_PAYLOAD_START:
        if (byte == ' ') {
          break;
        }
        s_lineLen = 0;
        s_lineAssembly[0] = '\0';
        s_parseState = V15_CHAT_COLLECT_PAYLOAD;
        // fall through
      case V15_CHAT_COLLECT_PAYLOAD:
        if (s_lineLen < (sizeof(s_lineAssembly) - 1)) {
          s_lineAssembly[s_lineLen++] = static_cast<char>(byte);
        }
        // Overflow: drop further payload bytes (no O(n) memmove per byte)
        break;
    }
  }
  return updated;
}

bool v15ChatUartHasNewLine()
{
  return s_hasNewLine;
}

bool v15ChatUartFetchLatestLine(char *buffer, uint32_t bufferLen, bool clearUpdated)
{
  if (!buffer || bufferLen == 0 || s_latestLine[0] == '\0') {
    return false;
  }

  strncpy(buffer, s_latestLine, bufferLen - 1);
  buffer[bufferLen - 1] = '\0';

  if (clearUpdated) {
    s_hasNewLine = false;
  }

  return true;
}

bool v15ChatUartTakeWifiScanWaitAlert()
{
  if (!s_wifiScanWaitHit) {
    return false;
  }
  s_wifiScanWaitHit = false;
  return true;
}

#endif  // MODULE_XIAOZHI_CHAT

#if !defined(MODULE_XIAOZHI_CHAT)

void v15ChatUartInit(uint32_t baudrate)
{
  (void)baudrate;
}
void v15ChatUartDeinit() {}
bool v15ChatUartIsReady() { return false; }
uint32_t v15ChatUartGetBaudrate() { return 0; }
void v15ChatUartSetBaudrate(uint32_t baudrate)
{
  (void)baudrate;
}
void v15ChatUartClearRxBuffer() {}
void v15ChatUartResetParser() {}
uint32_t v15ChatUartAvailable() { return 0; }
bool v15ChatUartReadByte(uint8_t *data)
{
  (void)data;
  return false;
}

uint32_t v15ChatUartRead(uint8_t *data, uint32_t maxLen)
{
  (void)data;
  (void)maxLen;
  return 0;
}

void v15ChatUartSendByte(uint8_t byte)
{
  (void)byte;
}

void v15ChatUartSendBuffer(const uint8_t *data, uint32_t len)
{
  (void)data;
  (void)len;
}

void v15ChatUartSendString(const char *text)
{
  (void)text;
}
void v15ChatUartWaitTxCompleted() {}
bool v15ChatUartPollLine() { return false; }
bool v15ChatUartHasNewLine() { return false; }
bool v15ChatUartFetchLatestLine(char *buffer, uint32_t bufferLen, bool clearUpdated)
{
  (void)buffer;
  (void)bufferLen;
  (void)clearUpdated;
  return false;
}
bool v15ChatUartTakeWifiScanWaitAlert() { return false; }

#endif
