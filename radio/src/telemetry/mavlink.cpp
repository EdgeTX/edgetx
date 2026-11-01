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
#include "sensor_names.h"

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
  const char* name;
};

static const MavlinkSensor mavlinkSensors[] = {
  MS(MAV_ID_TQLY,        STR_SENSOR_TX_QUALITY,            UNIT_PERCENT,            0),
  MS(MAV_ID_BATT_V,      STR_SENSOR_BATT,                  UNIT_VOLTS,              2),
  MS(MAV_ID_BATT_A,      STR_SENSOR_CURR,                  UNIT_AMPS,               2),
  MS(MAV_ID_BATT_PCT,    STR_SENSOR_BATT_PERCENT,          UNIT_PERCENT,            0),
  MS(MAV_ID_GPS,         STR_SENSOR_GPS,                   UNIT_GPS_LATITUDE,       0),
  MS(MAV_ID_GPS_ALT,     STR_SENSOR_GPSALT,                UNIT_METERS,             0),
  MS(MAV_ID_GPS_SPD,     STR_SENSOR_GSPD,                  UNIT_KMH,                1),
  MS(MAV_ID_GPS_SATS,    STR_SENSOR_SATELLITES,            UNIT_RAW,                0),
  MS(MAV_ID_GPS_HDG,     STR_SENSOR_HDG,                   UNIT_DEGREE,             2),
  MS(MAV_ID_ROLL,        STR_SENSOR_ROLL,                  UNIT_DEGREE,             1),
  MS(MAV_ID_PITCH,       STR_SENSOR_PITCH,                 UNIT_DEGREE,             1),
  MS(MAV_ID_YAW,         STR_SENSOR_YAW,                   UNIT_DEGREE,             1),
  MS(MAV_ID_ALT,         STR_SENSOR_ALT,                   UNIT_METERS,             1),
  MS(MAV_ID_VSPD,        STR_SENSOR_VSPD,                  UNIT_METERS_PER_SECOND,  2),
  MS(MAV_ID_ASPD,        STR_SENSOR_ASPD,                  UNIT_KMH,                1),
  MS(MAV_ID_FLIGHT_MODE, STR_SENSOR_FLIGHT_MODE,           UNIT_TEXT,               0),
  MS(MAV_ID_RQLY,        STR_SENSOR_RX_QUALITY,            UNIT_PERCENT,            0),
  MS(MAV_ID_CAPACITY,    STR_SENSOR_CAPACITY,              UNIT_MAH,                0),
  MS(MAV_ID_CELLS,       STR_SENSOR_CELLS,                 UNIT_CELLS,              2),
  MS(MAV_ID_TEMP,        STR_SENSOR_TEMP,                  UNIT_CELSIUS,            1),
  MS(MAV_ID_DATETIME,    STR_SENSOR_GPSDATETIME,           UNIT_DATETIME,           0),
  MS(MAV_ID_RPM,         STR_SENSOR_RPM,                   UNIT_RPMS,               0),
  MS(MAV_ID_NONE,        STR_SENSOR_UNKNOWN,               UNIT_RAW,                0),
};

static const MavlinkSensor& getMavlinkSensor(uint8_t id)
{
  for (auto& s : mavlinkSensors) {
    if (s.id == id) return s;
  }
  return mavlinkSensors[DIM(mavlinkSensors) - 1];
}

static inline void pushVal(uint8_t id, int32_t value, TelemetryUnit unit, uint8_t prec, uint8_t instance = 0)
{
  setTelemetryValue(PROTOCOL_TELEMETRY_MAVLINK, id, 0, instance, value, unit, prec);
}

static const char * const standardModes[] = {
  nullptr, "POSHOLD", "ORBIT", "CRUISE", "ALT HOLD", "RTL", "MISSION", "LAND", "TAKEOFF"
};

template <size_t N>
static const char * modeName(const char * const (&table)[N], uint32_t mode)
{
  return mode < N ? table[mode] : nullptr;
}

#if defined(MAVLINK_ARDUPILOT)
static const char * const planeModes[] = {
  "MANUAL", "CIRCLE", "STABILIZE", "TRAINING", "ACRO", "FBWA", "FBWB", "CRUISE", "AUTOTUNE",
  nullptr, "AUTO", "RTL", "LOITER", "TAKEOFF", "AVOID ADSB", "GUIDED", "INITIALISING",
  "QSTABILIZE", "QHOVER", "QLOITER", "QLAND", "QRTL", "QAUTOTUNE", "QACRO", "THERMAL",
  "LOITER2QLAND", "AUTOLAND"
};

