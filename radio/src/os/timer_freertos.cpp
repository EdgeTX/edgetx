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

int timer_create(timer_handle_t* h, timer_func_t func, const char* name,
                  unsigned period, bool repeat)
{
  if (h->_rtos_handle) return -1;
  h->_rtos_handle = xTimerCreateStatic(name, period / portTICK_RATE_MS,
                                       repeat ? pdTRUE : pdFALSE, (void*)0,
                                       func, &h->_timer_struct);
  return 0;
}

bool timer_is_created(timer_handle_t* h)
{
  return h->_rtos_handle != nullptr;  
}

int timer_start(timer_handle_t* h)
{
  if (!h->_rtos_handle) return -1;
  // TODO: do we need some sort of wait time?
  return xTimerStart(h->_rtos_handle, 0) == pdPASS ? 0 : -1;
}

int timer_stop(timer_handle_t* h)
{
  if (!h->_rtos_handle) return -1;
  // TODO: do we need some sort of wait time?
  return xTimerStop(h->_rtos_handle, 0) == pdPASS ? 0 : -1;
}
