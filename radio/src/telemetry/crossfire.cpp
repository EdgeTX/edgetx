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

#include "crossfire.h"
#include "edgetx.h"
#include "math.h"

#include "trainer.h"

// clang-format off
#define CS(id,subId,name,unit,precision) {id,subId,unit,precision,name}

#define CROSSFIRE_CH_BITS           11
#define CROSSFIRE_CH_MASK           ((1 << CROSSFIRE_CH_BITS) - 1)
#define CROSSFIRE_CH_CENTER         0x3E0

const CrossfireSensor crossfireSensors[] = {
  CS(LINK_ID,        0, STR_SENSOR_RX_RSSI1,      UNIT_DB,                0),
  CS(LINK_ID,        1, STR_SENSOR_RX_RSSI2,      UNIT_DB,                0),
  CS(LINK_ID,        2, STR_SENSOR_RX_QUALITY,    UNIT_PERCENT,           0),
  CS(LINK_ID,        3, STR_SENSOR_RX_SNR,        UNIT_DB,                0),
  CS(LINK_ID,        4, STR_SENSOR_ANTENNA,       UNIT_RAW,               0),
  CS(LINK_ID,        5, STR_SENSOR_RF_MODE,       UNIT_RAW,               0),
  CS(LINK_ID,        6, STR_SENSOR_TX_POWER,      UNIT_MILLIWATTS,        0),
  CS(LINK_ID,        7, STR_SENSOR_TX_RSSI,       UNIT_DB,                0),
  CS(LINK_ID,        8, STR_SENSOR_TX_QUALITY,    UNIT_PERCENT,           0),
  CS(LINK_ID,        9, STR_SENSOR_TX_SNR,        UNIT_DB,                0),
  CS(LINK_RX_ID,     0, STR_SENSOR_RX_RSSI_PERC,  UNIT_PERCENT,           0),
  CS(LINK_RX_ID,     1, STR_SENSOR_RX_RF_POWER,   UNIT_DBM,               0),
  CS(LINK_TX_ID,     0, STR_SENSOR_TX_RSSI_PERC,  UNIT_PERCENT,           0),
  CS(LINK_TX_ID,     1, STR_SENSOR_TX_POWER,      UNIT_DBM,               0),
  CS(LINK_TX_ID,     2, STR_SENSOR_TX_FPS,        UNIT_HERTZ,             0),
  CS(BATTERY_ID,     0, STR_SENSOR_BATT,          UNIT_VOLTS,             1),
  CS(BATTERY_ID,     1, STR_SENSOR_CURR,          UNIT_AMPS,              1),
  CS(BATTERY_ID,     2, STR_SENSOR_CAPACITY,      UNIT_MAH,               0),
  CS(BATTERY_ID,     3, STR_SENSOR_BATT_PERCENT,  UNIT_PERCENT,           0),
  CS(GPS_ID,         0, STR_SENSOR_GPS,           UNIT_GPS_LATITUDE,      0),
  CS(GPS_ID,         0, STR_SENSOR_GPS,           UNIT_GPS_LONGITUDE,     0),
  CS(GPS_ID,         2, STR_SENSOR_GSPD,          UNIT_KMH,               1),
  CS(GPS_ID,         3, STR_SENSOR_HDG,           UNIT_DEGREE,            2),
  CS(GPS_ID,         4, STR_SENSOR_ALT,           UNIT_METERS,            0),
  CS(GPS_ID,         5, STR_SENSOR_SATELLITES,    UNIT_RAW,               0),
  CS(GPS_ID,         6, STR_SENSOR_LONRAW,        UNIT_RAW,               0),
  CS(GPS_ID,         7, STR_SENSOR_LATRAW,        UNIT_RAW,               0),
  CS(ATTITUDE_ID,    0, STR_SENSOR_PITCH,         UNIT_RADIANS,           3),
  CS(ATTITUDE_ID,    1, STR_SENSOR_ROLL,          UNIT_RADIANS,           3),
  CS(ATTITUDE_ID,    2, STR_SENSOR_YAW,           UNIT_RADIANS,           3),
  CS(FLIGHT_MODE_ID, 0, STR_SENSOR_FLIGHT_MODE,   UNIT_TEXT,              0),
  CS(CF_VARIO_ID,    0, STR_SENSOR_VSPD,          UNIT_METERS_PER_SECOND, 2),
  CS(BARO_ALT_ID,    0, STR_SENSOR_ALT,           UNIT_METERS,            2),
  CS(AIRSPEED_ID,    0, STR_SENSOR_ASPD,          UNIT_KMH,               1),
  CS(CF_RPM_ID,      0, STR_SENSOR_RPM,           UNIT_RPMS,              0),
  CS(TEMP_ID,        0, STR_SENSOR_TEMP,          UNIT_DEGREE,            1),
  CS(CELLS_ID,       0, STR_SENSOR_CELLS,         UNIT_CELLS,             2),
  CS(VOLT_ARRAY_ID,  0, STR_SENSOR_VOLT,          UNIT_VOLTS,             2),
  CS(0,              0, "UNKNOWN",                UNIT_RAW,               0),
};
// clang-format on

