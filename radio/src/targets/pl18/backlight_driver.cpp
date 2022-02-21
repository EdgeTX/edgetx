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

#include "opentx_types.h"
#include "board.h"

void backlightInit()
{
  // PIN init
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource, BACKLIGHT_GPIO_AF);

  // TODO review this when the timer will be chosen
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

void lcdOn(){
  if(lcdOnFunction) lcdOnFunction();
  else lcdInit();
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

bool isBacklightEnabled() {
  return lastDutyCycle != 0;
}
