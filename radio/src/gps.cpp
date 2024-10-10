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
#include "gps_nmea.h"
#include "gps_ubx.h"
#include <ctype.h>

gpsdata_t gpsData;
static int gpsProtocol = -1;
const etx_serial_driver_t* gpsSerialDrv = nullptr;
void* gpsSerialCtx = nullptr;

#define DIGIT_TO_VAL(_x)    (_x - '0')

uint32_t GPS_coord_to_degrees(const char * coordinateString)
{
  const char * fieldSeparator, * remainingString;
  uint8_t degrees = 0, minutes = 0;
  uint16_t fractionalMinutes = 0;
  uint8_t digitIndex;

  // scan for decimal point or end of field
  for (fieldSeparator = coordinateString; isdigit((unsigned char) *fieldSeparator); fieldSeparator++) {
    if (fieldSeparator >= coordinateString + 15)
      return 0; // stop potential fail
  }
  remainingString = coordinateString;

  // convert degrees
  while ((fieldSeparator - remainingString) > 2) {
    if (degrees)
      degrees *= 10;
    degrees += DIGIT_TO_VAL(*remainingString++);
  }
  // convert minutes
  while (fieldSeparator > remainingString) {
    if (minutes)
      minutes *= 10;
    minutes += DIGIT_TO_VAL(*remainingString++);
  }
  // convert fractional minutes
  // expect up to four digits, result is in
  // ten-thousandths of a minute
  if (*fieldSeparator == '.') {
    remainingString = fieldSeparator + 1;
    for (digitIndex = 0; digitIndex < 4; digitIndex++) {
      fractionalMinutes *= 10;
      if (isdigit((unsigned char) *remainingString))
        fractionalMinutes += *remainingString++ - '0';
    }
  }
  // TODO return degrees * 10000000UL + (minutes * 1000000UL + fractionalMinutes * 100UL) / 6;
  return degrees * 1000000UL + (minutes * 100000UL + fractionalMinutes * 10UL) / 6;
}

static void changeBaudrate()
{
  const int baudrates_count = 5;
  const uint32_t baudrates[] = {9600, 57600, 115200, 19200, 38400};
  static uint8_t current_rate = 0;

  auto setBaudrate = gpsSerialDrv->setBaudrate;
  if (setBaudrate == nullptr) return;
  setBaudrate(gpsSerialCtx, baudrates[++current_rate % baudrates_count]);
}

static void autodetectProtocol(uint8_t c)
{
  static tmr10ms_t time;
  static uint8_t state = 0;
  static tmr10ms_t firstPacketNMEA = 0;

  switch (state)  {
    case 0: // Init
      time = get_tmr10ms();
      state = 1;
    case 1: // Wait for a valid packet
      if (gpsNewFrameNMEA(c)) {
        if (!firstPacketNMEA) {
          firstPacketNMEA = time;
        } else if (time - firstPacketNMEA > 200) {
          // continuous stream of NMEA packets for 2 seconds, but no UBX packets
          gpsProtocol = GPS_PROTOCOL_NMEA;
        }
        state = 0;
        return;
      }

      if (gpsNewFrameUBX(c, true)) {
        gpsProtocol = GPS_PROTOCOL_UBX;
        state = 0;
        return;
      }

      uint32_t new_time = get_tmr10ms();
      if (new_time - time > 50) {
        // No message received
        firstPacketNMEA = 0;
        changeBaudrate();
        time = new_time;
      }
  }
}

static void detectDisconnected(bool has_frame)
{
  static tmr10ms_t time = 0;

  if (has_frame) {
    time = get_tmr10ms();
  } else if (time > 0 && get_tmr10ms() - time > 500) {
    gpsProtocol = GPS_PROTOCOL_AUTO;
    time = get_tmr10ms();
  }
}


void gpsNewData(uint8_t c)
{
  switch (gpsProtocol) {
    case GPS_PROTOCOL_NMEA:
      detectDisconnected(gpsNewFrameNMEA(c));
      break;
    case GPS_PROTOCOL_UBX:
      detectDisconnected(gpsNewFrameUBX(c, false));
      break;
    case GPS_PROTOCOL_AUTO:
      autodetectProtocol(c);
      break;
  }
}


#if defined(DEBUG)
uint8_t gpsTraceEnabled = false;
#endif

void gpsSetSerialDriver(void* ctx, const etx_serial_driver_t* drv, int protocol)
{
  gpsProtocol = protocol;
  gpsSerialCtx = ctx;
  gpsSerialDrv = drv;
}

void gpsWakeup()
{
  if (!gpsSerialDrv) return;

  auto _getByte = gpsSerialDrv->getByte;
  if (!_getByte) return;

  static tmr10ms_t time = get_tmr10ms();
  uint8_t byte;
  while (_getByte(gpsSerialCtx, &byte)) {
#if defined(DEBUG)
    if (gpsTraceEnabled) {
      dbgSerialPutc(byte);
    }
#endif
    gpsNewData(byte);
    time = get_tmr10ms();
  }

  if (get_tmr10ms() - time > 20) {
    changeBaudrate();
    time = get_tmr10ms();
  }
}

void gpsSendFrame(const char * frame)
{
  switch (gpsProtocol) {
    case GPS_PROTOCOL_NMEA:
      gpsSendFrameNMEA(frame);
      break;
  }
}
