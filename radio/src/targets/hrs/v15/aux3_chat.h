#pragma once

#include <stdbool.h>
#include <stdint.h>

// Constants are always available, regardless of module enable/disable
constexpr uint32_t V15_CHAT_UART_DEFAULT_BAUDRATE = 115200;
constexpr uint32_t V15_CHAT_UART_LINE_BUFFER_SIZE = 1024;

// V15-only text UART interface for the smart chat module connected to AUX3/UART5.
// Later the UI can call `v15ChatUartPollLine()` periodically and fetch the latest
// complete line with `v15ChatUartGetLatestLine()` for bottom-line scrolling.

void v15ChatUartInit(uint32_t baudrate = V15_CHAT_UART_DEFAULT_BAUDRATE);
void v15ChatUartDeinit();
bool v15ChatUartIsReady();

uint32_t v15ChatUartGetBaudrate();
void v15ChatUartSetBaudrate(uint32_t baudrate);

void v15ChatUartClearRxBuffer();
void v15ChatUartResetParser();
uint32_t v15ChatUartAvailable();
bool v15ChatUartReadByte(uint8_t *data);
uint32_t v15ChatUartRead(uint8_t *data, uint32_t maxLen);

void v15ChatUartSendByte(uint8_t byte);
void v15ChatUartSendBuffer(const uint8_t *data, uint32_t len);
void v15ChatUartSendString(const char *text);
void v15ChatUartWaitTxCompleted();

// Consume buffered UART data and assemble CR/LF-terminated text lines.
// Returns true when a new complete line has just been captured.
// Work is bounded per call to avoid starving the watchdog under RX flood.
bool v15ChatUartPollLine();

bool v15ChatUartHasNewLine();
bool v15ChatUartFetchLatestLine(char *buffer, uint32_t bufferLen, bool clearUpdated = true);
// WifiStation "Wait for next scan" is tracked separately so it cannot overwrite
// a pending STATE: idle/listening/speaking line (icon would stick on screen).
bool v15ChatUartTakeWifiScanWaitAlert();