CrossfireModuleStatus crossfireModuleStatus[2] = {0};

const CrossfireSensor & getCrossfireSensor(uint8_t id, uint8_t subId)
{
  if (id == LINK_ID)
    return crossfireSensors[RX_RSSI1_INDEX + subId];
  else if (id == LINK_RX_ID)
    return crossfireSensors[RX_RSSI_PERC_INDEX + subId];
  else if (id == LINK_TX_ID)
    return crossfireSensors[TX_RSSI_PERC_INDEX + subId];
  else if (id == BATTERY_ID)
    return crossfireSensors[BATT_VOLTAGE_INDEX + subId];
  else if (id == GPS_ID)
    return crossfireSensors[GPS_LATITUDE_INDEX + subId];
  else if (id == CF_VARIO_ID)
    return crossfireSensors[VERTICAL_SPEED_INDEX];
  else if (id == ATTITUDE_ID)
    return crossfireSensors[ATTITUDE_PITCH_INDEX + subId];
  else if (id == FLIGHT_MODE_ID)
    return crossfireSensors[FLIGHT_MODE_INDEX];
  else if (id == BARO_ALT_ID)
    return crossfireSensors[BARO_ALTITUDE_INDEX];
  else if (id == AIRSPEED_ID)
    return crossfireSensors[AIRSPEED_INDEX];
  else if (id == CF_RPM_ID)
    return crossfireSensors[CF_RPM_INDEX];
  else if (id == TEMP_ID)
    return crossfireSensors[TEMP_INDEX];
  else if (id == CELLS_ID)
    return crossfireSensors[CELLS_INDEX];
  else if (id == VOLT_ARRAY_ID)
    return crossfireSensors[VOLT_ARRAY_INDEX];
  else
    return crossfireSensors[UNKNOWN_INDEX];
}

void processCrossfireTelemetryValue(uint8_t index, int32_t value)
{
  if (!TELEMETRY_STREAMING())
    return;

  const CrossfireSensor & sensor = crossfireSensors[index];
  setTelemetryValue(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id, 0, sensor.subId,
                    value, sensor.unit, sensor.precision);
}

template <int N>
bool getCrossfireTelemetryValue(uint8_t index, int32_t& value,
                                uint8_t* rxBuffer)
{
  bool result = false;
  uint8_t * byte = &rxBuffer[index];
  value = (*byte & 0x80) ? -1 : 0;
  for (uint8_t i=0; i<N; i++) {
    value <<= 8;
    if (*byte != 0xff) {
      result = true;
    }
    value += *byte++;
  }
  return result;
}