static const char * const copterModes[] = {
  "STABILIZE", "ACRO", "ALT HOLD", "AUTO", "GUIDED", "LOITER", "RTL", "CIRCLE", nullptr, "LAND",
  nullptr, "DRIFT", nullptr, "SPORT", "FLIP", "AUTOTUNE", "POSHOLD", "BRAKE", "THROW",
  "AVOID ADSB", "GUIDED NOGPS", "SMARTRTL", "FLOWHOLD", "FOLLOW", "ZIGZAG", "SYSTEMID",
  "AUTOROTATE", "AUTO RTL", "TURTLE", "RATE_ACRO"
};

static const char * const roverModes[] = {
  "MANUAL", "ACRO", nullptr, "STEERING", "HOLD", "LOITER", "FOLLOW", "SIMPLE", "DOCK", "CIRCLE",
  "AUTO", "RTL", "SMART RTL", nullptr, nullptr, "GUIDED", "INITIALISING"
};

static const char * const subModes[] = {
  "STABILIZE", "ACRO", "ALT HOLD", "AUTO", "GUIDED", nullptr, nullptr, "CIRCLE", nullptr,
  "SURFACE", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "POSHOLD", nullptr, nullptr,
  "MANUAL", "MOTORDETECT", "SURFTRAK"
};

static const char * const trackerModes[] = {
  "MANUAL", "STOP", "SCAN", "SERVO TEST", "GUIDED", nullptr, nullptr, nullptr, nullptr, nullptr,
  "AUTO", nullptr, nullptr, nullptr, nullptr, nullptr, "INITIALISING"
};

static const char * ardupilotModeName(uint8_t type, uint32_t mode)
{
  switch (type) {
    case MAV_TYPE_FIXED_WING:
    case MAV_TYPE_VTOL_TAILSITTER_DUOROTOR:
    case MAV_TYPE_VTOL_TAILSITTER_QUADROTOR:
    case MAV_TYPE_VTOL_TILTROTOR:
    case MAV_TYPE_VTOL_FIXEDROTOR:
    case MAV_TYPE_VTOL_TAILSITTER:
    case MAV_TYPE_VTOL_TILTWING:
    case MAV_TYPE_VTOL_RESERVED5:
      return modeName(planeModes, mode);
    case MAV_TYPE_QUADROTOR:
    case MAV_TYPE_COAXIAL:
    case MAV_TYPE_HELICOPTER:
    case MAV_TYPE_HEXAROTOR:
    case MAV_TYPE_OCTOROTOR:
    case MAV_TYPE_TRICOPTER:
    case MAV_TYPE_DECAROTOR:
    case MAV_TYPE_DODECAROTOR:
      return modeName(copterModes, mode);
    case MAV_TYPE_GROUND_ROVER:
    case MAV_TYPE_SURFACE_BOAT:
      return modeName(roverModes, mode);
    case MAV_TYPE_SUBMARINE:
      return modeName(subModes, mode);
    case MAV_TYPE_ANTENNA_TRACKER:
      return modeName(trackerModes, mode);
    default:
      return nullptr;
  }
}
#endif

static uint32_t currentModeCustom;
static uint8_t currentModeStandard;

void mavlinkSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  (void)subId;

  TelemetrySensor& telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.instance = instance;

  const MavlinkSensor& sensor = getMavlinkSensor((uint8_t)id);
  TelemetryUnit unit = sensor.unit;
  if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE) unit = UNIT_GPS;
  telemetrySensor.init(sensor.name, unit, sensor.precision);

  storageDirty(EE_MODEL);
}

