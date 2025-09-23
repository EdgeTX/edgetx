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

#if !defined(BACKLIGHT_GPIO) && (!defined(BACKLIGHT_GPIO_1) || !defined(BACKLIGHT_GPIO_2))
  // no backlight
  void backlightInit() {}
  void backlightEnable(uint8_t level) {}
  void backlightFullOn() {}
  void backlightDisable() {}
  bool isBacklightEnabled() { return false; }
#else
void backlightInit()
{
  gpio_init_af(BACKLIGHT_GPIO, BACKLIGHT_GPIO_AF, GPIO_PIN_SPEED_LOW);
  stm32_timer_enable_clock(BACKLIGHT_TIMER);  
  
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  BACKLIGHT_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC1E;
  BACKLIGHT_TIMER->CCR1 = 80;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = 1;
}

void backlightEnable(uint8_t level)
{
  BACKLIGHT_TIMER->CCR1 = 100 - level;
}

void backlightFullOn()
{
  backlightEnable(0);
}

void backlightDisable()
{
  BACKLIGHT_TIMER->CCR1 = 0;
}

bool isBacklightEnabled()
{
  return BACKLIGHT_TIMER->CCR1 != 0;
}
#endif
