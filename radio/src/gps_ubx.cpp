/*
 * Copyright (C) EdgeTX
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
 *
 * This file is based on code from Betaflight project
 * https://github.com/betaflight/betaflight
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>

#include "gps.h"
#include "edgetx.h"

extern const etx_serial_driver_t* gpsSerialDrv;
extern void* gpsSerialCtx;

// https://docs.ros.org/en/noetic/api/ublox_msgs/html/msg/NavPVT.html
typedef struct ubxNavPvt_s {
  uint32_t time;         // GPS Millisecond time of week [ms]
  uint16_t year;         // Year (UTC)
  uint8_t month;         // Month, range 1..12 (UTC)
  uint8_t day;           // Day of month, range 1..31 (UTC)
  uint8_t hour;          // Hour of day, range 0..23 (UTC)
  uint8_t min;           // Minute of hour, range 0..59 (UTC)
  uint8_t sec;           // Seconds of minute, range 0..60 (UTC)
  uint8_t valid;         // Validity flags
  uint32_t tAcc;         // time accuracy estimate [ns] (UTC)
  int32_t nano;          // fraction of a second [ns], range -1e9 .. 1e9 (UTC)
  uint8_t fixType;       // GNSS fix Type
  uint8_t flags;         // Fix Status Flags
  uint8_t flags2;        // Additional Flags
  uint8_t numSV;         // Number of SVs used in Nav Solution
  int32_t lon;           // Longitude [deg / 1e-7]
  int32_t lat;           // Latitude [deg / 1e-7]
  int32_t height;        // Height above Ellipsoid [mm]
  int32_t hMSL;          // Height above mean sea level [mm]
  uint32_t hAcc;         // Horizontal Accuracy Estimate [mm]
  uint32_t vAcc;         // Vertical Accuracy Estimate [mm]
  int32_t velN;          // NED north velocity [mm/s]
  int32_t velE;          // NED east velocity [mm/s]
  int32_t velD;          // NED down velocity [mm/s]
  int32_t gSpeed;        // Ground Speed (2-D) [mm/s]
  int32_t headMot;       // Heading of motion 2-D [deg / 1e-5]
  uint32_t sAcc;         // Speed Accuracy Estimate [mm/s]
  uint32_t headAcc;      // Heading Accuracy Estimate (both motion & vehicle)
  uint16_t pDOP;         // Position DOP [1 / 0.01]
  uint8_t flags3;        // Additional Flags
  uint8_t reserved0[5];  // Reserved
  int32_t headVeh;       // Heading of vehicle (2-D) [deg / 1e-5]
  int16_t magDec;        // # Magnetic declination [deg / 1e-2]
  uint16_t magAcc;       // Magnetic declination accuracy [deg / 1e-2]
} ubxNavPvt_t;

typedef struct ubxNavDop_s {
  uint32_t itow;  // GPS Millisecond Time of Week
  uint16_t gdop;  // Geometric DOP
  uint16_t pdop;  // Position DOP
  uint16_t tdop;  // Time DOP
  uint16_t vdop;  // Vertical DOP
  uint16_t hdop;  // Horizontal DOP
  uint16_t ndop;  // Northing DOP
  uint16_t edop;  // Easting DOP
} ubxNavDop_t;

typedef struct ubxCfgMsg_s {
  uint8_t msgClass;
  uint8_t msgID;
  uint8_t rate;
} ubxCfgMsg_t;

static void gpsSendMessage(uint16_t msg_type, uint16_t msg_len,
                           uint8_t* payload)
{
  if (!gpsSerialDrv) return;

  auto _sendByte = gpsSerialDrv->sendByte;
  if (!_sendByte) return;

  uint8_t ck_a, ck_b;
  gpsSerialDrv->sendByte(gpsSerialCtx, 0xb5);
  gpsSerialDrv->sendByte(gpsSerialCtx, 0x62);

  ck_b = ck_a = (msg_type >> 8);
  gpsSerialDrv->sendByte(gpsSerialCtx, msg_type >> 8);

  ck_b += (ck_a += (msg_type & 0xff));
  gpsSerialDrv->sendByte(gpsSerialCtx, msg_type & 0xff);
  ck_b += (ck_a += (msg_len & 0xff));
  gpsSerialDrv->sendByte(gpsSerialCtx, msg_len & 0xff);
  ck_b += (ck_a += (msg_len >> 8));
  gpsSerialDrv->sendByte(gpsSerialCtx, msg_len >> 8);
  for (int n = 0; n < msg_len; n++) {
    ck_b += (ck_a += payload[n]);
  }
  if (msg_len > 0) {
    gpsSerialDrv->sendBuffer(gpsSerialCtx, payload, msg_len);
  }
  gpsSerialDrv->sendByte(gpsSerialCtx, ck_a);
  gpsSerialDrv->sendByte(gpsSerialCtx, ck_b);
}

static void configureGps(bool detect)
{
  static int state = 0;

  if (detect) state = 0;

  auto txCompleted = gpsSerialDrv->txCompleted;
  if (txCompleted && !txCompleted(gpsSerialCtx)) return;

  switch (state) {
    case 0:
      gpsSendMessage(0x0a04, 0, nullptr);
      ubxCfgMsg_t val;
      val.msgClass = 0x01;
      val.msgID = 0x07;
      val.rate = 1;
      gpsSendMessage(0x0601, sizeof(ubxCfgMsg_t), (uint8_t*)&val);
      val.msgID = 0x04;
      gpsSendMessage(0x0601, sizeof(ubxCfgMsg_t), (uint8_t*)&val);
      state++;
  }
}

static void gpsProcessMessage(uint16_t msg_type, uint16_t msg_len,
                              uint8_t* payload)
{
  if (msg_type == 0x0107) {
    auto pvt = (ubxNavPvt_t*)payload;
    gpsData.fix = pvt->flags & 0x01;
    gpsData.numSat = pvt->numSV;
    gpsData.speed = pvt->gSpeed / 100;            // speed in 0.1m/s
    gpsData.groundCourse = pvt->headMot / 10000;  // degrees * 10
    if (gpsData.fix) {
      __disable_irq();                     // do the atomic update of lat/lon
      gpsData.longitude = pvt->lon / 10;   // degrees * 1.000.000
      gpsData.latitude = pvt->lat / 10;    // degrees * 1.000.000
      gpsData.altitude = pvt->hMSL / 100;  // altitude in 0.1m
      __enable_irq();
    }

#if defined(RTCLOCK)
    // set RTC clock if needed
    if (g_eeGeneral.adjustRTC && (pvt->valid & 0x03) == 0x03) {
      rtcAdjust(pvt->year, pvt->month, pvt->day, pvt->hour, pvt->min, pvt->sec);
    }
#endif
  }

  if (msg_type == 0x0104) {
    auto dop = (ubxNavDop_t*)payload;
    gpsData.hdop = dop->hdop;
  }
}

bool gpsNewFrameUBX(uint8_t c, bool detect)
{
  configureGps(detect);

  const auto BUFFER_SIZE = 100u;
  static int state = 0;
  static uint16_t msg_type;
  static uint16_t msg_len;
  static uint16_t msg_pos;
  static uint8_t buf[BUFFER_SIZE];
  static uint8_t ck_a;
  static uint8_t ck_b;

  bool result = false;

  switch (state) {
    case 0:  // Search Sync Char 1
      if (c == 0xb5) {
        state = 1;
      }
      break;
    case 1:  // Search Sync Char 2
      if (c == 0x62) {
        state = 2;
      } else {
        state = 0;
      }
      break;
    case 2:  // Get message class
      ck_b = ck_a = c;
      msg_type = c << 8;
      state = 3;
      break;
    case 3:  // Get message id
      ck_b += (ck_a += c);
      msg_type |= c;
      state = 4;
      break;
    case 4:  // Get length 1
      ck_b += (ck_a += c);
      msg_len = c;
      state = 5;
      break;
    case 5:  // Get length 2
      ck_b += (ck_a += c);
      msg_len |= c << 8;
      msg_pos = 0;
      if (msg_len == 0) {
        state = 7;
        break;
      }
      if ((msg_type != 0x0107 && msg_type != 0x0104) || msg_len > BUFFER_SIZE) {
        state = 9;
        break;
      }
      state = 6;
      break;
    case 6:  // Get payload
      ck_b += (ck_a += c);
      buf[msg_pos++] = c;
      if (msg_pos >= msg_len) {
        state = 7;
      }
      break;
    case 7:  // Get checksum 1
      if (c == ck_a) {
        state = 8;
      } else {
        gpsData.errorCount++;
        state = 0;
      }
      break;
    case 8:  // Get checksum 2, end of frame
      if (c == ck_b) {
        gpsProcessMessage(msg_type, msg_len, buf);
        gpsData.packetCount++;
        result = true;
      } else {
        gpsData.errorCount++;
      }
      state = 0;
      break;
    case 9:  // Discard message
      ck_b += (ck_a += c);
      msg_pos++;
      if (msg_pos >= msg_len) {
        state = 10;
        result = true;
      }
      break;
    case 10:  // Get checksum 1 of discarded message
      if (c == ck_a) {
        state = 11;
      } else {
        gpsData.errorCount++;
        state = 0;
      }
      break;
    case 11:  // Get checksum 2 of discarded message
      if (c == ck_b) {
        // We have a valid UBX message
        gpsData.packetCount++;
        result = true;
      } else {
        gpsData.errorCount++;
      }
      state = 0;
      break;
  }

  return result;
}
