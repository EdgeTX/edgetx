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
#include "definitions.h"

void stm32_pulse_init(const stm32_pulse_timer_t* tim)
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = tim->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Alternate = tim->GPIO_Alternate;
  LL_GPIO_Init(tim->GPIOx, &pinInit);

  LL_TIM_InitTypeDef timInit;
  LL_TIM_StructInit(&timInit);

  // 0.5uS (2Mhz)
  timInit.Prescaler = tim->TIM_Prescaler;
  timInit.Autoreload = 65535;
  LL_TIM_Init(tim->TIMx, &timInit);

  // Enable DMA IRQ
  NVIC_EnableIRQ(tim->DMA_IRQn);
  NVIC_SetPriority(tim->DMA_IRQn, 7);

  // Enable timer IRQ
  NVIC_EnableIRQ(tim->TIM_IRQn);
  NVIC_SetPriority(tim->TIM_IRQn, 7);
}

void stm32_pulse_deinit(const stm32_pulse_timer_t* tim)
{
  // Disable IRQs
  NVIC_DisableIRQ(tim->DMA_IRQn);
  NVIC_DisableIRQ(tim->TIM_IRQn);
  
  // De-init DMA & timer
  LL_DMA_DeInit(tim->DMAx, tim->DMA_Stream);
  LL_TIM_DeInit(tim->TIMx);

  // Reconfigure pin as output
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = tim->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  LL_GPIO_Init(tim->GPIOx, &pinInit);
}

void stm32_pulse_config_output(const stm32_pulse_timer_t* tim, bool polarity,
                               uint32_t ocmode, uint32_t cmp_val)
{
  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = ocmode;
  ocInit.CompareValue = cmp_val;

  if (tim->TIM_Channel != LL_TIM_CHANNEL_CH1N) {
    ocInit.OCState = LL_TIM_OCSTATE_ENABLE;
  } else {
    ocInit.OCNState = LL_TIM_OCSTATE_ENABLE;
    polarity = !polarity;
  }

  uint32_t ll_polarity;
  if (polarity) {
    ll_polarity = LL_TIM_OCPOLARITY_HIGH;
  } else {
    ll_polarity = LL_TIM_OCPOLARITY_LOW;
  }

  if (tim->TIM_Channel != LL_TIM_CHANNEL_CH1N) {
    ocInit.OCPolarity = ll_polarity;
  } else {
    ocInit.OCNPolarity = ll_polarity;
  }
  
  LL_TIM_OC_Init(tim->TIMx, tim->TIM_Channel, &ocInit);
  LL_TIM_OC_EnablePreload(tim->TIMx, tim->TIM_Channel);

  if (IS_TIM_BREAK_INSTANCE(tim->TIMx)) {
    LL_TIM_EnableAllOutputs(tim->TIMx);
  }

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
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
  switch(tim->TIM_Channel){
  case LL_TIM_CHANNEL_CH1:
  case LL_TIM_CHANNEL_CH1N:
    LL_TIM_OC_SetCompareCH1(tim->TIMx, val);
    break;
  case LL_TIM_CHANNEL_CH3:
    LL_TIM_OC_SetCompareCH3(tim->TIMx, val);
    break;
  }
}

void stm32_pulse_start_dma_req(const stm32_pulse_timer_t* tim,
                               const void* pulses, uint16_t length,
                               uint32_t ocmode, uint32_t cmp_val)
{
  // re-init DMA stream
  LL_DMA_DeInit(tim->DMAx, tim->DMA_Stream);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

  // Direction
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  
  // Source
  dmaInit.MemoryOrM2MDstAddress = CONVERT_PTR_UINT(pulses);
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;

  // Destination
  dmaInit.PeriphOrM2MSrcAddress = CONVERT_PTR_UINT(&tim->TIMx->ARR);
  dmaInit.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;

  // Data width
  if (IS_TIM_32B_COUNTER_INSTANCE(tim->TIMx)) {
    // TODO: try using 16-bit source as well
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  } else {
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  }

  dmaInit.NbData = length;
  dmaInit.Channel = tim->DMA_Channel;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;

  LL_DMA_Init(tim->DMAx, tim->DMA_Stream, &dmaInit);

  // Enable TC IRQ
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);

  // Enable DMA
  LL_TIM_ClearFlag_UPDATE(tim->TIMx);
  LL_TIM_DisableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_SetCounter(tim->TIMx, 0);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);

  // Re-configure timer output
  set_compare_reg(tim, cmp_val);
  LL_TIM_OC_SetMode(tim->TIMx, tim->TIM_Channel, ocmode);
  
  // Trigger update to effect the first DMA transation
  // and thus load ARR with the first duration
  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_GenerateEvent_UPDATE(tim->TIMx);

  // start timer
  LL_TIM_EnableCounter(tim->TIMx);
}

static bool check_and_clean_dma_tc_flag(const stm32_pulse_timer_t* tim)
{
  switch(tim->DMA_Stream) {
  case LL_DMA_STREAM_1:
    if (!LL_DMA_IsActiveFlag_TC1(tim->DMAx)) return false;
    LL_DMA_ClearFlag_TC1(tim->DMAx);
    break;
  case LL_DMA_STREAM_5:
    if (!LL_DMA_IsActiveFlag_TC5(tim->DMAx)) return false;
    LL_DMA_ClearFlag_TC5(tim->DMAx);
    break;
  case LL_DMA_STREAM_7:
    if (!LL_DMA_IsActiveFlag_TC7(tim->DMAx)) return false;
    LL_DMA_ClearFlag_TC7(tim->DMAx);
    break;
  }
  return true;
}

void stm32_pulse_dma_tc_isr(const stm32_pulse_timer_t* tim)
{
  if (!check_and_clean_dma_tc_flag(tim)) return;

  LL_TIM_ClearFlag_UPDATE(tim->TIMx);
  LL_TIM_EnableIT_UPDATE(tim->TIMx);

  set_compare_reg(tim, 0);
  LL_TIM_OC_SetMode(tim->TIMx, tim->TIM_Channel, LL_TIM_OCMODE_PWM1);
}

void stm32_pulse_tim_update_isr(const stm32_pulse_timer_t* tim)
{
  if (!LL_TIM_IsActiveFlag_UPDATE(tim->TIMx))
    return;

  LL_TIM_ClearFlag_UPDATE(tim->TIMx);
  LL_TIM_DisableIT_UPDATE(tim->TIMx);

  // Halt pulses by forcing to inactive level
  LL_TIM_OC_SetMode(tim->TIMx, tim->TIM_Channel, LL_TIM_OCMODE_FORCED_INACTIVE);
}
