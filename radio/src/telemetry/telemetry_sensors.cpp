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

#include "opentx.h"
#define _USE_MATH_DEFINES
#include <math.h>

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

#include "spektrum.h"

#if defined(CROSSFIRE)
  #include "crossfire.h"
#endif

#if defined(GHOST)
  #include "ghost.h"
#endif

#if defined(PCBNV14)
  #include "telemetry/flysky_nv14.h"
#endif

#if defined(MULTIMODULE)
  #include "hitec.h"
  #include "hott.h"
  #include "multi.h"
  #include "mlink.h"
#endif

#if defined(MULTIMODULE) || defined(AFHDS2) || defined(AFHDS3)
  #include "flysky_ibus.h"
#endif

TelemetryItem telemetryItems[MAX_TELEMETRY_SENSORS];
uint8_t allowNewSensors;

bool isFaiForbidden(source_t idx)
{
  if (idx < MIXSRC_FIRST_TELEM) {
    return false;
  }

  TelemetrySensor * sensor = &g_model.telemetrySensors[(idx-MIXSRC_FIRST_TELEM)/3];

  switch (telemetryProtocol) {
    case PROTOCOL_TELEMETRY_FRSKY_SPORT:
      if (sensor->id == RSSI_ID) {
        return false;
      }
      else if (sensor->id == BATT_ID) {
        return false;
      }
      break;

    case PROTOCOL_TELEMETRY_FRSKY_D:
      if (sensor->id == D_RSSI_ID) {
        return false;
      }
      else if (sensor->id == D_A1_ID) {
        return false;
      }
      break;

#if defined(CROSSFIRE)
    case PROTOCOL_TELEMETRY_CROSSFIRE:
      if (sensor->id == RX_RSSI1_INDEX) {
        return false;
      }
      else if (sensor->id == RX_RSSI2_INDEX) {
        return false;
      }
      else if (sensor->id == BATT_VOLTAGE_INDEX) {
        return false;
      }
      break;
#endif
  }
  return true;
}

// TODO in maths
uint32_t getDistFromEarthAxis(int32_t latitude)
{
  uint32_t lat = abs(latitude) / 10000;
  uint32_t angle2 = (lat * lat) / 10000;
  uint32_t angle4 = angle2 * angle2;
  return 139*(((uint32_t)10000000 - ((angle2*(uint32_t)123370)/81) + (angle4/25))/12500);
}

void TelemetryItem::setValue(const TelemetrySensor & sensor, const char * val, uint32_t, uint32_t)
{
  strncpy(text, val, sizeof(text));
  setFresh();
}

