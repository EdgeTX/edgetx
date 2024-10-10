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

#include "hal/watchdog_driver.h"
#include "stm32_hal_ll.h"

#ifdef IWDG1
  #define IWDG IWDG1
#endif

void watchdogInit(unsigned int duration)
{
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32);

  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetReloadCounter(IWDG, duration);

  LL_IWDG_ReloadCounter(IWDG);
  LL_IWDG_Enable(IWDG);
}

void watchdogReset()
{
  LL_IWDG_ReloadCounter(IWDG);  
}
