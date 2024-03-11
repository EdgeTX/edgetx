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
#include "hal/gpio.h"

struct stm32_pulse_timer_t;
typedef bool (*stm32_pulse_dma_tc_fct)(void*);

struct stm32_pulse_dma_tc_cb_t {
  stm32_pulse_dma_tc_fct cb;
  void* ctx;
};

struct stm32_pulse_timer_t {

  gpio_t                     GPIO;
  uint32_t                   GPIO_Alternate;

  TIM_TypeDef*               TIMx;
  uint32_t                   TIM_Freq;
  uint32_t                   TIM_Channel;
  IRQn_Type                  TIM_IRQn;

  DMA_TypeDef*               DMAx;
  uint32_t                   DMA_Stream;
  uint32_t                   DMA_Channel;
  IRQn_Type                  DMA_IRQn;
  stm32_pulse_dma_tc_cb_t*   DMA_TC_CallbackPtr;
};

int stm32_pulse_init(const stm32_pulse_timer_t* tim, uint32_t freq);
void stm32_pulse_deinit(const stm32_pulse_timer_t* tim);

void stm32_pulse_config_input(const stm32_pulse_timer_t* tim);

void stm32_pulse_config_output(const stm32_pulse_timer_t* tim, bool polarity,
                               uint32_t ocmode, uint32_t cmp_val);

void stm32_pulse_set_polarity(const stm32_pulse_timer_t* tim, bool polarity);

bool stm32_pulse_get_polarity(const stm32_pulse_timer_t* tim);

void stm32_pulse_set_period(const stm32_pulse_timer_t* tim, uint32_t period);

void stm32_pulse_set_cmp_val(const stm32_pulse_timer_t* tim, uint32_t cmp_val);

void stm32_pulse_start(const stm32_pulse_timer_t* tim);
void stm32_pulse_stop(const stm32_pulse_timer_t* tim);

void stm32_pulse_wait_for_completed(const stm32_pulse_timer_t* tim);

// return true if it could be disabled without interrupting a pulse train, false otherwise
bool stm32_pulse_if_not_running_disable(const stm32_pulse_timer_t* tim);

void stm32_pulse_start_dma_req(const stm32_pulse_timer_t* tim,
                               const void* pulses, uint16_t length,
                               uint32_t ocmode, uint32_t cmp_val);

// Must be called from DMA TC IRQ handler
void stm32_pulse_dma_tc_isr(const stm32_pulse_timer_t* tim);

// Must be called from timer UPDATE IRQ handler
void stm32_pulse_tim_update_isr(const stm32_pulse_timer_t* tim);

#define __STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(ch)       \
  ((ch) == LL_TIM_CHANNEL_CH1 || (ch) == LL_TIM_CHANNEL_CH1N || \
   (ch) == LL_TIM_CHANNEL_CH2 || (ch) == LL_TIM_CHANNEL_CH2N || \
   (ch) == LL_TIM_CHANNEL_CH3 || (ch) == LL_TIM_CHANNEL_CH3N || \
   (ch) == LL_TIM_CHANNEL_CH4)
