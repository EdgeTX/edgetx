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

#include "timers_driver.h"
#include "stm32_timer.h"
#include "stm32_hal_ll.h"

#include "hal.h"
#include "hal/watchdog_driver.h"

#include "FreeRTOSConfig.h"

static volatile uint32_t _ms_ticks;

static void _init_1ms_timer()
{
  stm32_timer_enable_clock(MS_TIMER);
  if ((MS_TIMER->CR1 & TIM_CR1_CEN) == TIM_CR1_CEN) return;

  _ms_ticks = 0;
  MS_TIMER->ARR = 999; // 1mS in uS
  MS_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1;  // 1uS
  MS_TIMER->CCER = 0;
  MS_TIMER->CCMR1 = 0;
  MS_TIMER->EGR = 0;
  MS_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_URS;
  MS_TIMER->DIER = TIM_DIER_UIE;

  NVIC_EnableIRQ(MS_TIMER_IRQn);
  NVIC_SetPriority(MS_TIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
}

void timersInit()
{
  _init_1ms_timer();
}

uint32_t timersGetMsTick()
{
  return _ms_ticks;
}

uint32_t timersGetUsTick()
{
  uint32_t ms;
  uint32_t us;

  do {
    ms = _ms_ticks;
    us = MS_TIMER->CNT;
    asm volatile("nop");
    asm volatile("nop");
  } while (ms != _ms_ticks);
  
  return ms * 1000 + us;
}

static volatile uint32_t watchdogTimeout = 0;

void watchdogSuspend(uint32_t timeout)
{
  watchdogTimeout = timeout;
}

static inline void _interrupt_1ms()
{
  static uint8_t pre_scale = 0;

  ++pre_scale;
  ++_ms_ticks;

  // 5ms loop
  if(pre_scale == 5 || pre_scale == 10) {
    per5ms();
  }
  
  // 10ms loop
  if (pre_scale == 10) {
    pre_scale = 0;

    if (watchdogTimeout) {
      watchdogTimeout -= 1;
      WDG_RESET();  // Retrigger hardware watchdog
    }

    per10ms();
  }
}

extern "C" void MS_TIMER_IRQHandler()
{
  MS_TIMER->SR &= ~TIM_SR_UIF;
  _interrupt_1ms();
}
