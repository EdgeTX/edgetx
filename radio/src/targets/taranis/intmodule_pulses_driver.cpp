/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include "pulses/pulses.h"

void intmoduleStop()
{
  INTERNAL_MODULE_OFF();

  NVIC_DisableIRQ(INTMODULE_DMA_STREAM_IRQn);
  NVIC_DisableIRQ(INTMODULE_TIMER_CC_IRQn);

  INTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
  INTMODULE_TIMER->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_UDE);
  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
}

// #define HEARBEAT_OFFSET unsigned(6000 + g_model.flightModeData[0].gvars[0] * 100)
constexpr unsigned HEARBEAT_OFFSET = 6000;

void intmoduleSendNextFrame()
{
  switch (moduleState[INTERNAL_MODULE].protocol) {
#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
    {
      if (INTMODULE_DMA_STREAM->CR & DMA_SxCR_EN)
        return;

      //disable timer
      INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
      
      //INTMODULE_TIMER->CCR2 = last - 4000; // 2mS in advance
      INTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
      INTMODULE_DMA_STREAM->CR |= INTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
      INTMODULE_DMA_STREAM->PAR = CONVERT_PTR_UINT(&INTMODULE_TIMER->ARR);
      INTMODULE_DMA_STREAM->M0AR = CONVERT_PTR_UINT(intmodulePulsesData.pxx.getData());
      INTMODULE_DMA_STREAM->NDTR = intmodulePulsesData.pxx.getSize();
      INTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA

      // re-init timer
      INTMODULE_TIMER->EGR = 1;
      INTMODULE_TIMER->CR1 |= TIM_CR1_CEN;
      break;
    }
#endif

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      INTMODULE_TIMER->CCR3 = GET_MODULE_PPM_DELAY(INTERNAL_MODULE) * 2;
      INTMODULE_TIMER->CCER = TIM_CCER_CC3E | (GET_MODULE_PPM_POLARITY(INTERNAL_MODULE) ? 0 : TIM_CCER_CC3P);
      INTMODULE_TIMER->CCR2 = *(intmodulePulsesData.ppm.ptr - 1) - 4000; // 2mS in advance
      INTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
      INTMODULE_DMA_STREAM->CR |= INTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
      INTMODULE_DMA_STREAM->PAR = CONVERT_PTR_UINT(&INTMODULE_TIMER->ARR);
      INTMODULE_DMA_STREAM->M0AR = CONVERT_PTR_UINT(intmodulePulsesData.ppm.pulses);
      INTMODULE_DMA_STREAM->NDTR = intmodulePulsesData.ppm.ptr - intmodulePulsesData.ppm.pulses;
      INTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA
      break;
#endif

    default:
      INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE;
      break;
  }
}

void intmodulePxx1PulsesStart()
{
  INTERNAL_MODULE_ON();

  GPIO_PinAFConfig(INTMODULE_TX_GPIO, INTMODULE_TX_GPIO_PinSource, INTMODULE_TX_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(INTMODULE_TX_GPIO, &GPIO_InitStructure);

  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  INTMODULE_TIMER->PSC = INTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

  INTMODULE_TIMER->CCER = TIM_CCER_CC3E | TIM_CCER_CC3NE;
  INTMODULE_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  INTMODULE_TIMER->CCR3 = 16;
  INTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0; // Force O/P high
  INTMODULE_TIMER->EGR = 1; // Restart
  INTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;
  INTMODULE_TIMER->ARR = 40000;
  INTMODULE_TIMER->SR &= ~TIM_SR_CC2IF; // Clear flag
  INTMODULE_TIMER->DIER |= TIM_DIER_UDE; // Enable DMA on update
  INTMODULE_TIMER->CR1 |= TIM_CR1_CEN;

  NVIC_EnableIRQ(INTMODULE_DMA_STREAM_IRQn);
  NVIC_SetPriority(INTMODULE_DMA_STREAM_IRQn, 7);
  NVIC_EnableIRQ(INTMODULE_TIMER_CC_IRQn);
  NVIC_SetPriority(INTMODULE_TIMER_CC_IRQn, 7);
}

#if defined(INTERNAL_MODULE_PPM)
void intmodulePpmStart()
{
  INTERNAL_MODULE_ON();

  GPIO_PinAFConfig(INTMODULE_TX_GPIO, INTMODULE_TX_GPIO_PinSource, INTMODULE_TX_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(INTMODULE_TX_GPIO, &GPIO_InitStructure);

  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  INTMODULE_TIMER->PSC = INTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS from 30MHz
  INTMODULE_TIMER->ARR = 45000;
  INTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1
  INTMODULE_TIMER->BDTR = TIM_BDTR_MOE;
  INTMODULE_TIMER->EGR = 1;
  INTMODULE_TIMER->DIER = TIM_DIER_UDE;
  INTMODULE_TIMER->SR &= ~TIM_SR_CC2IF; // Clear flag
  INTMODULE_TIMER->CCR2 = 40000; // The first frame will be sent in 20ms
  INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE; // Enable this interrupt
  INTMODULE_TIMER->CR1 = TIM_CR1_CEN;

  NVIC_EnableIRQ(INTMODULE_DMA_STREAM_IRQn);
  NVIC_SetPriority(INTMODULE_DMA_STREAM_IRQn, 7);
  NVIC_EnableIRQ(INTMODULE_TIMER_CC_IRQn);
  NVIC_SetPriority(INTMODULE_TIMER_CC_IRQn, 7);
}
#endif // defined(INTERNAL_MODULE_PPM)

extern "C" void INTMODULE_DMA_STREAM_IRQHandler()
{
  if (!DMA_GetITStatus(INTMODULE_DMA_STREAM, INTMODULE_DMA_FLAG_TC))
    return;

  DMA_ClearITPendingBit(INTMODULE_DMA_STREAM, INTMODULE_DMA_FLAG_TC);

  switch (moduleState[INTERNAL_MODULE].protocol) {
    case PROTOCOL_CHANNELS_PPM:
      INTMODULE_TIMER->SR &= ~TIM_SR_CC2IF; // Clear flag
      INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE; // Enable this interrupt
      break;
  }
}

extern "C" void INTMODULE_TIMER_CC_IRQHandler()
{
  INTMODULE_TIMER->DIER &= ~TIM_DIER_CC2IE; // Stop this interrupt
  INTMODULE_TIMER->SR &= ~TIM_SR_CC2IF;
  if (setupPulsesInternalModule()) {
    intmoduleSendNextFrame();
  }
}
