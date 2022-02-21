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

void hapticOff(void)
{
  HAPTIC_TIMER_COMPARE_VALUE = 0;
}

void hapticOn(uint32_t pwmPercent)
{
  if (pwmPercent > 100) {
    pwmPercent = 100;
  }
  HAPTIC_TIMER_COMPARE_VALUE = pwmPercent;
}

void hapticInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = HAPTIC_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(HAPTIC_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(HAPTIC_GPIO, HAPTIC_GPIO_PinSource, HAPTIC_GPIO_AF);

  HAPTIC_GPIO_TIMER->ARR = 100;
  HAPTIC_GPIO_TIMER->PSC = (PERI2_FREQUENCY * TIMER_MULT_APB2) / 10000 - 1;
  HAPTIC_GPIO_TIMER->CCMR1 = HAPTIC_TIMER_MODE; // PWM
  HAPTIC_GPIO_TIMER->CCER = HAPTIC_TIMER_OUTPUT_ENABLE;
  HAPTIC_GPIO_TIMER->CCR1 = 0;
  HAPTIC_GPIO_TIMER->EGR = TIM_EGR_UG;
  HAPTIC_GPIO_TIMER->CR1 = TIM_CR1_CEN; // counter enable
  HAPTIC_GPIO_TIMER->BDTR |= TIM_BDTR_MOE;
}

void hapticDone(void)
{
  hapticOff();
  RCC_AHB1PeriphClockCmd(HAPTIC_RCC_AHB1Periph, DISABLE);
}
