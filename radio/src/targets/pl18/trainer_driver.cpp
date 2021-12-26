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

#include "opentx.h"

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
  TRAINER_TIMER->CCR2 = GET_TRAINER_PPM_DELAY()*2;
  TRAINER_TIMER->CCER = TIM_CCER_CC2E | (GET_TRAINER_PPM_POLARITY() ? 0 : TIM_CCER_CC2P);
  TRAINER_TIMER->CCMR1 = TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_0; // Force O/P high
  TRAINER_TIMER->BDTR = TIM_BDTR_MOE;
  TRAINER_TIMER->EGR = 1;
  TRAINER_TIMER->DIER |= TIM_DIER_UDE;
  TRAINER_TIMER->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE; // PWM mode 1
  TRAINER_TIMER->CR1 |= TIM_CR1_CEN;

  trainerSendNextFrame();

  NVIC_EnableIRQ(TRAINER_TIMER_IRQn);
  NVIC_SetPriority(TRAINER_TIMER_IRQn, 7);
}

void stop_trainer_ppm()
{
  TRAINER_TIMER->DIER = 0; // Stop Interrupt
  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop counter
}

void init_trainer_capture()
{
  GPIO_PinAFConfig(TRAINER_GPIO, TRAINER_IN_GPIO_PinSource, TRAINER_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TRAINER_IN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_GPIO, &GPIO_InitStructure);

  TRAINER_TIMER->ARR = 0xFFFF;
  TRAINER_TIMER->PSC = TRAINER_TIMER_FREQ / 2000000 - 1; // 0.5uS
  TRAINER_TIMER->CR2 = 0;
  TRAINER_TIMER->CCMR1 = TRAINER_IN_CCMR1;
  TRAINER_TIMER->CCER = TRAINER_IN_CCER;
  TRAINER_TIMER->SR &= ~TIM_SR_CC3IF & ~TIM_SR_CC2IF & ~TIM_SR_UIF; // Clear flags
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
short unsigned int* lastPulse = 0;

void trainerSendNextFrame()
{
  setupPulsesPPMTrainer();
  TRAINER_TIMER->CCR2 = GET_TRAINER_PPM_DELAY() * 2;
  TRAINER_TIMER->CCER = TIM_CCER_CC2E | (GET_TRAINER_PPM_POLARITY() ? 0 : TIM_CCER_CC2P);
  lastPulse = trainerPulsesData.ppm.ptr;
  TRAINER_TIMER->CCR3 = *(trainerPulsesData.ppm.ptr - 1) - 4000; // 2mS in advance

  trainerPulsesData.ppm.ptr = trainerPulsesData.ppm.pulses;
  TRAINER_TIMER->DIER |= TIM_DIER_UDE;
  TRAINER_TIMER->SR &= ~TIM_SR_UIF; // Clear this flag
  TRAINER_TIMER->DIER |= TIM_DIER_UIE; // Enable this interrupt
}

extern "C" void TRAINER_TIMER_IRQHandler()
{
  DEBUG_INTERRUPT(INT_TRAINER);
  uint16_t capture = 0;
  bool doCapture = false;
  
  if ((TRAINER_TIMER->DIER & TIM_DIER_CC1IE) && (TRAINER_TIMER->SR & TIM_SR_CC1IF)) {
    // capture mode on trainer jack
    capture = TRAINER_IN_COUNTER_REGISTER;
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
    trainerSendNextFrame();
  }

  if ((TRAINER_TIMER->DIER & TIM_DIER_UIE) && (TRAINER_TIMER->SR & TIM_SR_UIF)) {
    TRAINER_TIMER->SR &= ~TIM_SR_UIF; // Clear flag
    TRAINER_TIMER->ARR = *trainerPulsesData.ppm.ptr++;
    if (trainerPulsesData.ppm.ptr == lastPulse) {
      TRAINER_TIMER->SR &= ~TIM_SR_CC3IF; // Clear flag
      TRAINER_TIMER->DIER |= TIM_DIER_CC3IE; // Enable this interrupt
    }
  }
}
