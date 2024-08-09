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

#include <inttypes.h>

#define SECS_PER_HOUR   3600ul
#define SECS_PER_DAY    86400ul
#define TM_YEAR_BASE    1900

#define TIME_T_MIN      (-LONG_MAX)
#define TIME_T_MAX      (LONG_MAX)

#define SET_LOAD_DATETIME(x)  { rtcSetTime(x); g_rtcTime = gmktime(x); }

typedef long int gtime_t;

struct gtm
{
  int8_t tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
  int8_t tm_min;                   /* Minutes.     [0-59] */
  int8_t tm_hour;                  /* Hours.       [0-23] */
  int8_t tm_mday;                  /* Day.         [1-31] */
  int8_t tm_mon;                   /* Month.       [0-11] */
  uint8_t tm_year;                 /* Year - 1900. Limited to the year 2155. */
  int8_t tm_wday;                  /* Day of week. [0-6] */
  int16_t tm_yday;                 /* Day of year. [0-365] Needed internally for calculations */
};

extern gtime_t g_rtcTime;
extern uint8_t g_ms100; // global to allow time set function to reset to zero

bool rtcIsValid();
void rtcInit();
void rtcSetTime(const struct gtm * tm);
gtime_t gmktime (struct gtm *tm);
uint8_t rtcAdjust(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif
void gettime(struct gtm * tm);
#if defined(__cplusplus) && !defined(SIMU)
}
#endif
