/*
 * Copyright (C) EdgeTx
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

struct stm32_pulse_timer_t {

  GPIO_TypeDef*              GPIOx;
  uint32_t                   GPIO_Pin;
  uint32_t                   GPIO_Alternate;

  TIM_TypeDef*               TIMx;
  uint16_t                   TIM_Prescaler;
  uint32_t                   TIM_Channel;

  DMA_TypeDef*               DMAx;
  uint32_t                   DMA_Stream;
  uint32_t                   DMA_Channel;
};

void stm32_pulse_init(const stm32_pulse_timer_t* tim);
void stm32_pulse_deinit(const stm32_pulse_timer_t* tim);
void stm32_pulse_config_output(const stm32_pulse_timer_t* tim, LL_TIM_OC_InitTypeDef* ocInit);

// return true if stopped, false otherwise
bool stm32_pulse_stop_if_running(const stm32_pulse_timer_t* tim);

void stm32_pulse_start_dma_req(const stm32_pulse_timer_t* tim,
                               const void* pulses, uint16_t length);
