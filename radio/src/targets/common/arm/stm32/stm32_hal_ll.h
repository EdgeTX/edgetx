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

#include "stm32_cmsis.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define USE_FULL_LL_DRIVER
#if defined(STM32F4)
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_bus.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_gpio.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_tim.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_dma.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_usart.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_exti.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_system.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_rcc.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_adc.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_spi.h"
#elif defined(STM32F2)
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_bus.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_gpio.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_tim.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_dma.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_usart.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_exti.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_system.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_rcc.h"
  #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_adc.h"
#endif

#if defined(__cplusplus)
}
#endif