void TelemetryItem::setValue(const TelemetrySensor & sensor, int32_t val, uint32_t unit, uint32_t prec)
{
  int32_t newVal = val;

  if (prec == 255) {
    prec = sensor.prec;
  }

  if (unit == UNIT_CELLS) {
    auto data = uint32_t(newVal);
    uint8_t cellsCount = (data >> 24u);
    uint8_t cellIndex = ((data >> 16u) & 0x0Fu);
    if (cellIndex >= MAX_CELLS)
      return;
    uint16_t cellValue = (data & 0xFFFFu);
    if (cellsCount == 0) {
      cellsCount = (cellIndex >= cells.count ? cellIndex + 1 : cells.count);
      if (cellsCount != cells.count) {
        clear();
        cells.count = cellsCount;
        // we skip this round as we are not sure we received all cells values
        return;
      }
    }
    else if (cellsCount != cells.count) {
      clear();
      cells.count = cellsCount;
    }
    cells.values[cellIndex].set(cellValue);
    if (cellIndex+1 == cells.count) {
      newVal = 0;
      for (int i=0; i<cellsCount; i++) {
        if (cells.values[i].state) {
          newVal += cells.values[i].value;
        }
        else {
          // we didn't receive all cells values
          return;
        }
      }
      newVal = sensor.getValue(newVal, UNIT_VOLTS, 2);
    }
    else {
      // we didn't receive all cells values
      return;
    }
  }
  else if (unit == UNIT_DATETIME) {
    auto data = uint32_t(newVal);
    if (data & 0x000000ff) {
      datetime.year = (uint16_t) ((data & 0xff000000) >> 24) + 2000;  // SPORT GPS year is only two digits
      datetime.month = (uint8_t) ((data & 0x00ff0000) >> 16);
      datetime.day = (uint8_t) ((data & 0x0000ff00) >> 8);
    }
    else {
      datetime.hour = (uint8_t) ((data & 0xff000000) >> 24);
      datetime.min = (uint8_t) ((data & 0x00ff0000) >> 16);
      datetime.sec = (uint8_t) ((data & 0x0000ff00) >> 8);
#if defined(RTCLOCK)
      if (g_eeGeneral.adjustRTC) {
        rtcAdjust(datetime.year, datetime.month, datetime.day, datetime.hour, datetime.min, datetime.sec);
      }
#endif
    }
    newVal = 0;
  }
  else if (unit == UNIT_GPS_LATITUDE) {
#if defined(INTERNAL_GPS)
    if (gpsData.fix  && gpsData.hdop < PILOTPOS_MIN_HDOP) {
      pilotLatitude = gpsData.latitude;
      distFromEarthAxis = getDistFromEarthAxis(pilotLatitude);
    }
#endif
    if (!pilotLatitude) {
      pilotLatitude = newVal;
      distFromEarthAxis = getDistFromEarthAxis(newVal);
    }
    gps.latitude = newVal;
    setFresh();
    return;
  }
  else if (unit == UNIT_GPS_LONGITUDE) {
#if defined(INTERNAL_GPS)
    if (gpsData.fix && gpsData.hdop < PILOTPOS_MIN_HDOP) {
      pilotLongitude = gpsData.longitude;
    }
#endif
    if (!pilotLongitude) {
      pilotLongitude = newVal;
    }
    gps.longitude = newVal;
    setFresh();
    return;
  }
  else if (unit == UNIT_DATETIME_YEAR) {
    datetime.year = newVal;
    return;
  }
  else if (unit == UNIT_DATETIME_DAY_MONTH) {
    auto data = uint32_t(newVal);
    datetime.month = data >> 8u;
    datetime.day = data & 0xFFu;
    return;
  }
  else if (unit == UNIT_DATETIME_HOUR_MIN) {
    auto data = uint32_t(newVal);
    datetime.hour = (data & 0xFFu);
    datetime.min = data >> 8u;
    return;
  }
  else if (unit == UNIT_DATETIME_SEC) {
    datetime.sec = newVal & 0xFFu;
    newVal = 0;
  }
  else if (unit == UNIT_RPMS) {
    if (sensor.custom.ratio != 0) {
      newVal = (newVal * sensor.custom.offset) / sensor.custom.ratio;
    }
  }
  else if (unit == UNIT_TEXT) {
    // Should be handled at telemetry protocol level
    return;
  }
  else {
    newVal = sensor.getValue(newVal, unit, prec);
    if (sensor.autoOffset) {
      if (!isAvailable()) {
        std.offsetAuto = -newVal;
      }
      newVal += std.offsetAuto;
    }
    if (sensor.filter) {
      if (!isAvailable()) {
        for (int i=0; i<TELEMETRY_AVERAGE_COUNT; i++) {
          std.filterValues[i] = newVal;
        }
      }
      else {
        // Calculate the average from values[] and value,
        // also shift readings in values [] array.
        // There is a possibility of value overflow in `sum` but
        // in reality no sensor value should be so big to cause it.
        int32_t sum = std.filterValues[0];
        for (int i=0; i<TELEMETRY_AVERAGE_COUNT-1; i++) {
          int32_t tmp = std.filterValues[i+1];
          std.filterValues[i] = tmp;
          sum += tmp;
        }
        std.filterValues[TELEMETRY_AVERAGE_COUNT-1] = newVal;
        sum += newVal;
        newVal = sum/(TELEMETRY_AVERAGE_COUNT+1);
      }
    }
  }

  if (!isAvailable()) {
    valueMin = newVal;
    valueMax = newVal;
  }
  else if (newVal < valueMin) {
    valueMin = newVal;
  }
  else if (newVal > valueMax) {
    valueMax = newVal;
    if (sensor.unit == UNIT_VOLTS) {
      valueMin = newVal; // the batt was changed
    }
  }

  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    TelemetrySensor & it = g_model.telemetrySensors[i];
    if (it.type == TELEM_TYPE_CALCULATED && it.formula == TELEM_FORMULA_TOTALIZE && &g_model.telemetrySensors[it.consumption.source-1] == &sensor) {
      TelemetryItem & item = telemetryItems[i];
      int32_t increment = it.getValue(val, unit, prec);
      item.setValue(it, item.value+increment, it.unit, it.prec);
    }
  }

  value = newVal;
  setFresh();
}

