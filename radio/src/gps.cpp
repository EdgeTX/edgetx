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

#include "opentx.h"
#include "gps.h"
#include "gps_nmea.h"
#include <ctype.h>

gpsdata_t gpsData;

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

void gpsNewData(uint8_t c)
{
  gpsNewFrameNMEA(c);
}

const etx_serial_driver_t* gpsSerialDrv = nullptr;
void* gpsSerialCtx = nullptr;

#if defined(DEBUG)
uint8_t gpsTraceEnabled = false;
#endif

void gpsSetSerialDriver(void* ctx, const etx_serial_driver_t* drv)
{
  gpsSerialCtx = ctx;
  gpsSerialDrv = drv;
}

void gpsWakeup()
{
  if (!gpsSerialDrv) return;

  auto _getByte = gpsSerialDrv->getByte;
  if (!_getByte) return;

  uint8_t byte;
  while (_getByte(gpsSerialCtx, &byte)) {
#if defined(DEBUG)
    if (gpsTraceEnabled) {
      dbgSerialPutc(byte);
    }
#endif
    gpsNewData(byte);
  }
}

void gpsSendFrame(const char * frame)
{
  gpsSendFrameNMEA(frame);
}
