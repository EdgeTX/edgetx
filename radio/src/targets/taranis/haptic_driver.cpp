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

#if defined(HAPTIC_PWM)
// TODO test Haptic on X7 (I added PWM support)
void hapticOff()
{
  HAPTIC_COUNTER_REGISTER = 0;
}

void hapticOn(uint32_t pwmPercent)
{
  if (pwmPercent > 100) {
    pwmPercent = 100;
  }
  HAPTIC_COUNTER_REGISTER = pwmPercent;
}

void hapticInit()
{
  gpio_init_af(HAPTIC_GPIO, HAPTIC_GPIO_AF, GPIO_PIN_SPEED_LOW);

  stm32_timer_enable_clock(HAPTIC_TIMER);
  HAPTIC_TIMER->ARR = 100;
  HAPTIC_TIMER->PSC = HAPTIC_TIMER_FREQ / 10000 - 1;
#if defined(HAPTIC_CCMR1)
  HAPTIC_TIMER->CCMR1 = HAPTIC_CCMR1; // PWM
#elif defined(HAPTIC_CCMR2)
  HAPTIC_TIMER->CCMR2 = HAPTIC_CCMR2; // PWM
#endif
  HAPTIC_TIMER->CCER = HAPTIC_CCER;
  HAPTIC_TIMER->BDTR = TIM_BDTR_MOE; // Enable output
  HAPTIC_COUNTER_REGISTER = 0;
  HAPTIC_TIMER->EGR = 0;
  HAPTIC_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
}

#else

// No PWM before X9D+
void hapticInit()
{
  gpio_init(HAPTIC_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

void hapticOff()
{
  gpio_clear(HAPTIC_GPIO);
}

void hapticOn()
{
  gpio_set(HAPTIC_GPIO);
}

#endif