void TelemetryItem::per10ms(const TelemetrySensor & sensor)
{
  switch (sensor.formula) {
    case TELEM_FORMULA_CONSUMPTION:
      if (sensor.consumption.source) {
        TelemetrySensor & currentSensor = g_model.telemetrySensors[sensor.consumption.source-1];
        TelemetryItem & currentItem = telemetryItems[sensor.consumption.source-1];
        if (!currentItem.isAvailable()) {
          return;
        }
        else if (currentItem.isOld()) {
          setOld();
          return;
        }
        int32_t current = convertTelemetryValue(currentItem.value, currentSensor.unit, currentSensor.prec, UNIT_AMPS, 1);
        currentItem.consumption.prescale += current;
        if (currentItem.consumption.prescale >= 3600) {
          currentItem.consumption.prescale -= 3600;
          setValue(sensor, value+1, sensor.unit, sensor.prec);
        }
        setFresh();
      }
      break;

    default:
      break;
  }
}

void TelemetryItem::eval(const TelemetrySensor & sensor)
{
  switch (sensor.formula) {
    case TELEM_FORMULA_CELL:
      if (sensor.cell.source) {
        TelemetryItem & cellsItem = telemetryItems[sensor.cell.source-1];
        if (cellsItem.isOld()) {
          setOld();
        }
        else {
          unsigned int index = sensor.cell.index;
          if (index == TELEM_CELL_INDEX_LOWEST || index == TELEM_CELL_INDEX_HIGHEST || index == TELEM_CELL_INDEX_DELTA) {
            unsigned int lowest=0, highest=0;
            for (int i=0; i<cellsItem.cells.count; i++) {
              if (cellsItem.cells.values[i].state) {
                if (!lowest || cellsItem.cells.values[i].value < cellsItem.cells.values[lowest-1].value)
                  lowest = i+1;
                if (!highest || cellsItem.cells.values[i].value > cellsItem.cells.values[highest-1].value)
                  highest = i+1;
              }
              else {
                lowest = highest = 0;
              }
            }
            if (lowest) {
              switch (index) {
                case TELEM_CELL_INDEX_LOWEST:
                  setValue(sensor, cellsItem.cells.values[lowest-1].value, UNIT_VOLTS, 2);
                  break;
                case TELEM_CELL_INDEX_HIGHEST:
                  setValue(sensor, cellsItem.cells.values[highest-1].value, UNIT_VOLTS, 2);
                  break;
                case TELEM_CELL_INDEX_DELTA:
                  setValue(sensor, cellsItem.cells.values[highest-1].value - cellsItem.cells.values[lowest-1].value, UNIT_VOLTS, 2);
                  break;
              }
            }
          }
          else {
            index -= 1;
            if (index < cellsItem.cells.count && cellsItem.cells.values[index].state) {
              setValue(sensor, cellsItem.cells.values[index].value, UNIT_VOLTS, 2);
            }
          }
        }
      }
      break;

    case TELEM_FORMULA_DIST:
      if (sensor.dist.gps) {
        TelemetryItem gpsItem = telemetryItems[sensor.dist.gps-1];
        TelemetryItem * altItem = nullptr;
        if (!gpsItem.isAvailable()) {
          return;
        }
        else if (gpsItem.isOld()) {
          setOld();
          return;
        }
        if (sensor.dist.alt) {
          altItem = &telemetryItems[sensor.dist.alt-1];
          if (!altItem->isAvailable()) {
            return;
          }
          else if (altItem->isOld()) {
            setOld();
            return;
          }
        }
        uint32_t angle = abs(gpsItem.gps.latitude - gpsItem.pilotLatitude);
        uint32_t dist = uint64_t(EARTH_RADIUS * M_PI / 180) * angle / 1000000;
        uint32_t result = dist * dist;

        angle = abs(gpsItem.gps.longitude - gpsItem.pilotLongitude);
        dist = uint64_t(gpsItem.distFromEarthAxis) * angle / 1000000;
        result += dist * dist;

        // Length on ground (ignoring curvature of the earth)
        result = isqrt32(result);

        if (altItem) {
          dist = convertTelemetryValue(abs(altItem->value), g_model.telemetrySensors[sensor.dist.alt-1].unit, g_model.telemetrySensors[sensor.dist.alt-1].prec, UNIT_METERS, 0);
          result = (dist * dist) + (result * result);
          result = isqrt32(result);
        }

        setValue(sensor, result, UNIT_METERS);
      }
      break;

    case TELEM_FORMULA_ADD:
    case TELEM_FORMULA_AVERAGE:
    case TELEM_FORMULA_MIN:
    case TELEM_FORMULA_MAX:
    case TELEM_FORMULA_MULTIPLY:
    {
      int32_t value=0, count=0, available=0, maxitems=4, mulprec=0;
      if (sensor.formula == TELEM_FORMULA_MULTIPLY) {
        maxitems = 2;
        value = 1;
      }
      for (int i=0; i<maxitems; i++) {
        int8_t source = sensor.calc.sources[i];
        if (source) {
          unsigned int index = abs(source)-1;
          TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
          TelemetryItem & telemetryItem = telemetryItems[index];
          if (sensor.formula == TELEM_FORMULA_AVERAGE) {
            if (telemetryItem.isAvailable())
              available = 1;
            else
              continue;
            if (telemetryItem.isOld())
              continue;
          }
          else {
            if (!telemetryItem.isAvailable()) {
              return;
            }
            else if (telemetryItem.isOld()) {
              setOld();
              return;
            }
          }
          int32_t sensorValue = telemetryItem.value;
          if (source < 0)
            sensorValue = -sensorValue;
          count += 1;
          if (sensor.formula == TELEM_FORMULA_MULTIPLY) {
            mulprec += telemetrySensor.prec;
            value *= convertTelemetryValue(sensorValue, telemetrySensor.unit, 0, sensor.unit, 0);
          }
          else {
            sensorValue = convertTelemetryValue(sensorValue, telemetrySensor.unit, telemetrySensor.prec, sensor.unit, sensor.prec);
            if (sensor.formula == TELEM_FORMULA_MIN)
              value = (count==1 ? sensorValue : min<int32_t>(value, sensorValue));
            else if (sensor.formula == TELEM_FORMULA_MAX)
              value = (count==1 ? sensorValue : max<int32_t>(value, sensorValue));
            else
              value += sensorValue;
          }
        }
      }
      if (sensor.formula == TELEM_FORMULA_AVERAGE) {
        if (count == 0) {
          if (available)
            setOld();
          return;
        }
        else {
          value = (value + count/2) / count;
        }
      }
      else if (sensor.formula == TELEM_FORMULA_MULTIPLY) {
        if (count == 0)
          return;
        value = convertTelemetryValue(value, sensor.unit, mulprec, sensor.unit, sensor.prec);
      }
      setValue(sensor, value, sensor.unit, sensor.prec);
      break;
    }

    default:
      break;
  }
}

