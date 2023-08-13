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

#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_

#include <inttypes.h>
#include "cpu_id.h"

#if !defined(SIMU) && !defined(BACKUP)

#if defined(STM32F4)
  #include "stm32f4xx.h"
  #include "stm32f4xx_rcc.h"
  #include "stm32f4xx_syscfg.h"
  #include "stm32f4xx_gpio.h"
  #include "stm32f4xx_exti.h"
  #include "stm32f4xx_tim.h"
  #include "stm32f4xx_spi.h"
  #include "stm32f4xx_pwr.h"
  #include "stm32f4xx_dma.h"
  #include "stm32f4xx_flash.h"
  #include "stm32f4xx_dbgmcu.h"
  #include "misc.h"
#else
  #include "stm32f2xx.h"
  #include "stm32f2xx_rcc.h"
  #include "stm32f2xx_syscfg.h"
  #include "stm32f2xx_gpio.h"
  #include "stm32f2xx_exti.h"
  #include "stm32f2xx_tim.h"
  #include "stm32f2xx_spi.h"
  #include "stm32f2xx_pwr.h"
  #include "stm32f2xx_dma.h"
  #include "stm32f2xx_flash.h"
  #include "stm32f2xx_dbgmcu.h"
  #include "misc.h"
  #include "dwt.h"    // the old ST library that we use does not define DWT register for STM32F2xx
#endif

#endif

#include "usb_driver.h"

#if defined(SIMU)
#include "../simu/simpgmspace.h"
#endif

// Delays driver
#define SYSTEM_TICKS_1MS  ((CPU_FREQ + 500) / 1000)
#define SYSTEM_TICKS_1US  ((CPU_FREQ + 500000)  / 1000000)
#define SYSTEM_TICKS_01US ((CPU_FREQ + 5000000) / 10000000)

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t ticksNow()
{
#if !defined(SIMU) && !defined(BACKUP)
  return DWT->CYCCNT;
#else
  return 0;
#endif
}
  
#ifdef __cplusplus
}
#endif

#include "delays_driver.h"

#endif