void processCrossfireTelemetryFrame(uint8_t module, uint8_t* rxBuffer,
                                    uint8_t rxBufferCount)
{
  if (telemetryState == TELEMETRY_INIT &&
      moduleState[module].counter != CRSF_FRAME_MODELID_SENT) {
    moduleState[module].counter = CRSF_FRAME_MODELID;
  }

  uint8_t crsfPayloadLen = rxBuffer[1];
  uint8_t id = rxBuffer[2];
  int32_t value;
  switch(id) {
    case CF_VARIO_ID:
      if (getCrossfireTelemetryValue<2>(3, value, rxBuffer))
        processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case GPS_ID:
      if (getCrossfireTelemetryValue<4>(3, value, rxBuffer)) {
        processCrossfireTelemetryValue(GPS_LATITUDE_INDEX, value/10);
        processCrossfireTelemetryValue(GPS_LATRAW_INDEX, value);
      }
      if (getCrossfireTelemetryValue<4>(7, value, rxBuffer)) {
        processCrossfireTelemetryValue(GPS_LONGITUDE_INDEX, value/10);
        processCrossfireTelemetryValue(GPS_LONRAW_INDEX, value);
      }
      if (getCrossfireTelemetryValue<2>(11, value, rxBuffer))
        processCrossfireTelemetryValue(GPS_GROUND_SPEED_INDEX, value);
      if (getCrossfireTelemetryValue<2>(13, value, rxBuffer))
        processCrossfireTelemetryValue(GPS_HEADING_INDEX, value);
      if (getCrossfireTelemetryValue<2>(15, value, rxBuffer))
        processCrossfireTelemetryValue(GPS_ALTITUDE_INDEX,  value - 1000);
      if (getCrossfireTelemetryValue<1>(17, value, rxBuffer))
        processCrossfireTelemetryValue(GPS_SATELLITES_INDEX, value);
      break;

    case BARO_ALT_ID:
      if (getCrossfireTelemetryValue<2>(3, value, rxBuffer)) {
        if (value & 0x8000) {
          // Altitude in meters
          value &= ~(0x8000);
          value *= 100; // cm
        } else {
          // Altitude in decimeters + 10000dm
          value -= 10000;
          value *= 10;
        }
        processCrossfireTelemetryValue(BARO_ALTITUDE_INDEX, value);
      }

      // Length of TBS BARO_ALT has 4 payload bytes with just 2 bytes of altitude
      // but support including TBS VARIO if the declared payload length is 5 bytes
      if (crsfPayloadLen == 5 &&
          getCrossfireTelemetryValue<1>(5, value, rxBuffer)) {
       constexpr int Kl = 100;       // linearity constant;
       constexpr float Kr = .026;    // range constant;

       int8_t sign = value < 0 ? -1 : 1;
       value =((expf(value * sign * Kr) - 1) * Kl) * sign;
       processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      }

      // Length of TBS BARO_ALT has 4 payload bytes with just 2 bytes of altitude
      // but support including ELRS VARIO if the declared payload length is 6 bytes or more
      if (crsfPayloadLen > 5 &&
          getCrossfireTelemetryValue<2>(5, value, rxBuffer))
        processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case AIRSPEED_ID:
      if (getCrossfireTelemetryValue<2>(3, value, rxBuffer)) {
        // Airspeed in 0.1 * km/h (hectometers/h)
        // Converstion to KMH is done through PREC1
        processCrossfireTelemetryValue(AIRSPEED_INDEX, value);
      }
      break;

    case CF_RPM_ID:
    {
      getCrossfireTelemetryValue<1>(3, value, rxBuffer);
      uint8_t sensorID = value;
      for(uint8_t i = 0; i * 3 < (crsfPayloadLen - 4);  i++) {
        getCrossfireTelemetryValue<3>(4 + i * 3, value, rxBuffer);
        const CrossfireSensor & sensor = crossfireSensors[CF_RPM_INDEX];
        setTelemetryValue(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id + (sensorID << 8), 0, i,
                          value, sensor.unit, sensor.precision);
      }
      break;
    }

    case TEMP_ID:
    {
      getCrossfireTelemetryValue<1>(3, value, rxBuffer);
      uint8_t sensorID = value;
      for(uint8_t i = 0; i * 2 < (crsfPayloadLen - 4);  i++) {
        getCrossfireTelemetryValue<2>(4 + i * 2, value, rxBuffer);
        const CrossfireSensor & sensor = crossfireSensors[TEMP_INDEX];
        setTelemetryValue(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id + (sensorID << 8), 0, i,
                          value, sensor.unit, sensor.precision);
      }
      break;
    }

    case CELLS_ID:
    {
      getCrossfireTelemetryValue<1>(3, value, rxBuffer);
      uint8_t sensorID = value;

      if (sensorID < 128) {
        // Treating frame as Cells sensor
        // We can handle only up to 8 cells
        for(uint8_t i = 0; i * 2 < min(16, crsfPayloadLen - 4);  i++) {
          getCrossfireTelemetryValue<2>(4 + i * 2, value, rxBuffer);
          const CrossfireSensor & sensor = crossfireSensors[CELLS_INDEX];
          setTelemetryValue(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id + (sensorID << 8), 0, 0,
                          i << 16 | value / 10, sensor.unit, sensor.precision);
        }
      } else {
        // Treating frame as Voltage sensor array
        for(uint8_t i = 0; i * 2 < (crsfPayloadLen - 4);  i++) {
          value = (rxBuffer[4 + i * 2] << 8) + rxBuffer[4 + i * 2 + 1];
          const CrossfireSensor & sensor = crossfireSensors[VOLT_ARRAY_INDEX];
          setTelemetryValue(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id + (sensorID << 8), 0, i,
                                    value / 10, sensor.unit, sensor.precision);
        }
      }
      break;
    }

    case LINK_ID:
      for (unsigned int i=0; i<=TX_SNR_INDEX; i++) {
        if (getCrossfireTelemetryValue<1>(3+i, value, rxBuffer)) {
          if (i == TX_POWER_INDEX) {
            static const int32_t power_values[] = {0,    10,   25,  100, 500,
                                                   1000, 2000, 250, 50};
            value =
                ((unsigned)value < DIM(power_values) ? power_values[value] : 0);
          }
          processCrossfireTelemetryValue(i, value);
          if (i == RX_QUALITY_INDEX) {
            if (value) {
              telemetryData.rssi.set(value);
              telemetryStreaming = TELEMETRY_TIMEOUT10ms;
              telemetryData.telemetryValid |= 1 << module;
            }
            else {
              if (telemetryData.telemetryValid & (1 << module)) {
                telemetryData.rssi.reset();
                telemetryStreaming = 0;
              }
              telemetryData.telemetryValid &= ~(1 << module);
            }
          }
        }
      }
      break;

    case CHANNELS_ID:
      if (g_model.trainerData.mode == TRAINER_MODE_CRSF) {
        uint8_t inputbitsavailable = 0;
        uint32_t inputbits = 0;
        uint8_t  byteIdx = 3;
        int16_t *pulses = trainerInput;

        for (int i = 0; i < min(CROSSFIRE_CHANNELS_COUNT, MAX_TRAINER_CHANNELS); i++) {
          while (inputbitsavailable < CROSSFIRE_CH_BITS) {
            inputbits |= (uint32_t)(rxBuffer[byteIdx++]) << inputbitsavailable;
            inputbitsavailable += 8;
          }
          *pulses++ = ((int32_t)(inputbits & CROSSFIRE_CH_MASK) - CROSSFIRE_CH_CENTER) * 5 / 8;
          inputbitsavailable -= CROSSFIRE_CH_BITS;
          inputbits >>= CROSSFIRE_CH_BITS;
        }

        trainerResetTimer();
      }
      break;

    case LINK_RX_ID:
      if (getCrossfireTelemetryValue<1>(4, value, rxBuffer))
        processCrossfireTelemetryValue(RX_RSSI_PERC_INDEX, value);
      if (getCrossfireTelemetryValue<1>(7, value, rxBuffer))
        processCrossfireTelemetryValue(TX_RF_POWER_INDEX, value);
      break;

    case LINK_TX_ID:
      if (getCrossfireTelemetryValue<1>(4, value, rxBuffer))
        processCrossfireTelemetryValue(TX_RSSI_PERC_INDEX, value);
      if (getCrossfireTelemetryValue<1>(7, value, rxBuffer))
        processCrossfireTelemetryValue(RX_RF_POWER_INDEX, value);
      if (getCrossfireTelemetryValue<1>(8, value, rxBuffer))
        processCrossfireTelemetryValue(TX_FPS_INDEX, value * 10);
      break;

    case BATTERY_ID:
      if (getCrossfireTelemetryValue<2>(3, value, rxBuffer))
        processCrossfireTelemetryValue(BATT_VOLTAGE_INDEX, value);
      if (getCrossfireTelemetryValue<2>(5, value, rxBuffer))
        processCrossfireTelemetryValue(BATT_CURRENT_INDEX, value);
      if (getCrossfireTelemetryValue<3>(7, value, rxBuffer))
        processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value);
      if (getCrossfireTelemetryValue<1>(10, value, rxBuffer))
        processCrossfireTelemetryValue(BATT_REMAINING_INDEX, value);
      break;

    case ATTITUDE_ID:
      if (getCrossfireTelemetryValue<2>(3, value, rxBuffer))
        processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(5, value, rxBuffer))
        processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(7, value, rxBuffer))
        processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value/10);
      break;

    case FLIGHT_MODE_ID:
    {
      const CrossfireSensor & sensor = crossfireSensors[FLIGHT_MODE_INDEX];
      auto textLength = min<int>(16, rxBuffer[1]);
      rxBuffer[textLength] = '\0';
      setTelemetryText(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id, 0, sensor.subId,
                       (const char *)rxBuffer + 3);
      break;
    }

    case RADIO_ID:
      if (rxBuffer[3] == 0xEA     // radio address
          && rxBuffer[5] == 0x10  // timing correction frame
      ) {
        uint32_t update_interval;
        int32_t offset;
        if (getCrossfireTelemetryValue<4>(6, (int32_t &)update_interval,
                                          rxBuffer) &&
            getCrossfireTelemetryValue<4>(10, offset, rxBuffer)) {
          // values are in 10th of micro-seconds
          update_interval /= 10;
          offset /= 10;

          //TRACE("[XF] Rate: %d, Lag: %d", update_interval, offset);
          getModuleSyncStatus(module).update(update_interval, offset);
        }
      }
      break;

