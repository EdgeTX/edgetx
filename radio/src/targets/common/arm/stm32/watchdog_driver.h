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

#define WDG_DURATION                              500 /*ms*/
void watchdogInit(unsigned int duration);
#if defined(SIMU)
  #define WAS_RESET_BY_WATCHDOG()               (false)
  #define WAS_RESET_BY_SOFTWARE()               (false)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (false)
  #define WDG_ENABLE(x)
  #define WDG_RESET()
#else
  #if defined(WATCHDOG)
    #define WDG_ENABLE(x)                       watchdogInit(x)
    #define WDG_RESET()                         IWDG->KR = 0xAAAA
  #else
    #define WDG_ENABLE(x)
    #define WDG_RESET()
  #endif
  #define WAS_RESET_BY_WATCHDOG()               (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF))
  #define WAS_RESET_BY_SOFTWARE()               (RCC->CSR & RCC_CSR_SFTRSTF)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_SFTRSTF))
#endif
