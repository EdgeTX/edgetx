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

#include "dataconstants.h"
#include "myeeprom.h"

#include "pulses/modules_helpers.h"

#include "frsky.h"
#include "io/frsky_sport.h"

extern uint8_t telemetryStreaming; // >0 (true) == data is streaming in. 0 = no data detected for some time

inline bool TELEMETRY_STREAMING()
{
  return telemetryStreaming > 0;
}

enum TelemetryStates {
  TELEMETRY_INIT,
  TELEMETRY_OK,
  TELEMETRY_KO
};
extern uint8_t telemetryState;

constexpr uint8_t TELEMETRY_TIMEOUT10ms = 100; // 1 second

#define TELEMETRY_SERIAL_DEFAULT       0
#define TELEMETRY_SERIAL_8E2           1
#define TELEMETRY_SERIAL_WITHOUT_DMA   2

#if defined(CROSSFIRE) || defined(MULTIMODULE) || defined(AFHDS3) || defined(PXX2)
#define TELEMETRY_RX_PACKET_SIZE       128
// multi module Spektrum telemetry is 18 bytes, FlySky is 37 bytes
#else
#define TELEMETRY_RX_PACKET_SIZE       19  // 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)
#endif

//TODO: remove this public definition
extern uint8_t telemetryRxBuffer[TELEMETRY_RX_PACKET_SIZE];
extern uint8_t telemetryRxBufferCount;

uint8_t* getTelemetryRxBuffer(uint8_t moduleIdx);
uint8_t& getTelemetryRxBufferCount(uint8_t moduleIdx);

// Set alternative telemetry input
void telemetrySetGetByte(void* ctx, int (*fct)(void*, uint8_t*));

// Set telemetry mirror callback
void telemetrySetMirrorCb(void* ctx, void (*fct)(void*, uint8_t));

// Mirror telemetry byte
void telemetryMirrorSend(uint8_t data);

void telemetryWakeup();
void telemetryReset();

void telemetryInterrupt10ms();

void telemetryStart();
void telemetryStop();

struct etx_proto_driver_t;

// Call from ISR to schedule telemetry frame
// processing for that module.
void telemetryFrameTrigger_ISR(uint8_t module, const etx_proto_driver_t* drv);

#define TELEMETRY_AVERAGE_COUNT        3

enum {
  TELEM_CELL_INDEX_LOWEST,
  TELEM_CELL_INDEX_1,
  TELEM_CELL_INDEX_2,
  TELEM_CELL_INDEX_3,
  TELEM_CELL_INDEX_4,
  TELEM_CELL_INDEX_5,
  TELEM_CELL_INDEX_6,
  TELEM_CELL_INDEX_7,
  TELEM_CELL_INDEX_8,
  TELEM_CELL_INDEX_HIGHEST,
  TELEM_CELL_INDEX_DELTA,
  TELEM_CELL_INDEX_LAST = TELEM_CELL_INDEX_DELTA
};

PACK(struct CellValue
{
  uint16_t value:15;
  uint16_t state:1;

  void set(uint16_t newValue)
  {
    if (newValue > 50) {
      value = newValue;
      state = 1;
    }
  }
});

int setTelemetryValue(TelemetryProtocol protocol, uint16_t id, uint8_t subId, uint8_t instance, int32_t value, uint32_t unit, uint32_t prec);
int setTelemetryText(TelemetryProtocol protocol, uint16_t id, uint8_t subId, uint8_t instance, const char * text);
void delTelemetryIndex(uint8_t index);
int availableTelemetryIndex();
int lastUsedTelemetryIndex();

int32_t convertTelemetryValue(int32_t value, uint8_t unit, uint8_t prec, uint8_t destUnit, uint8_t destPrec);

void frskySportSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);
void frskyDSetDefault(int index, uint16_t id);

#define IS_DISTANCE_UNIT(unit)         ((unit) == UNIT_METERS || (unit) == UNIT_FEET)
#define IS_SPEED_UNIT(unit)            ((unit) >= UNIT_KTS && (unit) <= UNIT_MPH)

