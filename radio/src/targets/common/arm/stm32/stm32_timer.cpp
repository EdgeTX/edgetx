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

void stm32_timer_enable_clock(TIM_TypeDef *TIMx)
{
  if (TIMx == TIM1) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  } else if (TIMx == TIM2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  } else if (TIMx == TIM3) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  } else if (TIMx == TIM4) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
  } else if (TIMx == TIM5) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
  } else if (TIMx == TIM8) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);
  } else if (TIMx == TIM12) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);
  } else if (TIMx == TIM13) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM13);
  } else if (TIMx == TIM14) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
  }  
}

void stm32_timer_disable_clock(TIM_TypeDef *TIMx)
{
  if (TIMx == TIM1) {
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
  } else if (TIMx == TIM2) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
  } else if (TIMx == TIM3) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM3);
  } else if (TIMx == TIM4) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM4);
  } else if (TIMx == TIM5) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM5);
  } else if (TIMx == TIM8) {
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM8);
  } else if (TIMx == TIM12) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM12);
  } else if (TIMx == TIM13) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM13);
  } else if (TIMx == TIM14) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM14);
  }  
}

bool stm32_timer_is_clock_enabled(TIM_TypeDef *TIMx)
{
  if (TIMx == TIM1) {
    return LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_TIM1) != 0;
  } else if (TIMx == TIM2) {
    return LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM2) != 0;
  } else if (TIMx == TIM3) {
    return LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM3) != 0;
  } else if (TIMx == TIM4) {
    return LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM4) != 0;
  } else if (TIMx == TIM5) {
    return LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM5) != 0;
  } else if (TIMx == TIM8) {
    return LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_TIM8) != 0;
  } else if (TIMx == TIM12) {
    return LL_APB2_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM12) != 0;
  } else if (TIMx == TIM13) {
    return LL_APB2_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM13) != 0;
  } else if (TIMx == TIM14) {
    return LL_APB2_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM14) != 0;
  }

  // not supported
  return false;
}
