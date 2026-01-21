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

#include "../../definitions.h"
#include "telemetry_holders.h"

#include "frsky_defs.h"

// Receive buffer state machine state enum
enum FrSkyDataState {
  STATE_DATA_IDLE,
  STATE_DATA_START,
  STATE_DATA_IN_FRAME,
  STATE_DATA_XOR,
};

#define FRSKY_SPORT_BAUDRATE          57600
#if defined(RADIO_TX16S) || defined(RADIO_F16)
#define FRSKY_TELEM_MIRROR_BAUDRATE   115200
#else
#define FRSKY_TELEM_MIRROR_BAUDRATE   FRSKY_SPORT_BAUDRATE
#endif

#define FRSKY_D_BAUDRATE          9600

inline bool IS_HIDDEN_TELEMETRY_VALUE(uint16_t id)
{
  return (id == SP2UART_A_ID) || (id == SP2UART_B_ID) || (id == XJT_VERSION_ID) || (id == RAS_ID) || (id == FACT_TEST_ID);
}

class TelemetryData {
  public:
    TelemetryExpiringDecorator<TelemetryValue> swrInternal;
    TelemetryExpiringDecorator<TelemetryValue> swrExternal;
    TelemetryFilterDecorator<TelemetryValue> rssi;
    uint16_t xjtVersion;
    uint8_t varioHighPrecision:1;
    uint8_t telemetryValid:3;
    uint8_t spare:4;

    void setSwr(uint8_t module, uint8_t value)
    {
      if (module == 0)
        swrInternal.set(value);
      else
        swrExternal.set(value);
    }

    void clear()
    {
      memset(this, 0, sizeof(*this));
    }
};

extern TelemetryData telemetryData;

inline uint8_t TELEMETRY_RSSI()
{
  return telemetryData.rssi.value();
}

constexpr uint8_t START_STOP    = 0x7E;
constexpr uint8_t BYTE_STUFF    = 0x7D;
constexpr uint8_t STUFF_MASK    = 0x20;
constexpr uint8_t TRAINER_FRAME = 0x80;

typedef enum {
  TS_IDLE = 0,  // waiting for 0x5e frame marker
  TS_DATA_ID,   // waiting for dataID
  TS_DATA_LOW,  // waiting for data low byte
  TS_DATA_HIGH, // waiting for data high byte
  TS_XOR = 0x80 // decode stuffed byte
} TS_STATE;

// FrSky D Telemetry Protocol
void processHubPacket(uint8_t id, int16_t value);
void frskyDProcessPacket(uint8_t module, const uint8_t *packet, uint8_t len);

// FrSky S.PORT Telemetry Protocol
bool sportProcessTelemetryPacket(uint8_t module, const uint8_t * packet, uint8_t len);
void sportProcessTelemetryPacket(uint16_t id, uint8_t subId, uint8_t instance,
                                 uint32_t data, TelemetryUnit unit = UNIT_RAW);

void sportProcessTelemetryPacketWithoutCrc(uint8_t module, uint8_t origin,
                                           const uint8_t *packet);

void processFrskySportTelemetryData(uint8_t module, uint8_t data,
                                    uint8_t* buffer, uint8_t* len);

void processFrskyDTelemetryData(uint8_t module, uint8_t data,
                                uint8_t* buffer, uint8_t* len);

#if defined(NO_RAS)
inline bool isRasValueValid()
{
  return false;
}
#elif defined(PXX2)
inline bool isRasValueValid()
{
  return true;
}
#elif defined(PCBTARANIS)
inline bool isRasValueValid()
{
  return telemetryData.xjtVersion != 0x00FF;
}
#elif defined(PCBHORUS)
inline bool isRasValueValid()
{
  return true;
}
#else
inline bool isRasValueValid()
{
  return false;
}
#endif

constexpr uint8_t FRSKY_BAD_ANTENNA_THRESHOLD = 0x33;
