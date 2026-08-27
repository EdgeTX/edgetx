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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_timer.h"

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
  gpio_init_af(HAPTIC_GPIO, HAPTIC_GPIO_AF, GPIO_PIN_SPEED_LOW);

  stm32_timer_enable_clock(HAPTIC_GPIO_TIMER);
  HAPTIC_GPIO_TIMER->ARR = 100;
  HAPTIC_GPIO_TIMER->PSC = (PERI2_FREQUENCY * TIMER_MULT_APB2) / 10000 - 1;
  HAPTIC_GPIO_TIMER->CCMR1 = HAPTIC_TIMER_MODE; // PWM
  HAPTIC_GPIO_TIMER->CCER = HAPTIC_TIMER_OUTPUT_ENABLE;
  
  hapticOff();
  
  HAPTIC_GPIO_TIMER->EGR = TIM_EGR_UG;
  HAPTIC_GPIO_TIMER->CR1 = TIM_CR1_CEN; // counter enable
  HAPTIC_GPIO_TIMER->BDTR |= TIM_BDTR_MOE;
}

void hapticDone(void)
{
  hapticOff();
}
