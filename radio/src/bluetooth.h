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

#pragma once

#include <stdint.h>
#include "dataconstants.h"
#include "popups.h"


enum BluetoothStates {
#if defined(PCBX9E)
  BLUETOOTH_INIT,
  BLUETOOTH_WAIT_TTM,
  BLUETOOTH_WAIT_BAUDRATE_CHANGE,
#endif
  BLUETOOTH_STATE_OFF,
  BLUETOOTH_STATE_FACTORY_BAUDRATE_INIT,
  BLUETOOTH_STATE_BAUDRATE_SENT,
  BLUETOOTH_STATE_BAUDRATE_INIT,
  BLUETOOTH_STATE_NAME_SENT,
  BLUETOOTH_STATE_POWER_SENT,
  BLUETOOTH_STATE_ROLE_SENT,
  BLUETOOTH_STATE_IDLE,
  BLUETOOTH_STATE_DISCOVER_REQUESTED,
  BLUETOOTH_STATE_DISCOVER_SENT,
  BLUETOOTH_STATE_DISCOVER_START,
  BLUETOOTH_STATE_DISCOVER_END,
  BLUETOOTH_STATE_BIND_REQUESTED,
  BLUETOOTH_STATE_CONNECT_SENT,
  BLUETOOTH_STATE_CONNECTED,
  BLUETOOTH_STATE_DISCONNECTED,
  BLUETOOTH_STATE_CLEAR_REQUESTED,
  BLUETOOTH_STATE_FLASH_FIRMWARE,
  BLUETOOTH_STATE_AUDIO_BAUD_UP,    // PCM baud command sent, UART switches next
  BLUETOOTH_STATE_AUDIO_BAUD_DOWN,  // AT+BAUD4 sent at the PCM rate, UART switches back next
  BLUETOOTH_STATE_AUDIO_STOP,
  BLUETOOTH_STATE_BOOT_WAIT,
  BLUETOOTH_STATE_AUDIO_STATS_WAIT,
  BLUETOOTH_STATE_AUDIO_STATS_SENT
};

// A2DP PCM link: mono signed 16-bit LE at AUDIO_SAMPLE_RATE (32 kHz).
// UART speed, same set as the CRSF rates; override per board if needed.
#if !defined(BLUETOOTH_AUDIO_BAUDRATE)
  #define BLUETOOTH_AUDIO_BAUDRATE      921600
#endif
#if BLUETOOTH_AUDIO_BAUDRATE == 921600
  #define BLUETOOTH_AUDIO_BAUD_CMD      "AT+BAUD7"
#elif BLUETOOTH_AUDIO_BAUDRATE == 1870000
  #define BLUETOOTH_AUDIO_BAUD_CMD      "AT+BAUD8"
#elif BLUETOOTH_AUDIO_BAUDRATE == 3750000
  #define BLUETOOTH_AUDIO_BAUD_CMD      "AT+BAUD9"
#elif BLUETOOTH_AUDIO_BAUDRATE == 5250000
  #define BLUETOOTH_AUDIO_BAUD_CMD      "AT+BAUD10"
#else
  #error "BLUETOOTH_AUDIO_BAUDRATE must be 921600, 1870000, 3750000 or 5250000"
#endif
#define BLUETOOTH_AUDIO_RING_SIZE       2048   // power of 2
#define BLUETOOTH_AUDIO_MAX_CHUNK       320    // bytes produced per 5 ms mixer period

#define LEN_BLUETOOTH_ADDR              16
#define MAX_BLUETOOTH_DISTANT_ADDR      6
#define BLUETOOTH_PACKET_SIZE           14
#define BLUETOOTH_LINE_LENGTH           512
#define BLUETOOTH_TRAINER_CHANNELS      8

#if defined(LOG_BLUETOOTH)
  #define BLUETOOTH_TRACE(...)  \
    f_printf(&g_bluetoothFile, __VA_ARGS__); \
    TRACE_NOCRLF(__VA_ARGS__);
#else
#if defined(DEBUG_BLUETOOTH)
  #define BLUETOOTH_TRACE(...)  TRACE_NOCRLF(__VA_ARGS__);
  #define BLUETOOTH_TRACE_TIMESTAMP(f_, ...)  debugPrintf((TRACE_TIME_FORMAT f_), TRACE_TIME_VALUE, ##__VA_ARGS__)
