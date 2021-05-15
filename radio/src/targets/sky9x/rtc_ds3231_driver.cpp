/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include "i2c_driver.h"

#define DS3231_I2C_CHIPID 0x68

void readRtc()
{
  uint8_t buffer[7];
  uint8_t address[]= {0};
  gtm utm;
  int res = i2cReadBuffer(DS3231_I2C_CHIPID, address, 1, buffer, 7);
  if (res != 0)
    return;

  // Set the date and time
  utm.tm_sec = fromBCD(buffer[0] & 0x7F);
  utm.tm_min = fromBCD(buffer[1] & 0x7F);
  utm.tm_hour = fromBCD(buffer[2] & 0x3F);
  utm.tm_mday = fromBCD(buffer[4] & 0x3F);
  utm.tm_mon = fromBCD(buffer[5] & 0x1F) - 1;
  utm.tm_year = fromBCD(buffer[6]) + 100;
  g_rtcTime = gmktime(&utm);
}

void writeRtc(const gtm* ptr)
{
  g_ms100 = 0; // start of next second begins now
  uint8_t buffer[7];
  uint8_t address[]= {0};

  buffer[0] = toBCD(ptr->tm_sec);
  buffer[1] = toBCD(ptr->tm_min);
  buffer[2] = toBCD(ptr->tm_hour);
  buffer[3] = toBCD(ptr->tm_wday);
  buffer[4] = toBCD(ptr->tm_mday);
  buffer[5] = toBCD(ptr->tm_mon + 1);
  uint8_t offset = ptr->tm_year >= 100 ? 100 : 0;
  buffer[6] = toBCD(ptr->tm_year - offset);
  i2cWriteBuffer(DS3231_I2C_CHIPID, address, 1, buffer, 7);
}

void rtcSetTime(const struct gtm * t)
{
  writeRtc(t);
}

void rtcInit()
{
  readRtc();
}
