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

#include "mixer_scheduler.h"
#include "stm32_hal_ll.h"
#include "stm32_timer.h"

#include "FreeRTOSConfig.h"
#include "hal.h"

// Start scheduler with default period
void mixerSchedulerStart()
{
  stm32_timer_enable_clock(MIXER_SCHEDULER_TIMER);

  MIXER_SCHEDULER_TIMER->CR1 &= ~TIM_CR1_CEN;
  MIXER_SCHEDULER_TIMER->PSC   = MIXER_SCHEDULER_TIMER_FREQ / 1000000 - 1; // 1uS (1Mhz)
  MIXER_SCHEDULER_TIMER->CCER  = 0;
  MIXER_SCHEDULER_TIMER->CCMR1 = 0;
  MIXER_SCHEDULER_TIMER->ARR   = getMixerSchedulerPeriod() - 1;
  MIXER_SCHEDULER_TIMER->CNT   = 0;   // reset counter

  NVIC_EnableIRQ(MIXER_SCHEDULER_TIMER_IRQn);
  NVIC_SetPriority(MIXER_SCHEDULER_TIMER_IRQn,
                   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

  MIXER_SCHEDULER_TIMER->SR   &= TIM_SR_UIF;   // clear interrupt flag
  MIXER_SCHEDULER_TIMER->DIER |= TIM_DIER_UIE; // enable interrupt
  MIXER_SCHEDULER_TIMER->CR1  |= TIM_CR1_CEN;
}

void mixerSchedulerStop()
{
  MIXER_SCHEDULER_TIMER->CR1 &= ~TIM_CR1_CEN;
  NVIC_DisableIRQ(MIXER_SCHEDULER_TIMER_IRQn);
}

void mixerSchedulerEnableTrigger()
{
  MIXER_SCHEDULER_TIMER->DIER |= TIM_DIER_UIE; // enable interrupt
}

void mixerSchedulerDisableTrigger()
{
  MIXER_SCHEDULER_TIMER->DIER &= ~TIM_DIER_UIE; // disable interrupt
}

void mixerSchedulerSoftTrigger() {
  // Generate a timer update event (TIM_EGR_UG) to reload the Prescaler and the repetition 
  // counter value immediately to avoid making FreeRTOS calls within this ISR:
  // - fires MIXER_SCHEDULER_TIMER interrupt after returning from this ISR
  // - MIXER_SCHEDULER_TIMER_IRQHandler(void) takes care of making FreeRTOS calls
  //   to ensure switching to highest priority task.
  MIXER_SCHEDULER_TIMER->EGR = TIM_EGR_UG; 
}

extern "C" void MIXER_SCHEDULER_TIMER_IRQHandler(void)
{
  MIXER_SCHEDULER_TIMER->SR &= ~TIM_SR_UIF; // clear flag
  mixerSchedulerDisableTrigger();

  // set next period
  MIXER_SCHEDULER_TIMER->ARR = getMixerSchedulerPeriod() - 1;

  // trigger mixer start
  mixerSchedulerISRTrigger();
}