#if defined(LUA)
    default:
      if (id == DEVICE_INFO_ID && rxBuffer[4]== MODULE_ADDRESS) {
        uint8_t nameSize = rxBuffer[1] - 18;
        strncpy((char *)&crossfireModuleStatus[module].name, (const char *)&rxBuffer[5], CRSF_NAME_MAXSIZE);
        crossfireModuleStatus[module].name[CRSF_NAME_MAXSIZE -1] = 0; // For some reason, GH din't like strlcpy
        if (strncmp((const char *) &rxBuffer[5 + nameSize], "ELRS", 4) == 0)
          crossfireModuleStatus[module].isELRS = true;
        crossfireModuleStatus[module].major = rxBuffer[14 + nameSize];
        crossfireModuleStatus[module].minor = rxBuffer[15 + nameSize];
        crossfireModuleStatus[module].revision = rxBuffer[16 + nameSize];

        ModuleData *md = &g_model.moduleData[module];

        if(!CRSF_ELRS_MIN_VER(module, 4, 0) &&
           (md->crsf.crsfArmingMode != ARMING_MODE_CH5 || md->crsf.crsfArmingMode != SWSRC_NONE)) {
          md->crsf.crsfArmingMode = ARMING_MODE_CH5;
          md->crsf.crsfArmingTrigger = SWSRC_NONE;

          storageDirty(EE_MODEL);
        }

        crossfireModuleStatus[module].queryCompleted = true;
      }

      // destination address and CRC are skipped
      pushTelemetryDataToQueues(rxBuffer + 1, rxBufferCount - 2);
      break;
#endif
  }
}

void crossfireSetDefault(int index, uint16_t id, uint8_t subId)
{
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.instance = subId;

  const CrossfireSensor & sensor = getCrossfireSensor(id, subId);
  TelemetryUnit unit = sensor.unit;
  if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE)
    unit = UNIT_GPS;
  uint8_t prec = min<uint8_t>(2, sensor.precision);
  telemetrySensor.init(sensor.name, unit, prec);
  if (id == LINK_ID) {
    telemetrySensor.logs = true;
  }

  storageDirty(EE_MODEL);
}
