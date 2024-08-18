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

#include "stm32_timer.h"
#include "stm32_hal.h"

void stm32_timer_enable_clock(TIM_TypeDef *TIMx)
{
  if (((intptr_t)TIMx & 0xFFFF0000) == APB1PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB1PERIPH_BASE) >> 10;
    LL_APB1_GRP1_EnableClock(1U << offset);
  } else if (((intptr_t)TIMx & 0xFFFF0000) == APB2PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB2PERIPH_BASE) >> 10;
    LL_APB2_GRP1_EnableClock(1U << offset);
  }
}

void stm32_timer_disable_clock(TIM_TypeDef *TIMx)
{
  if (((intptr_t)TIMx & 0xFFFF0000) == APB1PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB1PERIPH_BASE) >> 10;
    LL_APB1_GRP1_DisableClock(1U << offset);
  } else if (((intptr_t)TIMx & 0xFFFF0000) == APB2PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB2PERIPH_BASE) >> 10;
    LL_APB2_GRP1_DisableClock(1U << offset);
  }
}

bool stm32_timer_is_clock_enabled(TIM_TypeDef *TIMx)
{
  if (((intptr_t)TIMx & 0xFFFF0000) == APB1PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB1PERIPH_BASE) >> 10;
    return LL_APB1_GRP1_IsEnabledClock(1U << offset);
  } else if (((intptr_t)TIMx & 0xFFFF0000) == APB2PERIPH_BASE) {
    uint32_t offset = ((intptr_t)TIMx - APB2PERIPH_BASE) >> 10;
    return LL_APB2_GRP1_IsEnabledClock(1U << offset);
  }

  // not supported
  return false;
}
