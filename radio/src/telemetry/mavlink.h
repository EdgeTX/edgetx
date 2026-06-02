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

#include <stdint.h>

typedef struct __mavlink_message mavlink_message_t;

enum MavlinkSensorId {
  MAV_ID_NONE = 0,
  MAV_ID_RSSI,
  MAV_ID_BATT_V,
  MAV_ID_BATT_A,
  MAV_ID_BATT_PCT,
  MAV_ID_GPS,
  MAV_ID_GPS_ALT,
  MAV_ID_GPS_SPD,
  MAV_ID_GPS_SATS,
  MAV_ID_GPS_HDG,
  MAV_ID_ROLL,
  MAV_ID_PITCH,
  MAV_ID_YAW,
  MAV_ID_ALT,
  MAV_ID_VSPD,
  MAV_ID_ASPD,
  MAV_ID_FLIGHT_MODE,
  MAV_ID_RRSSI,
};

void mavlinkTelemetryDecode(uint8_t module, const mavlink_message_t* msg);
void mavlinkSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);