#if defined(DEBUG_BLUETOOTH_VERBOSE)
  #define BLUETOOTH_TRACE_VERBOSE(...) TRACE_NOCRLF(__VA_ARGS__);
#else
  #define BLUETOOTH_TRACE_VERBOSE(...)
#endif
#else
  #define BLUETOOTH_TRACE(...)
  #define BLUETOOTH_TIMESTAMP(f_, ...)
  #define BLUETOOTH_TRACE_VERBOSE(...)
#endif
#endif

class Bluetooth
{
  public:
    void writeString(const char * str);
    char * readline(bool error_reset = true);
    void write(const uint8_t * data, uint8_t length);

    void forwardTelemetry(const uint8_t * packet);
    void wakeup();
    const char * flashFirmware(const char * filename, ProgressHandler progressHandler);

    volatile uint8_t state;
    char localAddr[LEN_BLUETOOTH_ADDR+1];
    char distantAddr[LEN_BLUETOOTH_ADDR+1];

#if defined(BLUETOOTH_A2DP)
    // Sinks found by the last A2DP scan (12 hex digits, NUL terminated)
    char audioDevices[MAX_BLUETOOTH_DISTANT_ADDR][LEN_BLUETOOTH_AUDIO_ADDR + 1];
    volatile uint8_t audioDevicesCount = 0;
    bool audioStatsValid = false;
    uint32_t audioUnderruns = 0;
    uint32_t audioUartDropBytes = 0;
    uint32_t audioUartOverflowEvents = 0;
    uint32_t audioUartFifoOverflowEvents = 0;
    uint32_t audioUartBufferFullEvents = 0;
    uint32_t audioPcmDropBytes = 0;
    uint32_t audioPcmQueueDropBytes = 0;
    uint32_t audioPcmPacketTimeoutBytes = 0;
    uint32_t audioResets = 0;
    uint32_t audioLowWaterBytes = 0;

    // True while PCM is being streamed to the sink
    bool audioActive() const;
    // Queue mixer output (audio_data_t at AUDIO_SAMPLE_RATE) for the sink
    bool audioPush(const void * samples, uint16_t count);
    void audioStartScan();
    void audioSelectDevice(const char * addr);
    void audioForgetDevice();
#endif

  protected:
    void pushByte(uint8_t byte);
    uint8_t read(uint8_t * data, uint8_t size, uint32_t timeout=1000/*ms*/);
    void appendTrainerByte(uint8_t data);
    void processTrainerFrame(const uint8_t * buffer);
    void processTrainerByte(uint8_t data);
    void sendTrainer();
    void receiveTrainer();

    uint8_t bootloaderChecksum(uint8_t command, const uint8_t * data, uint8_t size);
    void bootloaderSendCommand(uint8_t command, const void *data = nullptr, uint8_t size = 0);
    void bootloaderSendCommandResponse(uint8_t response);
    const char * bootloaderWaitCommandResponse(uint32_t timeout=1000/*ms*/);
    const char * bootloaderWaitResponseData(uint8_t *data, uint8_t size);
    const char * bootloaderSetAutoBaud();
    const char * bootloaderReadStatus(uint8_t &status);
    const char * bootloaderCheckStatus();
    const char * bootloaderSendData(const uint8_t * data, uint8_t size);
    const char * bootloaderEraseFlash(uint32_t start, uint32_t size);
    const char * bootloaderStartWriteFlash(uint32_t start, uint32_t size);
    const char * bootloaderWriteFlash(const uint8_t * data, uint32_t size);
    const char * doFlashFirmware(const char * filename, ProgressHandler progressHandler);

#if defined(BLUETOOTH_A2DP)
    bool audioWakeup(tmr10ms_t now);
    void audioPump();
    void audioSetStoredAddr(const char * addr);
    void discardInput();

    volatile bool audioLink = false;  // UART runs at the PCM rate
    bool audioMode = false;           // current init sequence is the A2DP one
    bool audioConnectTried = false;
    uint8_t audioNextState = BLUETOOTH_STATE_OFF;
    uint8_t audioRing[BLUETOOTH_AUDIO_RING_SIZE];
    volatile uint16_t audioRidx = 0;
    volatile uint16_t audioWidx = 0;
#endif

    uint8_t buffer[BLUETOOTH_LINE_LENGTH+1];
    uint16_t bufferIndex = 0;
    tmr10ms_t wakeupTime = 0;
    uint8_t crc;
};

extern Bluetooth bluetooth;