void delTelemetryIndex(uint8_t index)
{
  memclear(&g_model.telemetrySensors[index], sizeof(TelemetrySensor));
  telemetryItems[index].clear();
  storageDirty(EE_MODEL);
}

int availableTelemetryIndex()
{
  for (int index=0; index<MAX_TELEMETRY_SENSORS; index++) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    if (!telemetrySensor.isAvailable()) {
      return index;
    }
  }
  return -1;
}

int lastUsedTelemetryIndex()
{
  for (int index=MAX_TELEMETRY_SENSORS-1; index>=0; index--) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    if (telemetrySensor.isAvailable()) {
      return index;
    }
  }
  return -1;
}

template <class T>
int setTelemetryValue(TelemetryProtocol protocol, uint16_t id, uint8_t subId, uint8_t instance, T value, uint32_t unit = 0, uint32_t prec = 0)
{
  bool sensorFound = false;

  for (int index = 0; index < MAX_TELEMETRY_SENSORS; index++) {
    TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];

    if (telemetrySensor.type == TELEM_TYPE_CUSTOM && telemetrySensor.id == id &&
        telemetrySensor.subId == subId &&
        (telemetrySensor.isSameInstance(protocol, instance) ||
         g_model.ignoreSensorIds)) {

      telemetryItems[index].setValue(telemetrySensor, value, unit, prec);
      sensorFound = true;
      // we continue search here, because sensors can share the same id and
      // instance
    }
  }

  if (sensorFound || !allowNewSensors) {
    return -1;
  }

  int index = availableTelemetryIndex();
  if (index >= 0) {
    switch (protocol) {
      case PROTOCOL_TELEMETRY_FRSKY_SPORT:
        frskySportSetDefault(index, id, subId, instance);
        break;

      case PROTOCOL_TELEMETRY_FRSKY_D:
        frskyDSetDefault(index, id);
        break;

#if defined(CROSSFIRE)
      case PROTOCOL_TELEMETRY_CROSSFIRE:
        crossfireSetDefault(index, id, instance);
        break;
#endif

#if defined(GHOST)
      case PROTOCOL_TELEMETRY_GHOST:
        ghostSetDefault(index, id, instance);
        break;
#endif

#if defined(MULTIMODULE) || defined(AFHDS3) || defined(AFHDS2)
      case PROTOCOL_TELEMETRY_FLYSKY_IBUS:
        flySkySetDefault(index, id, subId, instance);
        break;
#endif

#if defined(AFHDS2) && defined(PCBNV14)
      case PROTOCOL_TELEMETRY_FLYSKY_NV14:
        flySkyNv14SetDefault(index, id, subId, instance);
        break;
#endif

      case PROTOCOL_TELEMETRY_SPEKTRUM:
        spektrumSetDefault(index, id, subId, instance);
        break;

#if defined(MULTIMODULE)
      case PROTOCOL_TELEMETRY_HITEC:
        hitecSetDefault(index, id, subId, instance);
        break;

      case PROTOCOL_TELEMETRY_HOTT:
        hottSetDefault(index, id, subId, instance);
        break;

      case PROTOCOL_TELEMETRY_MLINK:
        mlinkSetDefault(index, id, subId, instance);
        break;
#endif

#if defined(LUA)
     case PROTOCOL_TELEMETRY_LUA:
        // Sensor will be initialized by calling function
        // This drops the first value
        return index;
#endif

      default:
        return index;
    }
    telemetryItems[index].setValue(g_model.telemetrySensors[index], value, unit, prec);
    return index;
  }
  else {
    POPUP_WARNING(STR_TELEMETRYFULL);
    return -1;
  }
}

