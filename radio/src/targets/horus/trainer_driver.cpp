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

void trainerSendNextFrame();

void init_trainer_ppm()
{
  GPIO_PinAFConfig(TRAINER_GPIO, TRAINER_OUT_GPIO_PinSource, TRAINER_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TRAINER_OUT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_GPIO, &GPIO_InitStructure);

  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN;
  TRAINER_TIMER->PSC = TRAINER_TIMER_FREQ / 2000000 - 1; // 0.5uS
  TRAINER_TIMER->ARR = 45000;
  TRAINER_TIMER->CCR2 = GET_TRAINER_PPM_DELAY() * 2;
  TRAINER_TIMER->CCER = TIM_CCER_CC2E | (GET_TRAINER_PPM_POLARITY() ? 0 : TIM_CCER_CC2P);
  TRAINER_TIMER->CCMR1 = TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_0; // Force O/P high
  TRAINER_TIMER->BDTR = TIM_BDTR_MOE;
  TRAINER_TIMER->EGR = 1;
  TRAINER_TIMER->DIER |= TIM_DIER_UDE;
  TRAINER_TIMER->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE; // PWM mode 1
  TRAINER_TIMER->CR1 |= TIM_CR1_CEN;

  setupPulsesPPMTrainer();
  trainerSendNextFrame();

  NVIC_EnableIRQ(TRAINER_DMA_IRQn);
  NVIC_SetPriority(TRAINER_DMA_IRQn, 7);
  NVIC_EnableIRQ(TRAINER_TIMER_IRQn);
  NVIC_SetPriority(TRAINER_TIMER_IRQn, 7);
}

void stop_trainer_ppm()
{
  NVIC_DisableIRQ(TRAINER_DMA_IRQn);
  NVIC_DisableIRQ(TRAINER_TIMER_IRQn);

  TRAINER_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
  TRAINER_TIMER->DIER = 0; // Stop Interrupt
  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop counter
}

void init_trainer_capture()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = TRAINER_IN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(TRAINER_GPIO, TRAINER_IN_GPIO_PinSource, TRAINER_GPIO_AF);

  TRAINER_TIMER->ARR = 0xFFFF;
  TRAINER_TIMER->PSC = TRAINER_TIMER_FREQ / 2000000 - 1; // 0.5uS
  TRAINER_TIMER->CR2 = 0;
  TRAINER_TIMER->CCMR1 = TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_CC1S_0;
  TRAINER_TIMER->CCER = TIM_CCER_CC1E;
  TRAINER_TIMER->SR &= ~TIM_SR_CC1IF & ~TIM_SR_CC2IF & ~TIM_SR_UIF; // Clear flags
  TRAINER_TIMER->DIER |= TIM_DIER_CC1IE;
  TRAINER_TIMER->CR1 = TIM_CR1_CEN;

  NVIC_EnableIRQ(TRAINER_TIMER_IRQn);
  NVIC_SetPriority(TRAINER_TIMER_IRQn, 7);
}

void stop_trainer_capture()
{
  NVIC_DisableIRQ(TRAINER_TIMER_IRQn); // Stop Interrupt

  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop counter
  TRAINER_TIMER->DIER = 0; // Stop Interrupt
}

void trainerSendNextFrame()
{
  TRAINER_TIMER->CCR2 = GET_TRAINER_PPM_DELAY() * 2;
  TRAINER_TIMER->CCER = TIM_CCER_CC2E | (GET_TRAINER_PPM_POLARITY() ? 0 : TIM_CCER_CC2P);
  TRAINER_TIMER->CCR3 = *(trainerPulsesData.ppm.ptr - 1) - 4000; // 2mS in advance

  TRAINER_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
  TRAINER_DMA_STREAM->CR |= TRAINER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  TRAINER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&TRAINER_TIMER->ARR);
  TRAINER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(trainerPulsesData.ppm.pulses);
  TRAINER_DMA_STREAM->NDTR = trainerPulsesData.ppm.ptr - trainerPulsesData.ppm.pulses;
  TRAINER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA
}

extern "C" void TRAINER_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(TRAINER_DMA_STREAM, TRAINER_DMA_FLAG_TC))
    return;

  DMA_ClearITPendingBit(TRAINER_DMA_STREAM, TRAINER_DMA_FLAG_TC);

  TRAINER_TIMER->SR &= ~TIM_SR_CC3IF; // Clear flag
  TRAINER_TIMER->DIER |= TIM_DIER_CC3IE; // Enable this interrupt
}

extern "C" void TRAINER_TIMER_IRQHandler()
{
  DEBUG_INTERRUPT(INT_TRAINER);

  uint16_t capture = 0;
  bool doCapture = false;

  // What mode? in or out?
  if ((TRAINER_TIMER->DIER & TIM_DIER_CC1IE) && (TRAINER_TIMER->SR & TIM_SR_CC1IF)) {
    // capture mode on trainer jack
    capture = TRAINER_TIMER->CCR1;
    if (TRAINER_CONNECTED() && currentTrainerMode == TRAINER_MODE_MASTER_TRAINER_JACK) {
      doCapture = true;
    }
  }

  if (doCapture) {
    captureTrainerPulses(capture);
  }

  // PPM out compare interrupt
  if ((TRAINER_TIMER->DIER & TIM_DIER_CC3IE) && (TRAINER_TIMER->SR & TIM_SR_CC3IF)) {
    // compare interrupt
    TRAINER_TIMER->DIER &= ~TIM_DIER_CC3IE; // stop this interrupt
    TRAINER_TIMER->SR &= ~TIM_SR_CC3IF; // Clear flag
    setupPulsesPPMTrainer();
    trainerSendNextFrame();
  }
}

int sbusGetByte(uint8_t * byte)
{
  switch (currentTrainerMode) {
#if defined(AUX_SERIAL) || defined(AUX2_SERIAL)
    case TRAINER_MODE_MASTER_BATTERY_COMPARTMENT:
#if defined(AUX_SERIAL)
      if (auxSerialMode == UART_MODE_SBUS_TRAINER)
        return auxSerialRxFifo.pop(*byte);
#endif
#if defined(AUX2_SERIAL)
      if (aux2SerialMode == UART_MODE_SBUS_TRAINER)
        return aux2SerialRxFifo.pop(*byte);
#endif
#endif
    default:
      return false;
  }
}
