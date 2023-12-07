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

#include "stm32_gpio_driver.h"
#include "stm32_hal_ll.h"

void stm32_gpio_enable_clock(GPIO_TypeDef *GPIOx)
{
  uint32_t reg_idx = (((uint32_t) GPIOx) - GPIOA_BASE) / 0x0400UL;
#if defined(RCC_AHB4ENR_GPIOAEN)
  uint32_t reg_msk = RCC_AHB4ENR_GPIOAEN << reg_idx;
  LL_AHB4_GRP1_EnableClock(reg_msk);
#else
  uint32_t reg_msk = RCC_AHB1ENR_GPIOAEN << reg_idx;
  LL_AHB1_GRP1_EnableClock(reg_msk);
#endif
}