int setTelemetryValue(TelemetryProtocol protocol, uint16_t id, uint8_t subId, uint8_t instance, int32_t value, uint32_t unit, uint32_t prec)
{
  return setTelemetryValue<int32_t>(protocol, id, subId, instance, value, unit, prec);
}

int setTelemetryText(TelemetryProtocol protocol, uint16_t id, uint8_t subId, uint8_t instance, const char * text)
{
  return setTelemetryValue<const char *>(protocol, id, subId, instance, text);
}

void TelemetrySensor::init(const char * label, uint8_t unit, uint8_t prec)
{
  memclear(this->label, TELEM_LABEL_LEN);
  strncpy(this->label, label, TELEM_LABEL_LEN);
  this->unit = unit;
  if (prec > 1 && (IS_DISTANCE_UNIT(unit) || IS_SPEED_UNIT(unit))) {
    // 2 digits precision is not needed here
    prec = 1;
  }
  this->prec = prec;
  // Log sensors by default
  this->logs = true;
}

void TelemetrySensor::init(uint16_t id)
{
  char label[TELEM_LABEL_LEN]; // 4
  label[0] = hex2char((id & 0xf000) >> 12);
  label[1] = hex2char((id & 0x0f00) >> 8);
  label[2] = hex2char((id & 0x00f0) >> 4);
  label[3] = hex2char((id & 0x000f) >> 0);
  init(label);
}

bool TelemetrySensor::isAvailable() const
{
  return ZLEN(label) > 0;
}

PACK(typedef struct {
  uint8_t unitFrom;
  uint8_t unitTo;
  int16_t multiplier;
  int16_t divisor;
}) UnitConversionRule;

