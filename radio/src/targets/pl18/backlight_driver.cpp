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

#include "edgetx_types.h"
#include "board.h"

#include "globals.h"
#include "lcd_driver.h"

void backlightLowInit( void )
{
  gpio_init(BACKLIGHT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(BACKLIGHT_GPIO);
}

void backlightInit()
{
  // PIN init
  gpio_init_af(BACKLIGHT_GPIO, BACKLIGHT_GPIO_AF, GPIO_PIN_SPEED_LOW);

  // TODO review this when the timer will be chosen
  stm32_timer_enable_clock(BACKLIGHT_TIMER);
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 1000000 - 1; // 10kHz (same as FrOS)
  BACKLIGHT_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE; // PWM mode 1
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC1NE;
  BACKLIGHT_TIMER->CCR1 = 100; // 100% on init
  BACKLIGHT_TIMER->EGR = TIM_EGR_UG;
  BACKLIGHT_TIMER->CR1 |= TIM_CR1_CEN; // Counter enable
  BACKLIGHT_TIMER->BDTR |= TIM_BDTR_MOE;
}

uint8_t lastDutyCycle = 0;

void backlightEnable(uint8_t dutyCycle)
{
  BACKLIGHT_TIMER->CCR1 = dutyCycle;
  if(!dutyCycle) {
    //experimental to turn off LCD when no backlight
    if(lcdOffFunction) lcdOffFunction();
  }
  else if(!lastDutyCycle) {
    if(lcdOnFunction) lcdOnFunction();
    else lcdInit();
  }
  lastDutyCycle = dutyCycle;
}

void lcdOff() {
  backlightEnable(0);
}

void lcdOn() {
  if(lcdOnFunction) lcdOnFunction();
  else lcdInit();
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

bool boardBacklightOn;

bool isBacklightEnabled()
{
  return boardBacklightOn;
}
