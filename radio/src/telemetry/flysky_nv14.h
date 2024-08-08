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

enum FlySkySensorType_E {
  FLYSKY_SENSOR_RX_VOLTAGE,
  FLYSKY_SENSOR_RX_SIGNAL,
  FLYSKY_SENSOR_RX_RSSI,
  FLYSKY_SENSOR_RX_NOISE,
  FLYSKY_SENSOR_RX_SNR,
  FLYSKY_SENSOR_TEMP,
  FLYSKY_SENSOR_EXT_VOLTAGE,
  FLYSKY_SENSOR_MOTO_RPM,
  FLYSKY_SENSOR_PRESSURE,
  FLYSKY_SENSOR_GPS,
  FLYSKY_SENSOR_SYNC = 0xEE,
};


typedef struct FLYSKY_GPS_INFO_S {
  uint8_t position_fix;
  uint8_t satell_cnt;
  uint8_t latitude[4];
  uint8_t longtitude[4];
  uint8_t altitude[4];
  uint8_t g_speed[2];
  uint8_t direction[2];
} gps_info_t;
typedef struct FLYSKY_SENSOR_DATA_S {
  uint8_t sensor_type;
  uint8_t sensor_id;
  uint8_t voltage[2];
  uint8_t signal;
  uint8_t rssi[2];
  uint8_t noise[2];
  uint8_t snr[2];
  uint8_t temp[2];
  uint8_t ext_voltage[2];
  uint8_t moto_rpm[2];
  uint8_t pressure_value[2];
  gps_info_t gps_info;
} rx_sensor_t;

void flySkyNv14SetDefault(int index, uint8_t id, uint8_t subId, uint8_t instance);
void flySkyNv14ProcessTelemetryPacket(const uint8_t * ptr, uint8_t SensorType );
void processInternalFlySkyTelemetryData(uint8_t byte);
uint8_t intmoduleGetByte(uint8_t * byte);

extern bool syncAfhds2Module;
