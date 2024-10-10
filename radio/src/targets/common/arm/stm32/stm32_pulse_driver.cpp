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

#include "stm32_pulse_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_timer.h"
#include "stm32_dma.h"
#include "stm32_gpio.h"

#include "definitions.h"

#include <string.h>

  // 0.5uS (2Mhz)
#define STM32_DEFAULT_TIMER_FREQ       2000000
#define STM32_DEFAULT_TIMER_AUTORELOAD   65535

// TODO:
// - DMA IRQ prio configurable (now 7)
// - Timer IRQ prio configurable (now 7)

static void init_dma_arr_mode(const stm32_pulse_timer_t* tim)
{
  // re-init DMA stream
  LL_DMA_DeInit(tim->DMAx, tim->DMA_Stream);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

  // Direction
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;

  // Source
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;

  // Destination
  dmaInit.PeriphOrM2MSrcAddress = CONVERT_PTR_UINT(&tim->TIMx->ARR);
  dmaInit.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;

  // Data width
  if (IS_TIM_32B_COUNTER_INSTANCE(tim->TIMx)) {
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  } else {
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  }

#if defined(STM32H7)
  dmaInit.PeriphRequest = tim->DMA_Channel;
#else
  dmaInit.Channel = tim->DMA_Channel;
#endif
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;

  stm32_dma_enable_clock(tim->DMAx);
  LL_DMA_Init(tim->DMAx, tim->DMA_Stream, &dmaInit);
}

int stm32_pulse_init(const stm32_pulse_timer_t* tim, uint32_t freq)
{
  // check if timer is free first (clock disabled)
  if (stm32_timer_is_clock_enabled(tim->TIMx)) return -1;

  // .. and GPIO pin is not used
  if(gpio_get_mode(tim->GPIO) != GPIO_IN) return -1;

  if (tim->DMA_TC_CallbackPtr) {
    memset(tim->DMA_TC_CallbackPtr, 0, sizeof(stm32_pulse_dma_tc_cb_t));
  }
  
  gpio_init_af(tim->GPIO, tim->GPIO_Alternate, GPIO_PIN_SPEED_MEDIUM);

  LL_TIM_InitTypeDef timInit;
  LL_TIM_StructInit(&timInit);

  if (!freq) {
    freq = STM32_DEFAULT_TIMER_FREQ;
  }
  
  timInit.Prescaler = __LL_TIM_CALC_PSC(tim->TIM_Freq, freq);
  timInit.Autoreload = STM32_DEFAULT_TIMER_AUTORELOAD;

  stm32_timer_enable_clock(tim->TIMx);
  LL_TIM_Init(tim->TIMx, &timInit);

  if (tim->DMAx && (int32_t)tim->DMA_IRQn >= 0) {
    init_dma_arr_mode(tim);
    NVIC_EnableIRQ(tim->DMA_IRQn);
    NVIC_SetPriority(tim->DMA_IRQn, 7);
  }

  if ((int32_t)tim->TIM_IRQn >= 0) {
    NVIC_EnableIRQ(tim->TIM_IRQn);
    NVIC_SetPriority(tim->TIM_IRQn, 7);
  }

  return 0;
}

void stm32_pulse_deinit(const stm32_pulse_timer_t* tim)
{
  // Disable IRQs
  if ((int32_t)tim->DMA_IRQn >= 0) {
    NVIC_DisableIRQ(tim->DMA_IRQn);
  }

  if ((int32_t)tim->TIM_IRQn >= 0) {
    NVIC_DisableIRQ(tim->TIM_IRQn);
  }

  if (tim->DMAx) {
    LL_DMA_DeInit(tim->DMAx, tim->DMA_Stream);
  }

  if (tim->DMA_TC_CallbackPtr) {
    memset(tim->DMA_TC_CallbackPtr, 0, sizeof(stm32_pulse_dma_tc_cb_t));
  }

  LL_TIM_DeInit(tim->TIMx);
  stm32_timer_disable_clock(tim->TIMx);

  gpio_init(tim->GPIO, GPIO_IN, GPIO_PIN_SPEED_MEDIUM);
}

