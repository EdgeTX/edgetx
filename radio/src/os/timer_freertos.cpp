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

#include "timer.h"

static void _timer_cb(TimerHandle_t xTimer)
{
  timer_handle_t* h = (timer_handle_t*)pvTimerGetTimerID(xTimer);
  if (h->_func) h->_func(h);
}

int timer_create(timer_handle_t* h, timer_func_t func, const char* name,
                  unsigned period, bool repeat)
{
  if (h->_rtos_handle) return -1;
  h->_rtos_handle = xTimerCreateStatic(name, period / portTICK_PERIOD_MS,
                                       repeat ? pdTRUE : pdFALSE, (void*)h,
                                       _timer_cb, &h->_timer_struct);
  h->_func = func;
  return 0;
}

bool timer_is_created(timer_handle_t* h)
{
  return h->_rtos_handle != nullptr;  
}

bool timer_is_active(timer_handle_t* h)
{
  return (h->_rtos_handle != nullptr) && (xTimerIsTimerActive(h->_rtos_handle));
}

int timer_start(timer_handle_t* h)
{
  if (!h->_rtos_handle) return -1;
  return xTimerStart(h->_rtos_handle, 0) == pdPASS ? 0 : -1;
}

int timer_stop(timer_handle_t* h)
{
  if (!h->_rtos_handle) return -1;
  return xTimerStop(h->_rtos_handle, 0) == pdPASS ? 0 : -1;
}

int timer_set_period(timer_handle_t* h, unsigned period)
{
  if (!h->_rtos_handle) return -1;
  return xTimerChangePeriod(h->_rtos_handle, period / portTICK_PERIOD_MS, 0) ==
                 pdPASS
             ? 0
             : -1;
}
