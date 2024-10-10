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

/*
 * This file is based on code from Cleanflight project
 * https://github.com/cleanflight/cleanflight
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "edgetx.h"
#include "gps.h"
#include <ctype.h>

/* This is a light implementation of a GPS frame decoding
   This should work with most of modern GPS devices configured to output 5 frames.
   It assumes there are some NMEA GGA frames to decode on the serial bus
   Now verifies checksum correctly before applying data

   Here we use only the following data :
     - latitude
     - longitude
     - GPS fix is/is not ok
     - GPS num sat (4 is enough to be +/- reliable)
     // added by Mis
     - GPS altitude (for OSD displaying)
     - GPS speed (for OSD displaying)
*/

#define NO_FRAME   0
#define FRAME_GGA  1
#define FRAME_RMC  2

#define DIGIT_TO_VAL(_x)    (_x - '0')

extern const etx_serial_driver_t* gpsSerialDrv;
extern void* gpsSerialCtx;

char hex(uint8_t b) {
  return b > 9 ? b + 'A' - 10 : b + '0';
}

void gpsSendFrameNMEA(const char * frame)
{
  if (!gpsSerialDrv) return;

  auto _sendByte = gpsSerialDrv->sendByte;
  if (!_sendByte) return;

  // send given frame, add checksum and CRLF
  uint8_t parity = 0;
  TRACE_NOCRLF("gps> %s", frame);

  while (*frame) {
    if (*frame != '$') parity ^= *frame;
    _sendByte(gpsSerialCtx, *frame);
    ++frame;
  }

  _sendByte(gpsSerialCtx, '*');
  _sendByte(gpsSerialCtx, hex(parity >> 4));
  _sendByte(gpsSerialCtx, hex(parity & 0x0F));
  _sendByte(gpsSerialCtx, '\r');
  _sendByte(gpsSerialCtx, '\n');

  TRACE("*%02x", parity);
}

// helper functions
uint32_t grab_fields(char * src, uint8_t mult)
{
  uint32_t i;
  uint32_t tmp = 0;
  for (i = 0; src[i] != 0; i++) {
    if (src[i] == '.') {
      i++;
      if (mult == 0)
        break;
      else
        src[i + mult] = 0;
    }
    tmp *= 10;
    if (src[i] >= '0' && src[i] <= '9')
      tmp += src[i] - '0';
    if (i >= 15)
      return 0; // out of bounds
  }
  return tmp;
}

typedef struct gpsDataNmea_s
{
  uint8_t fix;
  int32_t latitude;
  int32_t longitude;
  uint8_t numSat;
  uint16_t altitude;
  uint16_t speed;
  uint16_t groundCourse;
  uint16_t hdop;
  uint32_t date;
  uint32_t time;
} gpsDataNmea_t;

