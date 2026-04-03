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
 
#include "edgetx_types.h"
#include "board.h"

#include "globals.h"

typedef void (*lcdSpiInitFucPtr)(void);
typedef unsigned int  LcdReadIDFucPtr( void );

extern lcdSpiInitFucPtr lcdOffFunction;
extern lcdSpiInitFucPtr lcdOnFunction;

static const stm32_pulse_timer_t _bl_timer = {
  .GPIO = (gpio_t)BACKLIGHT_GPIO,
  .GPIO_Alternate = BACKLIGHT_GPIO_AF,
  .TIMx = BACKLIGHT_TIMER,
  .TIM_Freq = BACKLIGHT_TIMER_FREQ,
  .TIM_Channel = BACKLIGHT_TIMER_CHANNEL,
  .TIM_IRQn = (IRQn_Type)-1,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)-1,
  .DMA_TC_CallbackPtr = nullptr,
};

void backlightLowInit()
{
  gpio_init(BACKLIGHT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(BACKLIGHT_GPIO);
}

void backlightInit()
{
  stm32_pulse_init(&_bl_timer, 100000);
  stm32_pulse_config_output(&_bl_timer, true, LL_TIM_OCMODE_PWM1, 100);
  LL_TIM_SetAutoReload(_bl_timer.TIMx, 100);
  LL_TIM_EnableCounter(_bl_timer.TIMx);
}

uint8_t lastDutyCycle = 0;

void backlightEnable(uint8_t dutyCycle)
{
  stm32_pulse_set_cmp_val(&_bl_timer, dutyCycle);
  lastDutyCycle = dutyCycle;
}

void lcdOff() {
  backlightEnable(0);
}

void lcdOn(){
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

bool boardBacklightOn;

bool isBacklightEnabled()
{
  return boardBacklightOn;
}
