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

#include "mavlink.h"
#include "edgetx.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "common/mavlink.h"
#pragma GCC diagnostic pop

#define MAVLINK_DEFAULT_RSSI 100
#define MS(id,name,unit,precision) {id,unit,precision,name}

struct MavlinkSensor {
  const uint8_t id;
  const TelemetryUnit unit;
  const uint8_t precision;
  STR_TYP name;
};

static const MavlinkSensor mavlinkSensors[] = {
  MS(MAV_ID_RSSI,        STR_DEF(STR_SENSOR_RX_RSSI_PERC), UNIT_PERCENT,            0),
  MS(MAV_ID_BATT_V,      STR_DEF(STR_SENSOR_BATT),         UNIT_VOLTS,              2),
  MS(MAV_ID_BATT_A,      STR_DEF(STR_SENSOR_CURR),         UNIT_AMPS,               2),
  MS(MAV_ID_BATT_PCT,    STR_DEF(STR_SENSOR_BATT_PERCENT), UNIT_PERCENT,            0),
  MS(MAV_ID_GPS,         STR_DEF(STR_SENSOR_GPS),          UNIT_GPS_LATITUDE,       0),
  MS(MAV_ID_GPS_ALT,     STR_DEF(STR_SENSOR_GPSALT),       UNIT_METERS,             0),
  MS(MAV_ID_GPS_SPD,     STR_DEF(STR_SENSOR_GSPD),         UNIT_KMH,                1),
  MS(MAV_ID_GPS_SATS,    STR_DEF(STR_SENSOR_SATELLITES),   UNIT_RAW,                0),
  MS(MAV_ID_GPS_HDG,     STR_DEF(STR_SENSOR_HDG),          UNIT_DEGREE,             2),
  MS(MAV_ID_ROLL,        STR_DEF(STR_SENSOR_ROLL),         UNIT_DEGREE,             1),
  MS(MAV_ID_PITCH,       STR_DEF(STR_SENSOR_PITCH),        UNIT_DEGREE,             1),
  MS(MAV_ID_YAW,         STR_DEF(STR_SENSOR_YAW),          UNIT_DEGREE,             1),
  MS(MAV_ID_ALT,         STR_DEF(STR_SENSOR_ALT),          UNIT_METERS,             1),
  MS(MAV_ID_VSPD,        STR_DEF(STR_SENSOR_VSPD),         UNIT_METERS_PER_SECOND,  2),
  MS(MAV_ID_ASPD,        STR_DEF(STR_SENSOR_ASPD),         UNIT_KMH,                1),
  MS(MAV_ID_FLIGHT_MODE, STR_DEF(STR_SENSOR_FLIGHT_MODE),  UNIT_TEXT,               0),
  MS(MAV_ID_RRSSI,       STR_DEF(STR_SENSOR_TX_RSSI_PERC), UNIT_PERCENT,            0),
  MS(MAV_ID_NONE,        STR_DEF(STR_UNKNOWN),             UNIT_RAW,                0),
};

static const MavlinkSensor& getMavlinkSensor(uint8_t id)
{
  for (auto& s : mavlinkSensors) {
    if (s.id == id) return s;
  }
  return mavlinkSensors[DIM(mavlinkSensors) - 1];
}

static inline void pushVal(uint8_t id, int32_t value, TelemetryUnit unit, uint8_t prec)
{
  setTelemetryValue(PROTOCOL_TELEMETRY_MAVLINK, id, 0, 0, value, unit, prec);
}

void mavlinkSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  (void)subId;

  TelemetrySensor& telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.instance = instance;

  const MavlinkSensor& sensor = getMavlinkSensor((uint8_t)id);
  TelemetryUnit unit = sensor.unit;
  if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE) unit = UNIT_GPS;
  telemetrySensor.init(STR_VAL(sensor.name), unit, sensor.precision);

  storageDirty(EE_MODEL);
}

