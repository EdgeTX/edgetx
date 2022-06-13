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
  BLUETOOTH_STATE_FLASH_FIRMWARE
};

#define LEN_BLUETOOTH_ADDR              16
#define MAX_BLUETOOTH_DISTANT_ADDR      6
#define BLUETOOTH_PACKET_SIZE           14
#define BLUETOOTH_LINE_LENGTH           32
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

    uint8_t buffer[BLUETOOTH_LINE_LENGTH+1];
    uint8_t bufferIndex = 0;
    tmr10ms_t wakeupTime = 0;
    uint8_t crc;
};

extern Bluetooth bluetooth;