const UnitConversionRule unitConversionTable[] = {
  /* unitFrom     unitTo                    multiplier   divisor */
  { UNIT_METERS,            UNIT_FEET,             105,    32},
  { UNIT_FEET,              UNIT_METERS,            32,   105},
  { UNIT_METERS_PER_SECOND, UNIT_FEET_PER_SECOND,  105,    32},

  { UNIT_KTS, UNIT_KMH,                           1852,  1000}, // 1 knot = 1.85200 kilometers per hour
  { UNIT_KTS, UNIT_MPH,                           1151,  1000}, // 1 knot = 1.15077945 miles per hour
  { UNIT_KTS, UNIT_METERS_PER_SECOND,             1000,  1944}, // 1 knot = 0.514444444 meters / second (divide with 1.94384449)
  { UNIT_KTS, UNIT_FEET_PER_SECOND,               1688,  1000}, // 1 knot = 1.68780986 feet per second

  { UNIT_KMH, UNIT_KTS,                           1000,  1852}, // 1 km/h = 0.539956803 knots (divide with 1.85200)
  { UNIT_KMH, UNIT_MPH,                           1000,  1609}, // 1 km/h = 0.621371192 miles per hour (divide with 1.60934400)
  { UNIT_KMH, UNIT_METERS_PER_SECOND,               10,    36}, // 1 km/h = 0.277777778 meters / second (divide with 3.6)
  { UNIT_KMH, UNIT_FEET_PER_SECOND,                911,  1000}, // 1 km/h = 0.911344415 feet per second

  { UNIT_MILLILITERS, UNIT_FLOZ,                   100,  2957},

  { UNIT_RADIANS, UNIT_DEGREE,                   10000,   175}, // 1 rad = 57.29578 deg
  { UNIT_DEGREE, UNIT_RADIANS,                     175, 10000}, // 1 deg = ‪0,0174533‬ rad

  { 0, 0, 0, 0}   // termination
};

int32_t convertTelemetryValue(int32_t value, uint8_t unit, uint8_t prec, uint8_t destUnit, uint8_t destPrec)
{
  for (int i=prec; i<destPrec; i++)
    value *= 10;

  if (unit == UNIT_CELSIUS) {
    if (destUnit == UNIT_FAHRENHEIT) {
      // T(°F) = T(°C)×1,8 + 32
      value = 32 + (value*18) / 10;
    }
  } else if (unit == UNIT_FAHRENHEIT) {
    if (destUnit == UNIT_CELSIUS) {
      value = (value - 32) * 10/18;
    }
  }
  else {
    const UnitConversionRule * p = unitConversionTable;
    while (p->divisor) {
      if (p->unitFrom == unit && p->unitTo == destUnit) {
        value = (value * (int32_t)p->multiplier) / (int32_t)p->divisor;
        break;
      }
      ++p;
    }
  }

  for (int i=destPrec; i<prec; i++)
    value /= 10;

  return value;
}

int32_t TelemetrySensor::getValue(int32_t value, uint8_t unit, uint8_t prec) const
{
  if (type == TELEM_TYPE_CUSTOM && custom.ratio) {
    if (this->prec == 2) {
      value *= 10;
      prec = 2;
    }
    else {
      prec = 1;
    }
    value = (custom.ratio * value + 122) / 255;
  }

  value = convertTelemetryValue(value, unit, prec, this->unit, this->prec);

  if (type == TELEM_TYPE_CUSTOM) {
    value += custom.offset;
    if (value < 0 && onlyPositive) {
      value = 0;
    }
  }

  return value;
}

bool TelemetrySensor::isConfigurable() const
{
  if (type == TELEM_TYPE_CALCULATED) {
    if (formula >= TELEM_FORMULA_CELL) {
      return false;
    }
  }
  else {
    if (unit >= UNIT_FIRST_VIRTUAL)  {
      return false;
    }
  }
  return true;
}

bool TelemetrySensor::isPrecConfigurable() const
{
  if (isConfigurable()) {
    return true;
  }
  else if (unit == UNIT_CELLS) {
    return true;
  }
  else {
    return false;
  }
}

int32_t TelemetrySensor::getPrecMultiplier() const
{
  /*
    Important: the return type must be signed, otherwise
    mathematic operations with a negative telemetry value won't work
  */
  if (prec == 2) return 1;
  if (prec == 1) return 10;
  return 100;
}

int32_t TelemetrySensor::getPrecDivisor() const
{
  if (prec == 2) return 100;
  if (prec == 1) return 10;
  return 1;
}

bool TelemetrySensor::isSameInstance(TelemetryProtocol protocol, uint8_t instance)
{
  if (this->instance == instance)
    return true;

  if (protocol == PROTOCOL_TELEMETRY_FRSKY_SPORT) {
#if defined(SIMU)
    if (((this->instance ^ instance) & 0x1F) == 0)
      return true;
#else
    if (((this->instance ^ instance) & 0x9F) == 0 && (this->instance >> 5) != TELEMETRY_ENDPOINT_SPORT && (instance >> 5) != TELEMETRY_ENDPOINT_SPORT) {
      this->instance = instance; // update the instance in case we had telemetry switching
      return true;
    }
#endif
  }

  return false;
}
