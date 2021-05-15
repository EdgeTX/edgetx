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

#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "edgetx_types.h"

#define TMR_OFF      0
#define TMR_RUNNING  1
#define TMR_NEGATIVE 2
#define TMR_STOPPED  3

typedef int32_t tmrval_t;
typedef uint32_t tmrstart_t;
typedef int16_t tmrmode_t;
#define TIMER_MAX     (0xffffff/2)

#define TIMER_MIN     (tmrval_t(-TIMER_MAX-1))

struct TimerState {
  uint16_t cnt;
  uint16_t sum;
  uint8_t  state;
  tmrval_t  val;
  uint8_t  val_10ms;
};

#if defined(TIMERS)
extern TimerState timersStates[TIMERS];
#endif

void timerReset(uint8_t idx);

void timerSet(int idx, int val);

void saveTimers();
void restoreTimers();

void evalTimers(int16_t throttle, uint8_t tick10ms);

extern volatile tmr10ms_t g_tmr10ms;
static inline tmr10ms_t get_tmr10ms()
{
  return g_tmr10ms;
}

#endif // _TIMERS_H_
