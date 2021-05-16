/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _TELEMETRY_H_
#define _TELEMETRY_H_

#include "frsky.h"
#include "io/frsky_sport.h"
#include "crossfire.h"
#include "myeeprom.h"
#include "io/frsky_sport.h"
#if defined(GHOST)
  #include "ghost.h"
#endif
#if defined(MULTIMODULE)
  #include "spektrum.h"
  #include "hitec.h"
  #include "hott.h"
  #include "multi.h"
  #include "mlink.h"
#endif
#include "myeeprom.h"
#if defined(MULTIMODULE) || defined(AFHDS3)
  #include "flysky_ibus.h"
#endif

//OW
//extern uint8_t telemetryStreaming; // >0 (true) == data is streaming in. 0 = no data detected for some time
extern uint16_t telemetryStreaming; // >0 (true) == data is streaming in. 0 = no data detected for some time
//OWEND

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

#if defined(CROSSFIRE) || defined(MULTIMODULE) || defined(AFHDS3)
#define TELEMETRY_RX_PACKET_SIZE       128
// multi module Spektrum telemetry is 18 bytes, FlySky is 37 bytes
#else
#define TELEMETRY_RX_PACKET_SIZE       19  // 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)
#endif

extern uint8_t telemetryRxBuffer[TELEMETRY_RX_PACKET_SIZE];
extern uint8_t telemetryRxBufferCount;

#define TELEMETRY_AVERAGE_COUNT        3

enum {
  TELEM_CELL_INDEX_LOWEST,
  TELEM_CELL_INDEX_1,
  TELEM_CELL_INDEX_2,
  TELEM_CELL_INDEX_3,
  TELEM_CELL_INDEX_4,
  TELEM_CELL_INDEX_5,
  TELEM_CELL_INDEX_6,
  TELEM_CELL_INDEX_HIGHEST,
  TELEM_CELL_INDEX_DELTA,
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

extern uint8_t telemetryProtocol;

#if defined (MULTIMODULE)
  #define IS_D16_MULTI(module)           (((g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_FRSKY) && (g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16 || g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_8CH || g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_LBT || g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH || g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_CLONED)) \
                                         || (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_FRSKYX2))
  #define IS_R9_MULTI(module)            (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_FRSKY_R9)
  #define IS_HOTT_MULTI(module)          (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_HOTT)
  #define IS_CONFIG_MULTI(module)        (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_CONFIG)
  #define IS_DSM_MULTI(module)           (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_DSM2)
  #define IS_RX_MULTI(module)            ((g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_AFHDS2A_RX) || (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_FRSKYX_RX) \
                                         || (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_BAYANG_RX) || (g_model.moduleData[module].getMultiProtocol() == MODULE_SUBTYPE_MULTI_DSM_RX))
  #if defined(HARDWARE_INTERNAL_MODULE)
    #define IS_FRSKY_SPORT_PROTOCOL()    (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT || (telemetryProtocol == PROTOCOL_TELEMETRY_MULTIMODULE && (IS_D16_MULTI(INTERNAL_MODULE) || IS_D16_MULTI(EXTERNAL_MODULE) || IS_R9_MULTI(INTERNAL_MODULE) || IS_R9_MULTI(EXTERNAL_MODULE))))
  #else
    #define IS_FRSKY_SPORT_PROTOCOL()    (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT || (telemetryProtocol == PROTOCOL_TELEMETRY_MULTIMODULE && (IS_D16_MULTI(EXTERNAL_MODULE) || IS_R9_MULTI(EXTERNAL_MODULE))))
  #endif
#else
  #define IS_D16_MULTI(module)           false
  #define IS_R9_MULTI(module)            false
  #define IS_HOTT_MULTI(module)          false
  #define IS_CONFIG_MULTI(module)        false
  #define IS_DSM_MULTI(module)           false
  #define IS_FRSKY_SPORT_PROTOCOL()      (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT)
  #define IS_RX_MULTI(module)            false
#endif

#define IS_SPEKTRUM_PROTOCOL()           (telemetryProtocol == PROTOCOL_TELEMETRY_SPEKTRUM)

#if defined(PCBTARANIS) || defined(PCBHORUS)
inline bool isSportLineUsedByInternalModule()
{
  return g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_XJT_PXX1;
}
#else
inline bool isSportLineUsedByInternalModule()
{
  return false;
}
#endif

inline uint8_t modelTelemetryProtocol()
{
  bool sportUsed = isSportLineUsedByInternalModule();

#if defined(CROSSFIRE)
  if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE) {
    return PROTOCOL_TELEMETRY_CROSSFIRE;
  }
#endif

#if defined(GHOST)
  if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_GHOST) {
    return PROTOCOL_TELEMETRY_GHOST;
  }
#endif

  if (!sportUsed && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM) {
    return g_model.telemetryProtocol;
  }

#if defined(MULTIMODULE)
  if (!sportUsed && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_MULTIMODULE) {
    return PROTOCOL_TELEMETRY_MULTIMODULE;
  }
#if defined(INTERNAL_MODULE_MULTI)
  if (g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_MULTIMODULE && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_NONE) {
    return PROTOCOL_TELEMETRY_MULTIMODULE;
  }
#endif
#endif
#if defined(AFHDS3)
  if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_AFHDS3) {
    return PROTOCOL_TELEMETRY_AFHDS3;
  }
#endif
  // default choice
  return PROTOCOL_TELEMETRY_FRSKY_SPORT;
}

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
#define LUA_TELEMETRY_INPUT_FIFO_SIZE  256
extern Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE> * luaInputTelemetryFifo;
#endif

void processPXX2Frame(uint8_t module, const uint8_t *frame);

// Module pulse synchronization
struct ModuleSyncStatus
{
  // feedback input: last received values
  uint16_t  refreshRate; // in us
  int16_t   inputLag;    // in us

  tmr10ms_t lastUpdate;  // in 10ms
  int16_t   currentLag;  // in us
  
  inline bool isValid() {
    // 2 seconds
    return (get_tmr10ms() - lastUpdate < 200);
  }

  // Set feedback from RF module
  void update(uint16_t newRefreshRate, int16_t newInputLag);

  // Get computed settings for scheduler
  uint16_t getAdjustedRefreshRate();

  // Status string for the UI
  void getRefreshString(char* refreshText);

  ModuleSyncStatus();
};

ModuleSyncStatus& getModuleSyncStatus(uint8_t moduleIdx);

#endif // _TELEMETRY_H_
