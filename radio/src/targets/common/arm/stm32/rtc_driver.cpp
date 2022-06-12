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

#include "board.h"
#include "rtc.h"

void rtcSetTime(const struct gtm * t)
{
  g_ms100 = 0; // start of next second begins now

  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  RTC_TimeStructInit(&RTC_TimeStruct);
  RTC_DateStructInit(&RTC_DateStruct);

  RTC_TimeStruct.RTC_Hours = t->tm_hour;
  RTC_TimeStruct.RTC_Minutes = t->tm_min;
  RTC_TimeStruct.RTC_Seconds = t->tm_sec;
  RTC_DateStruct.RTC_Year = t->tm_year - 100;
  RTC_DateStruct.RTC_Month = t->tm_mon + 1;
  RTC_DateStruct.RTC_Date = t->tm_mday;
  
  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
}

void rtcGetTime(struct gtm * t)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
  
  t->tm_hour = RTC_TimeStruct.RTC_Hours;
  t->tm_min  = RTC_TimeStruct.RTC_Minutes;
  t->tm_sec  = RTC_TimeStruct.RTC_Seconds;
  t->tm_year = RTC_DateStruct.RTC_Year + 100; // STM32 year is two decimals only (so base is currently 2000), gtm is based on number of years since 1900
  t->tm_mon  = RTC_DateStruct.RTC_Month - 1;
  t->tm_mday = RTC_DateStruct.RTC_Date;
}

void rtcInit()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  RCC_LSEConfig(RCC_LSE_ON);
  
  // Prevent lockup in case of 32kHz oscillator failure
  uint32_t i = 0;
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
    if (++i > 1000000)
      return;
  }
  
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  RCC_RTCCLKCmd(ENABLE);
  RTC_WaitForSynchro();

#if !defined(BOOT)
  RTC_InitTypeDef RTC_InitStruct;

  // RTC time base = LSE / ((AsynchPrediv+1) * (SynchPrediv+1)) = 1 Hz*/
  RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
  RTC_InitStruct.RTC_AsynchPrediv = 127;
  RTC_InitStruct.RTC_SynchPrediv = 255;
  RTC_Init(&RTC_InitStruct);

  struct gtm utm;
  rtcGetTime(&utm);
  g_rtcTime = gmktime(&utm);
#endif

#if defined(RTC_BACKUP_RAM) && !defined(BOOT)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  PWR_BackupRegulatorCmd(ENABLE);
#endif
}
