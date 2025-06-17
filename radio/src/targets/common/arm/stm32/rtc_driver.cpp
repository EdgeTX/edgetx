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

#include "stm32_hal.h"
#include "rtc.h"

// LSEDRV resets to LOW, which is marginal for the crystals fitted on these
// boards. Override with -DLSE_DRIVE_STRENGTH if a board needs less.
#if !defined(LSE_DRIVE_STRENGTH) && \
    (defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5))
  #define LSE_DRIVE_STRENGTH  RCC_LSEDRIVE_HIGH
#endif

RTC_HandleTypeDef rtc = {};

void rtcSetTime(const struct gtm * t)
{
  g_ms100 = 0; // start of next second begins now

  RTC_TimeTypeDef RTC_TimeStruct = {};
  RTC_DateTypeDef RTC_DateStruct = {};

  RTC_TimeStruct.Hours = t->tm_hour;
  RTC_TimeStruct.Minutes = t->tm_min;
  RTC_TimeStruct.Seconds = t->tm_sec;
  RTC_DateStruct.Year = t->tm_year - 100;
  RTC_DateStruct.Month = t->tm_mon + 1;
  RTC_DateStruct.Date = t->tm_mday;
  HAL_RTC_SetTime(&rtc, &RTC_TimeStruct, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&rtc, &RTC_DateStruct, RTC_FORMAT_BIN);
}

void rtcGetTime(struct gtm * t)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  HAL_RTC_GetTime(&rtc, &RTC_TimeStruct, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&rtc, &RTC_DateStruct, RTC_FORMAT_BIN);
  
  t->tm_hour = RTC_TimeStruct.Hours;
  t->tm_min  = RTC_TimeStruct.Minutes;
  t->tm_sec  = RTC_TimeStruct.Seconds;
  t->tm_year = RTC_DateStruct.Year + 100; // STM32 year is two decimals only (so base is currently 2000), gtm is based on number of years since 1900
  t->tm_mon  = RTC_DateStruct.Month - 1;
  t->tm_mday = RTC_DateStruct.Date;
}

#if defined(LSE_DRIVE_STRENGTH)
// LSEDRV can only be written while the LSE is stopped, and it is kept in the
// backup domain, so only stop the oscillator when the drive is not already set.
static void rtcSetLSEDriveStrength()
{
  uint32_t drive = LSE_DRIVE_STRENGTH;

#if defined(RCC_VER_X)
  // rev.Y and below encode the two MEDIUM levels inverted
  if ((drive == RCC_LSEDRIVE_MEDIUMLOW || drive == RCC_LSEDRIVE_MEDIUMHIGH) &&
      HAL_GetREVID() <= REV_ID_Y) {
    drive = ~drive & RCC_BDCR_LSEDRV_Msk;
  }
#endif

  if ((RCC->BDCR & RCC_BDCR_LSEDRV) == drive) return;

  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
#if !defined(STM32H7RS) && !defined(STM32H5)
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
#endif
  RCC_OscInitStruct.LSEState       = RCC_LSE_OFF;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) return;

  MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, drive);
}
#endif

void rtcInit()
{
  rtc.Instance = RTC;

#if !defined(STM32H7) && !defined(STM32H7RS) && !defined(STM32H5)
  // Power management peripheral is active by default at startup level in
  // STM32h7xx lines.
  __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(STM32H7) || defined(STM32H7RS)
  __HAL_RCC_BKPRAM_CLK_ENABLE();
#endif

  HAL_PWR_EnableBkUpAccess();
#if defined(LSE_DRIVE_STRENGTH)
  rtcSetLSEDriveStrength();
#endif

  // Enable LSE Oscillator
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
#if !defined(STM32H7RS) && !defined(STM32H5)
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
#endif
  RCC_OscInitStruct.LSEState       = RCC_LSE_ON;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK) {
    __HAL_RCC_RTC_CLKPRESCALER(RCC_RTCCLKSOURCE_LSE);
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
  }

  __HAL_RCC_RTC_ENABLE();
  HAL_RTC_WaitForSynchro(&rtc);

#if !defined(BOOT)
  // RTC time base = LSE / ((AsynchPrediv+1) * (SynchPrediv+1)) = 1 Hz*/
  rtc.Init.HourFormat = RTC_HOURFORMAT_24;
  rtc.Init.AsynchPrediv = 127;
  rtc.Init.SynchPrediv = 255;
  rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&rtc);

  HAL_PWR_EnableBkUpAccess();
  HAL_RTC_WaitForSynchro(&rtc);

  struct gtm utm;
  rtcGetTime(&utm);
  g_rtcTime = gmktime(&utm);
#endif

#if defined(RTC_BACKUP_RAM) && !defined(BOOT)
#if defined(STM32F4)
  __HAL_RCC_BKPSRAM_CLK_ENABLE();
#endif
#if !defined(STM32H5)
  HAL_PWREx_EnableBkUpReg();
#endif
  HAL_PWR_EnableBkUpAccess();
#endif
}

void rtcDisableBackupReg()
{
#if defined(RTC_BACKUP_RAM)
#if defined(STM32F4)
  __HAL_RCC_BKPSRAM_CLK_DISABLE();
#endif
#if !defined(STM32H5)
  HAL_PWREx_DisableBkUpReg();
#endif
  HAL_PWR_DisableBkUpAccess();
#endif
}
