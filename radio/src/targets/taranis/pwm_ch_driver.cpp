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

#include "stm32_pulse_driver.h"
#include "stm32_gpio.h"

#include "edgetx_types.h"
#include "board.h"

static const stm32_pulse_timer_t _pwm_timer = {
  .GPIO = (gpio_t)PWM_GPIO,
  .GPIO_Alternate = PWM_GPIO_AF,
  .TIMx = PWM_GPIO_TIMER,
  .TIM_Freq = PWM_TIMER_FREQ,
  .TIM_Channel = LL_TIM_CHANNEL_CH3,
  .TIM_IRQn = (IRQn_Type)-1,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)-1,
  .DMA_TC_CallbackPtr = nullptr,
};

void pwmOutputDisable()
{
  stm32_pulse_stop(&_pwm_timer);
}

void pwmOutputInit()
{
  stm32_pulse_init(&_pwm_timer, 2000000);
  stm32_pulse_config_output(&_pwm_timer, true, LL_TIM_OCMODE_PWM1, 0);
  LL_TIM_SetAutoReload(_pwm_timer.TIMx, 15000); // 15ms between PWM pulses
  LL_TIM_EnableCounter(_pwm_timer.TIMx);
}

void pwmOutputEnable(uint16_t pulse_width_us)
{
  stm32_pulse_set_cmp_val(&_pwm_timer, pulse_width_us);
  stm32_pulse_start(&_pwm_timer);
}