void mavlinkTelemetryDecode(uint8_t module, const mavlink_message_t* msg)
{
  (void)module;

  // in case there won't be RADIO_STATUS received
  if (telemetryStreaming == 0) {
    telemetryData.rssi.reset();
    telemetryData.rssi.set(MAVLINK_DEFAULT_RSSI);
  }

  telemetryStreaming = TELEMETRY_TIMEOUT10ms;

  switch (msg->msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: {
      if (msg->compid == MAV_COMP_ID_TELEMETRY_RADIO)
        break;
      mavlink_heartbeat_t hb;
      mavlink_msg_heartbeat_decode(msg, &hb);
      bool armed = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
      setTelemetryText(PROTOCOL_TELEMETRY_MAVLINK, MAV_ID_FLIGHT_MODE, 0, 0, armed ? "Armed" : "Disarm");
    } break;

    case MAVLINK_MSG_ID_SYS_STATUS: {
      mavlink_sys_status_t s;
      mavlink_msg_sys_status_decode(msg, &s);
      if (s.voltage_battery != UINT16_MAX)
        pushVal(MAV_ID_BATT_V, s.voltage_battery / 10, UNIT_VOLTS, 2);
      if (s.current_battery != -1)
        pushVal(MAV_ID_BATT_A, s.current_battery, UNIT_AMPS, 2);
      if (s.battery_remaining != -1)
        pushVal(MAV_ID_BATT_PCT, s.battery_remaining, UNIT_PERCENT, 0);
    } break;

    case MAVLINK_MSG_ID_GPS_RAW_INT: {
      mavlink_gps_raw_int_t g;
      mavlink_msg_gps_raw_int_decode(msg, &g);
      pushVal(MAV_ID_GPS, g.lat / 10, UNIT_GPS_LATITUDE, 0);
      pushVal(MAV_ID_GPS, g.lon / 10, UNIT_GPS_LONGITUDE, 0);
      pushVal(MAV_ID_GPS_ALT, g.alt / 1000, UNIT_METERS, 0);
      if (g.vel != UINT16_MAX)
        pushVal(MAV_ID_GPS_SPD, ((int32_t)g.vel * 36) / 100, UNIT_KMH, 1);
      if (g.cog != UINT16_MAX)
        pushVal(MAV_ID_GPS_HDG, g.cog, UNIT_DEGREE, 2);
      if (g.satellites_visible != UINT8_MAX)
        pushVal(MAV_ID_GPS_SATS, g.satellites_visible, UNIT_RAW, 0);
    } break;

    case MAVLINK_MSG_ID_ATTITUDE: {
      mavlink_attitude_t a;
      mavlink_msg_attitude_decode(msg, &a);
      const float toDeg10 = 1800.0f / 3.14159265f; // rad -> 0.1 deg
      pushVal(MAV_ID_ROLL, (int32_t)(a.roll * toDeg10), UNIT_DEGREE, 1);
      pushVal(MAV_ID_PITCH, (int32_t)(a.pitch * toDeg10), UNIT_DEGREE, 1);
      pushVal(MAV_ID_YAW, (int32_t)(a.yaw * toDeg10), UNIT_DEGREE, 1);
    } break;

    case MAVLINK_MSG_ID_VFR_HUD: {
      mavlink_vfr_hud_t v;
      mavlink_msg_vfr_hud_decode(msg, &v);
      pushVal(MAV_ID_ASPD, (int32_t)(v.airspeed * 36), UNIT_KMH, 1); // m/s -> 0.1 km/h
      pushVal(MAV_ID_ALT, (int32_t)(v.alt * 10), UNIT_METERS, 1);
      pushVal(MAV_ID_VSPD, (int32_t)(v.climb * 100), UNIT_METERS_PER_SECOND, 2);
    } break;

    case MAVLINK_MSG_ID_RADIO_STATUS: {
      mavlink_radio_status_t rs;
      mavlink_msg_radio_status_decode(msg, &rs);
      if (rs.rssi != UINT8_MAX) {
        int32_t rssiPercent = ((int32_t)rs.rssi * 100) / 254; // 0-254 -> 0-100%
        pushVal(MAV_ID_RSSI, rssiPercent, UNIT_PERCENT, 0);
        telemetryData.rssi.set((uint8_t)rssiPercent);
      }
      if (rs.remrssi != UINT8_MAX) {
        pushVal(MAV_ID_RRSSI, ((int32_t)rs.remrssi * 100) / 254, UNIT_PERCENT, 0);
      }
    } break;

    default:
      break;
  }
}