static inline bool _is_complementary_channel(uint32_t channel)
{
  return (channel == LL_TIM_CHANNEL_CH1N) || (channel == LL_TIM_CHANNEL_CH2N) ||
         (channel == LL_TIM_CHANNEL_CH3N);
}

static const uint32_t _base_channel[] = {
  LL_TIM_CHANNEL_CH1, LL_TIM_CHANNEL_CH1,
  LL_TIM_CHANNEL_CH2, LL_TIM_CHANNEL_CH2,
  LL_TIM_CHANNEL_CH3, LL_TIM_CHANNEL_CH3,
  LL_TIM_CHANNEL_CH4
};

static inline uint32_t _get_base_channel(uint32_t channel)
{
  auto idx = TIM_GET_CHANNEL_INDEX(channel);
  return _base_channel[idx];
}

void stm32_pulse_config_output(const stm32_pulse_timer_t* tim, bool polarity,
                               uint32_t ocmode, uint32_t cmp_val)
{
  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = ocmode;
  ocInit.CompareValue = cmp_val;

  uint32_t channel = _get_base_channel(tim->TIM_Channel);
  bool comp_ch = _is_complementary_channel(tim->TIM_Channel);
  if (!comp_ch) {
    ocInit.OCState = LL_TIM_OCSTATE_ENABLE;
    ocInit.OCNState = LL_TIM_OCSTATE_DISABLE;
  } else {
    ocInit.OCState = LL_TIM_OCSTATE_DISABLE;
    ocInit.OCNState = LL_TIM_OCSTATE_ENABLE;
  }

  uint32_t ll_polarity;
  if (polarity) {
    ll_polarity = LL_TIM_OCPOLARITY_HIGH;
  } else {
    ll_polarity = LL_TIM_OCPOLARITY_LOW;
  }

  if (!comp_ch) {
    ocInit.OCPolarity = ll_polarity;
  } else {
    ocInit.OCNPolarity = ll_polarity;
  }
  
  LL_TIM_OC_Init(tim->TIMx, channel, &ocInit);
  LL_TIM_OC_EnablePreload(tim->TIMx, channel);

  if (IS_TIM_BREAK_INSTANCE(tim->TIMx)) {
    LL_TIM_EnableAllOutputs(tim->TIMx);
  }
}

void stm32_pulse_set_polarity(const stm32_pulse_timer_t* tim, bool polarity)
{
  uint32_t ll_polarity;
  if (polarity) {
    ll_polarity = LL_TIM_OCPOLARITY_HIGH;
  } else {
    ll_polarity = LL_TIM_OCPOLARITY_LOW;
  }
  LL_TIM_OC_SetPolarity(tim->TIMx, tim->TIM_Channel, ll_polarity);  
}

bool stm32_pulse_get_polarity(const stm32_pulse_timer_t* tim)
{
  return LL_TIM_OC_GetPolarity(tim->TIMx, tim->TIM_Channel) ==
         LL_TIM_OCPOLARITY_HIGH;
}

// return true if stopped, false otherwise
bool stm32_pulse_if_not_running_disable(const stm32_pulse_timer_t* tim)
{
  if (LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream))
    return false;

  // disable timer
  LL_TIM_DisableCounter(tim->TIMx);
  LL_TIM_DisableIT_UPDATE(tim->TIMx);

  return true;
}

static void set_compare_reg(const stm32_pulse_timer_t* tim, uint32_t val)
{
  switch(_get_base_channel(tim->TIM_Channel)){
  case LL_TIM_CHANNEL_CH1:
    LL_TIM_OC_SetCompareCH1(tim->TIMx, val);
    break;
  case LL_TIM_CHANNEL_CH2:
    LL_TIM_OC_SetCompareCH2(tim->TIMx, val);
    break;
  case LL_TIM_CHANNEL_CH3:
    LL_TIM_OC_SetCompareCH3(tim->TIMx, val);
    break;
  case LL_TIM_CHANNEL_CH4:
    LL_TIM_OC_SetCompareCH4(tim->TIMx, val);
    break;
  }
}