bool gpsNewFrameNMEA(char c)
{
  static gpsDataNmea_t gps_Msg;

  uint8_t frameOK = 0;
  static uint8_t param = 0, offset = 0, parity = 0;
  static char string[15];
  static uint8_t checksum_param, gps_frame = NO_FRAME;

  switch (c) {
    case '$':
      param = 0;
      offset = 0;
      parity = 0;
      break;
    case ',':
    case '*':
      string[offset] = 0;
      if (param == 0) {
        // Frame identification (accept all GPS talkers (GP: GPS, GL:Glonass, GN:combination, etc...))
        gps_frame = NO_FRAME;
        if (string[0] == 'G' && string[2] == 'G' && string[3] == 'G' && string[4] == 'A') {
          gps_frame = FRAME_GGA;
        }
        else if (string[0] == 'G' && string[2] == 'R' && string[3] == 'M' && string[4] == 'C') {
          gps_frame = FRAME_RMC;
        }
        else {
          // turn off this frame (do this only once a second)
          static gtime_t lastGpsCmdSent = 0;
          if (string[0] == 'G' && g_rtcTime != lastGpsCmdSent) {
            lastGpsCmdSent = g_rtcTime;
            char cmd[] = "$PUBX,40,GSV,0,0,0,0";
            cmd[9]  = string[2];
            cmd[10] = string[3];
            cmd[11] = string[4];
            gpsSendFrameNMEA(cmd);
          }
        }
      }

      switch (gps_frame) {
        case FRAME_GGA:        //************* GPGGA FRAME parsing
          switch (param) {
            case 2:
              gps_Msg.latitude = GPS_coord_to_degrees(string);
              break;
            case 3:
              if (string[0] == 'S')
                gps_Msg.latitude *= -1;
              break;
            case 4:
              gps_Msg.longitude = GPS_coord_to_degrees(string);
              break;
            case 5:
              if (string[0] == 'W')
                gps_Msg.longitude *= -1;
              break;
            case 6:
              if (string[0] > '0') {
                gps_Msg.fix = 1;
              }
              else {
                gps_Msg.fix = 0;
              }
              break;
            case 7:
              gps_Msg.numSat = grab_fields(string, 0);
              break;
            case 8:
              gps_Msg.hdop = grab_fields(string, 1) * 10;
              break;
            case 9:
              gps_Msg.altitude = grab_fields(string, 0);     // altitude in meters added by Mis
              break;
          }
          break;
        case FRAME_RMC:        //************* GPRMC FRAME parsing
          switch (param) {
            case 1:
              gps_Msg.time = grab_fields(string, 0);
              break;
            case 2:
              if (string[0] == 'A') {
                gps_Msg.fix = 1;
              }
              else {
                gps_Msg.fix = 0;
              }
              break;
            case 7:
              gps_Msg.speed = ((grab_fields(string, 1) * 5144L) / 1000L);    // speed in cm/s added by Mis
              break;
            case 8:
              gps_Msg.groundCourse = (grab_fields(string, 1));      // ground course deg * 10
              break;
            case 9:
              gps_Msg.date = grab_fields(string, 0);
              break;
          }
          break;

      }

      param++;
      offset = 0;
      if (c == '*')
        checksum_param = 1;
      else
        parity ^= c;
      break;
    case '\r':
    case '\n':
      if (checksum_param) {   //parity checksum
        uint8_t checksum = 16 * ((string[0] >= 'A') ? string[0] - 'A' + 10 : string[0] - '0') +
                           ((string[1] >= 'A') ? string[1] - 'A' + 10 : string[1] - '0');
        if (checksum == parity) {
          gpsData.packetCount++;
          switch (gps_frame) {
            case FRAME_GGA:
              frameOK = 1;
              gpsData.fix = gps_Msg.fix;
              gpsData.numSat = gps_Msg.numSat;
              gpsData.hdop = gps_Msg.hdop;
              if (gps_Msg.fix) {
                __disable_irq();    // do the atomic update of lat/lon
                gpsData.latitude = gps_Msg.latitude;
                gpsData.longitude = gps_Msg.longitude;
                gpsData.altitude = gps_Msg.altitude;
                __enable_irq();
              }
              break;
            case FRAME_RMC:
              gpsData.speed = gps_Msg.speed;
              gpsData.groundCourse = gps_Msg.groundCourse;
#if defined(RTCLOCK)
              // set RTC clock if needed
              if (g_eeGeneral.adjustRTC && gps_Msg.fix) {
                div_t qr = div(gps_Msg.date, 100);
                uint8_t year = qr.rem;
                qr = div(qr.quot, 100);
                uint8_t mon = qr.rem;
                uint8_t day = qr.quot;
                qr = div(gps_Msg.time, 100);
                uint8_t sec = qr.rem;
                qr = div(qr.quot, 100);
                uint8_t min = qr.rem;
                uint8_t hour = qr.quot;
                rtcAdjust(year+2000, mon, day, hour, min, sec);
              }
#endif
          } // end switch
        }
        else {
          gpsData.errorCount++;
        }
      }
      checksum_param = 0;
      break;
    default:
      if (offset < 15)
        string[offset++] = c;
      if (!checksum_param)
        parity ^= c;
  }
  return frameOK;
}
