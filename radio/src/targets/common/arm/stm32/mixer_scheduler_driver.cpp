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

#include "opentx.h"
#include "mixer_scheduler.h"

#if !defined(SIMU)

// Start scheduler with default period
void mixerSchedulerStart()
{
  MIXER_SCHEDULER_TIMER->CR1 &= ~TIM_CR1_CEN;

  MIXER_SCHEDULER_TIMER->CR1   = TIM_CR1_URS; // do not generate interrupt on soft update
  MIXER_SCHEDULER_TIMER->PSC   = MIXER_SCHEDULER_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)
  MIXER_SCHEDULER_TIMER->CCER  = 0;
  MIXER_SCHEDULER_TIMER->CCMR1 = 0;
  MIXER_SCHEDULER_TIMER->ARR   = 2 * getMixerSchedulerPeriod() - 1;
  MIXER_SCHEDULER_TIMER->EGR   = TIM_EGR_UG;   // reset timer

  NVIC_EnableIRQ(MIXER_SCHEDULER_TIMER_IRQn);
  NVIC_SetPriority(MIXER_SCHEDULER_TIMER_IRQn, 8);

  MIXER_SCHEDULER_TIMER->SR   &= TIM_SR_UIF;   // clear interrupt flag
  MIXER_SCHEDULER_TIMER->DIER |= TIM_DIER_UIE; // enable interrupt
  MIXER_SCHEDULER_TIMER->CR1  |= TIM_CR1_CEN;
}

void mixerSchedulerStop()
{
  MIXER_SCHEDULER_TIMER->CR1 &= ~TIM_CR1_CEN;
  NVIC_DisableIRQ(MIXER_SCHEDULER_TIMER_IRQn);
}

void mixerSchedulerResetTimer()
{
  mixerSchedulerDisableTrigger();
  MIXER_SCHEDULER_TIMER->CNT = 0;
  mixerSchedulerEnableTrigger();
}

void mixerSchedulerEnableTrigger()
{
  MIXER_SCHEDULER_TIMER->DIER |= TIM_DIER_UIE; // enable interrupt
}

void mixerSchedulerDisableTrigger()
{
  MIXER_SCHEDULER_TIMER->DIER &= ~TIM_DIER_UIE; // disable interrupt
}

extern "C" void MIXER_SCHEDULER_TIMER_IRQHandler(void)
{
  MIXER_SCHEDULER_TIMER->SR &= ~TIM_SR_UIF; // clear flag
  mixerSchedulerDisableTrigger();

  // set next period
  MIXER_SCHEDULER_TIMER->ARR = 2 * getMixerSchedulerPeriod() - 1;

  // trigger mixer start
  mixerSchedulerISRTrigger();
}

#endif