void mavlinkTelemetryDecode(uint8_t module, const mavlink_message_t* msg)
{
  (void)module;

  if (msg->msgid == MAVLINK_MSG_ID_HEARTBEAT && msg->compid == MAV_COMP_ID_TELEMETRY_RADIO)
    return;

  if (msg->msgid != MAVLINK_MSG_ID_RADIO_STATUS) {
    // in case there won't be RADIO_STATUS received
    if (telemetryStreaming == 0) {
      telemetryData.rssi.reset();
      telemetryData.rssi.set(MAVLINK_DEFAULT_RSSI);
    }
    telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  }

  switch (msg->msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: {
      mavlink_heartbeat_t hb;
      mavlink_msg_heartbeat_decode(msg, &hb);
      if (hb.autopilot == MAV_AUTOPILOT_INVALID)
        break;
      const char * name = nullptr;
      if (currentModeStandard && hb.custom_mode == currentModeCustom)
        name = modeName(standardModes, currentModeStandard);
#if defined(MAVLINK_ARDUPILOT)
      if (!name && hb.autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA)
        name = ardupilotModeName(hb.type, hb.custom_mode);
#endif
      char text[24];
      char * p = name ? strAppend(text, name) : strAppendUnsigned(strAppend(text, "Mode "), hb.custom_mode);
      if (!(hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED))
        strAppend(p, "*");
      text[TELEMETRY_SENSOR_TEXT_LENGTH - 1] = ' ';
      setTelemetryText(PROTOCOL_TELEMETRY_MAVLINK, MAV_ID_FLIGHT_MODE, 0, 0, text);
    } break;

    case MAVLINK_MSG_ID_CURRENT_MODE: {
      mavlink_current_mode_t cm;
      mavlink_msg_current_mode_decode(msg, &cm);
      currentModeCustom = cm.custom_mode;
      currentModeStandard = cm.standard_mode;
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
        pushVal(MAV_ID_TQLY, ((int32_t)rs.rssi * 100) / 254, UNIT_PERCENT, 0);
      }
      if (rs.remrssi != UINT8_MAX) {
        int32_t rqly = ((int32_t)rs.remrssi * 100) / 254;
        pushVal(MAV_ID_RQLY, rqly, UNIT_PERCENT, 0);
        if (rs.remrssi) {
          telemetryData.rssi.set((uint8_t)rqly);
          telemetryStreaming = TELEMETRY_TIMEOUT10ms;
        } else {
          telemetryData.rssi.reset();
          telemetryStreaming = 0;
        }
      }
    } break;

    case MAVLINK_MSG_ID_BATTERY_STATUS: {
      mavlink_battery_status_t b;
      mavlink_msg_battery_status_decode(msg, &b);
      if (b.current_consumed != -1)
        pushVal(MAV_ID_CAPACITY, b.current_consumed, UNIT_MAH, 0, b.id);
      if (b.temperature != INT16_MAX)
        pushVal(MAV_ID_TEMP, b.temperature / 10, UNIT_CELSIUS, 1, b.id);
      uint8_t cells = 0;
      while (cells < DIM(b.voltages) && b.voltages[cells] != UINT16_MAX)
        cells++;
      if (cells >= 2 && b.voltages[0] != UINT16_MAX - 1) {
        cells = min<uint8_t>(cells, MAX_CELLS);
        for (uint8_t i = 0; i < cells; i++)
          pushVal(MAV_ID_CELLS, ((uint32_t)cells << 24) | ((uint32_t)i << 16) | (b.voltages[i] / 10),
                  UNIT_CELLS, 2, b.id);
      }
    } break;

#if defined(RTCLOCK)
    case MAVLINK_MSG_ID_SYSTEM_TIME: {
      mavlink_system_time_t t;
      mavlink_msg_system_time_decode(msg, &t);
      gtime_t secs = (gtime_t)(t.time_unix_usec / 1000000);
      struct gtm tm;
      filltm(&secs, &tm);
      if (tm.tm_year < 100)
        break;
      pushVal(MAV_ID_DATETIME, ((uint32_t)(tm.tm_year - 100) << 24) | ((uint32_t)(tm.tm_mon + 1) << 16) |
              ((uint32_t)tm.tm_mday << 8) | 0xFF, UNIT_DATETIME, 0);
      pushVal(MAV_ID_DATETIME, ((uint32_t)tm.tm_hour << 24) | ((uint32_t)tm.tm_min << 16) |
              ((uint32_t)tm.tm_sec << 8), UNIT_DATETIME, 0);
    } break;
#endif

    case MAVLINK_MSG_ID_RAW_RPM: {
      mavlink_raw_rpm_t r;
      mavlink_msg_raw_rpm_decode(msg, &r);
      pushVal(MAV_ID_RPM, (int32_t)r.frequency, UNIT_RPMS, 0, r.index);
    } break;

    default:
      break;
  }
}