void stm32_pulse_set_period(const stm32_pulse_timer_t* tim, uint32_t period)
{
  LL_TIM_SetAutoReload(tim->TIMx, period - 1);
}

void stm32_pulse_set_cmp_val(const stm32_pulse_timer_t* tim, uint32_t cmp_val)
{
  set_compare_reg(tim, cmp_val);
}

void stm32_pulse_start(const stm32_pulse_timer_t* tim)
{
  LL_TIM_EnableCounter(tim->TIMx);
}

void stm32_pulse_stop(const stm32_pulse_timer_t* tim)
{
  LL_TIM_DisableCounter(tim->TIMx);
}

static void set_oc_mode(const stm32_pulse_timer_t* tim, uint32_t ocmode)
{
  uint32_t channel = _get_base_channel(tim->TIM_Channel);
  LL_TIM_OC_SetMode(tim->TIMx, channel, ocmode);
}

void stm32_pulse_wait_for_completed(const stm32_pulse_timer_t* tim)
{
  uint32_t channel = _get_base_channel(tim->TIM_Channel);
  while(LL_TIM_IsEnabledCounter(tim->TIMx) &&
        LL_TIM_OC_GetMode(tim->TIMx, channel) != LL_TIM_OCMODE_FORCED_INACTIVE);
}


void stm32_pulse_start_dma_req(const stm32_pulse_timer_t* tim,
                               const void* pulses, uint16_t length,
                               uint32_t ocmode, uint32_t cmp_val)
{
  // Re-configure timer output
  set_compare_reg(tim, cmp_val);
  set_oc_mode(tim, ocmode);

  LL_DMA_SetDataLength(tim->DMAx, tim->DMA_Stream, length);
  LL_DMA_SetMemoryAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)pulses);

  // Enable TC IRQ
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);

  if (ocmode == LL_TIM_OCMODE_PWM1) {
    // preloads first period for PWM)
    LL_TIM_SetCounter(tim->TIMx, 0x00);
    LL_TIM_GenerateEvent_UPDATE(tim->TIMx);
  } else {
    // Reset counter close to overflow
    if (IS_TIM_32B_COUNTER_INSTANCE(tim->TIMx)) {
      LL_TIM_SetCounter(tim->TIMx, 0xFFFFFFFF);
    } else {
      LL_TIM_SetCounter(tim->TIMx, 0xFFFF);
    }
  }

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);

  // start timer
  LL_TIM_EnableCounter(tim->TIMx);
}

void stm32_pulse_dma_tc_isr(const stm32_pulse_timer_t* tim)
{
  if (!stm32_dma_check_tc_flag(tim->DMAx, tim->DMA_Stream))
    return;

  if (tim->DMA_TC_CallbackPtr) {
    auto closure = tim->DMA_TC_CallbackPtr;
    if (closure->cb && closure->cb(closure->ctx)) {
      return;
    }
  }
  
  LL_TIM_ClearFlag_UPDATE(tim->TIMx);
  LL_TIM_EnableIT_UPDATE(tim->TIMx);

  // PWM mode with compare value = 0 then OCxRef is held at ‘0’
  set_compare_reg(tim, 0);
  set_oc_mode(tim, LL_TIM_OCMODE_PWM1);
}

void stm32_pulse_tim_update_isr(const stm32_pulse_timer_t* tim)
{
  if (!LL_TIM_IsActiveFlag_UPDATE(tim->TIMx))
    return;

  LL_TIM_ClearFlag_UPDATE(tim->TIMx);
  LL_TIM_DisableIT_UPDATE(tim->TIMx);

  // Halt pulses by forcing to inactive level
  set_oc_mode(tim, LL_TIM_OCMODE_FORCED_INACTIVE);
  LL_TIM_DisableCounter(tim->TIMx);
}

// input mode
void stm32_pulse_config_input(const stm32_pulse_timer_t* tim)
{
  LL_TIM_IC_InitTypeDef icInit;
  LL_TIM_IC_StructInit(&icInit);
  icInit.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  icInit.ICFilter = LL_TIM_IC_FILTER_FDIV1_N8;
  LL_TIM_IC_Init(tim->TIMx, tim->TIM_Channel, &icInit);
}
