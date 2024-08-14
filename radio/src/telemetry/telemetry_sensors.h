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

#include "telemetry.h"

constexpr int8_t TELEMETRY_SENSOR_TIMEOUT_UNAVAILABLE = -2;
constexpr int8_t TELEMETRY_SENSOR_TIMEOUT_OLD = -1;
constexpr int8_t TELEMETRY_SENSOR_TIMEOUT_START = 125; // * 160ms = 20s
constexpr uint8_t TELEMETRY_SENSOR_TEXT_LENGTH = 16;

class TelemetryItem
{
  public:
    int32_t  value;           // value, stored as uint32_t but interpreted accordingly to type

    union {
      int32_t valueMin;         // min store
      int32_t pilotLongitude;
    };

    union {
      int32_t valueMax;         // max store
      int32_t pilotLatitude;
    };

    int8_t timeout; // for detection of sensor loss

    union {
      struct {
        int32_t  offsetAuto;
        int32_t  filterValues[TELEMETRY_AVERAGE_COUNT];
      } std;
      struct {
        uint16_t prescale;
      } consumption;
      struct {
        uint8_t   count;
        CellValue values[MAX_CELLS];
      } cells;
      struct {
        uint16_t year;          // full year (4 digits)
        uint8_t  month;
        uint8_t  day;
        uint8_t  hour;
        uint8_t  min;
        uint8_t  sec;
      } datetime;
      struct {
        int32_t latitude;
        int32_t longitude;
        uint32_t pilotDistFromEarthAxis;  // pilot distance from earth axis
                                          // pilot longitude is stored in min
                                          // pilot latitude is stored in max
      } gps;
      char text[TELEMETRY_SENSOR_TEXT_LENGTH];
    };

    TelemetryItem()
    {
      clear();
    }

    void clear()
    {
      memset(reinterpret_cast<void*>(this), 0, sizeof(TelemetryItem));
      timeout = TELEMETRY_SENSOR_TIMEOUT_UNAVAILABLE;
    }

    void eval(const TelemetrySensor & sensor);
    void per10ms(const TelemetrySensor & sensor);

    void setValue(const TelemetrySensor & sensor, const char * newVal, uint32_t unit=UNIT_TEXT, uint32_t prec=0);

    void setValue(const TelemetrySensor & sensor, int32_t newVal, uint32_t unit, uint32_t prec=0);

    inline bool isAvailable()
    {
      return (timeout != TELEMETRY_SENSOR_TIMEOUT_UNAVAILABLE);
    }

    inline bool isOld()
    {
      return (timeout == TELEMETRY_SENSOR_TIMEOUT_OLD);
    }

    inline bool hasReceiveTime()
    {
      return timeout >= 0;
    }

    inline int8_t getDelaySinceLastValue()
    {
      return hasReceiveTime() ? TELEMETRY_SENSOR_TIMEOUT_START - timeout : TELEMETRY_SENSOR_TIMEOUT_OLD;
    }

    inline bool isFresh()
    {
      return TELEMETRY_SENSOR_TIMEOUT_START - timeout <= 1; // 2 * 160ms
    }

    inline void setFresh()
    {
      timeout = TELEMETRY_SENSOR_TIMEOUT_START;
    }

    inline void setOld()
    {
      timeout = TELEMETRY_SENSOR_TIMEOUT_OLD;
    }
};

extern TelemetryItem telemetryItems[MAX_TELEMETRY_SENSORS];
extern uint8_t allowNewSensors;
bool isFaiForbidden(source_t idx);
