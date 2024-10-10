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

#pragma once

#include <stdint.h>
#include "stm32_hal_ll.h"

struct stick_pwm_input_t {
  uint8_t channel;
  uint8_t inverted;
};

struct stick_pwm_timer_t {

  GPIO_TypeDef*              GPIOx;
  uint32_t                   GPIO_Pin;
  uint32_t                   GPIO_Alternate;

  TIM_TypeDef*               TIMx;
  IRQn_Type                  TIM_IRQn;
};

// returns 'true' if PWM sticks have been detected
bool sticks_pwm_detect(const stick_pwm_timer_t* timer,
		       const stick_pwm_input_t* inputs,
		       uint8_t n_inputs);

void sticks_pwm_isr(const stick_pwm_timer_t* tim,
		    const stick_pwm_input_t* inputs,
		    uint8_t n_inputs);
