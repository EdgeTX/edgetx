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
#include "stm32_pulse_driver.h"

#include "board.h"

#define HAPTIC_TIMER_FREQ 200000000

static const stm32_pulse_timer_t _haptic_timer = {
  .GPIO = (gpio_t)HAPTIC_GPIO,
  .GPIO_Alternate = HAPTIC_GPIO_AF,
  .TIMx = HAPTIC_GPIO_TIMER,
  .TIM_Freq = HAPTIC_TIMER_FREQ,
  .TIM_Channel = LL_TIM_CHANNEL_CH2,
  .TIM_IRQn = (IRQn_Type)-1,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)-1,
  .DMA_TC_CallbackPtr = nullptr,
};

void hapticInit(void)
{
  stm32_pulse_init(&_haptic_timer, 10000);
  stm32_pulse_config_output(&_haptic_timer, true, LL_TIM_OCMODE_PWM1, 0);
  LL_TIM_SetAutoReload(_haptic_timer.TIMx, 100);
  LL_TIM_EnableCounter(_haptic_timer.TIMx);
}

void hapticOn(uint32_t pwmPercent)
{
  if (pwmPercent > 100) pwmPercent = 100;
  stm32_pulse_set_cmp_val(&_haptic_timer, pwmPercent);
}

void hapticOff(void) { hapticOn(0); }
void hapticDone(void) { hapticOff(); }
