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
}

void stm32_pulse_deinit(const stm32_pulse_timer_t* tim)
{
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

void stm32_pulse_config_output(const stm32_pulse_timer_t* tim, LL_TIM_OC_InitTypeDef* ocInit)
{
  LL_TIM_DisableCounter(tim->TIMx);
  LL_TIM_OC_Init(tim->TIMx, tim->TIM_Channel, ocInit);
  LL_TIM_OC_EnablePreload(tim->TIMx, tim->TIM_Channel);

  if (IS_TIM_BREAK_INSTANCE(tim->TIMx)) {
    LL_TIM_EnableAllOutputs(tim->TIMx);
  }

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_EnableCounter(tim->TIMx);
}

// return true if stopped, false otherwise
bool stm32_pulse_stop_if_running(const stm32_pulse_timer_t* tim)
{
  if (LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream))
    return false;

  // disable timer
  LL_TIM_DisableCounter(tim->TIMx);
  return true;
}

void stm32_pulse_start_dma_req(const stm32_pulse_timer_t* tim,
                               const void* pulses, uint16_t length)
{
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

  // Enable DMA
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);

  // re-init timer
  LL_TIM_GenerateEvent_UPDATE(tim->TIMx);
  LL_TIM_EnableCounter(tim->TIMx);
}