typedef struct {
  const char *label;
  const char *unit;
} rxStatStruct;

rxStatStruct *getRxStatLabels();

#include "telemetry_sensors.h"

#if defined(LOG_TELEMETRY) && !defined(SIMU)
void logTelemetryWriteStart();
void logTelemetryWriteByte(uint8_t data);
#define LOG_TELEMETRY_WRITE_START()    logTelemetryWriteStart()
#define LOG_TELEMETRY_WRITE_BYTE(data) logTelemetryWriteByte(data)
#else
#define LOG_TELEMETRY_WRITE_START()
#define LOG_TELEMETRY_WRITE_BYTE(data)
#endif
#define TELEMETRY_OUTPUT_BUFFER_SIZE  64

class OutputTelemetryBuffer {
  public:
    OutputTelemetryBuffer()
    {
      reset();
    }

    void setDestination(uint8_t value)
    {
      destination = value;
      timeout = 200; /* 2s */
    }

    bool isModuleDestination(uint8_t module)
    {
      return destination != TELEMETRY_ENDPOINT_NONE && destination != TELEMETRY_ENDPOINT_SPORT && (destination >> 2) == module;
    }

    void per10ms()
    {
      if (timeout > 0) {
        if (--timeout == 0)
          reset();
      }
    }

    void reset()
    {
      destination = TELEMETRY_ENDPOINT_NONE;
      size = 0;
      timeout = 0;
    }

    bool isAvailable()
    {
      return destination == TELEMETRY_ENDPOINT_NONE;
    }

    void pushByte(uint8_t byte)
    {
      if (size < TELEMETRY_OUTPUT_BUFFER_SIZE)
        data[size++] = byte;
    }

    void pushByteWithBytestuffing(uint8_t byte)
    {
      if (byte == 0x7E || byte == 0x7D) {
        pushByte(0x7D);
        pushByte(0x20 ^ byte);
      }
      else {
        pushByte(byte);
      }
    }

    void pushSportPacketWithBytestuffing(SportTelemetryPacket & packet)
    {
      size = 0;
      uint16_t crc = 0;
      pushByte(packet.physicalId); // no bytestuffing, no CRC
      for (uint8_t i=1; i<sizeof(SportTelemetryPacket); i++) {
        uint8_t byte = packet.raw[i];
        pushByteWithBytestuffing(byte);
        crc += byte; // 0-1FF
        crc += crc >> 8; // 0-100
        crc &= 0x00ff;
      }
      pushByteWithBytestuffing(0xFF - crc);
    }

  public:
    union {
      SportTelemetryPacket sport;
      uint8_t data[TELEMETRY_OUTPUT_BUFFER_SIZE];
    };
    uint8_t size;
    uint8_t timeout;
    uint8_t destination;
};

extern OutputTelemetryBuffer outputTelemetryBuffer __DMA;

#if defined(LUA)
#include "fifo.h"
#define LUA_TELEMETRY_INPUT_FIFO_SIZE  256
extern Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE> * luaInputTelemetryFifo;
#endif

void processPXX2Frame(uint8_t idx, const uint8_t* frame,
                      const etx_serial_driver_t* drv, void* ctx);

// Module pulse synchronization
struct ModuleSyncStatus
{
  // feedback input: last received values
  uint16_t  refreshRate; // in us
  int16_t   inputLag;    // in us

  tmr10ms_t lastUpdate;  // in 10ms
  int16_t   currentLag;  // in us
  
  inline bool isValid() const {
    // 2 seconds
    return (get_tmr10ms() - lastUpdate < 200);
  }

  // Set feedback from RF module
  void update(uint16_t newRefreshRate, int16_t newInputLag);

  //mark as timeouted
  void invalidate();

  // Get computed settings for scheduler
  uint16_t getAdjustedRefreshRate();

  // Status string for the UI
  void getRefreshString(char* refreshText);

  ModuleSyncStatus();
};

ModuleSyncStatus& getModuleSyncStatus(uint8_t moduleIdx);
