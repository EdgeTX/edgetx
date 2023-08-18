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
#include "watchdog_driver.h"

static volatile uint32_t msTickCount;  // Used to get 1 kHz counter
static volatile uint32_t _us_overflow_cnt;

// Start TIMER at 2000000Hz
void init2MhzTimer()
{
  _us_overflow_cnt = 0;
  TIMER_2MHz_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 2000000 - 1; // 0.5 uS, 2 MHz
  TIMER_2MHz_TIMER->ARR = 65535;
  TIMER_2MHz_TIMER->CR2 = 0;
  TIMER_2MHz_TIMER->CR1 = TIM_CR1_CEN;
  TIMER_2MHz_TIMER->DIER = TIM_DIER_UIE;

  NVIC_EnableIRQ(TIMER_2MHz_IRQn);
  NVIC_SetPriority(TIMER_2MHz_IRQn, 4);  
}

// Start TIMER at 1000Hz
void init1msTimer()
{
  msTickCount = 0;

  INTERRUPT_xMS_TIMER->ARR = 999; // 1mS in uS
  INTERRUPT_xMS_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1;  // 1uS
  INTERRUPT_xMS_TIMER->CCER = 0;
  INTERRUPT_xMS_TIMER->CCMR1 = 0;
  INTERRUPT_xMS_TIMER->EGR = 0;
  INTERRUPT_xMS_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_URS;
  INTERRUPT_xMS_TIMER->DIER = TIM_DIER_UIE;
  NVIC_EnableIRQ(INTERRUPT_xMS_IRQn);
  NVIC_SetPriority(INTERRUPT_xMS_IRQn, 4);
}

void stop1msTimer()
{
  INTERRUPT_xMS_TIMER->CR1 = 0; // stop timer
  NVIC_DisableIRQ(INTERRUPT_xMS_IRQn);
}

uint32_t timersGetMsTick()
{
  return msTickCount;
}

uint32_t timersGetUsTick()
{
  uint32_t us;
  us = TIMER_2MHz_TIMER->CNT >> 1;
  us += _us_overflow_cnt << 15;
  return us;
}

static uint32_t watchdogTimeout = 0;

void watchdogSuspend(uint32_t timeout)
{
  watchdogTimeout = timeout;
}

static void interrupt1ms()
{
  static uint8_t pre_scale = 0;

  ++pre_scale;
  ++msTickCount;


  // 5ms loop
  if(pre_scale == 5 || pre_scale == 10) {
#if defined(HAPTIC)
    DEBUG_TIMER_START(debugTimerHaptic);
    HAPTIC_HEARTBEAT();
    DEBUG_TIMER_STOP(debugTimerHaptic);
#endif
  }
  
  // 10ms loop
  if (pre_scale == 10) {
    pre_scale = 0;
    if (watchdogTimeout) {
      watchdogTimeout -= 1;
      WDG_RESET();  // Retrigger hardware watchdog
    }

    DEBUG_TIMER_START(debugTimerPer10ms);
    DEBUG_TIMER_SAMPLE(debugTimerPer10msPeriod);
    per10ms();
    DEBUG_TIMER_STOP(debugTimerPer10ms);
  }
}

extern "C" void INTERRUPT_xMS_IRQHandler()
{
  INTERRUPT_xMS_TIMER->SR &= ~TIM_SR_UIF;
  interrupt1ms();
  DEBUG_INTERRUPT(INT_5MS);
}

extern "C" void TIMER_2MHz_IRQHandler()
{
  TIMER_2MHz_TIMER->SR &= ~TIM_SR_UIF;
  _us_overflow_cnt += 1;
}
