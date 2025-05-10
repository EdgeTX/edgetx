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

#include "task.h"

static void _task_stub(void* p)
{
  task_func_t func = (task_func_t)p;
  func();
}

void task_create(task_handle_t* h, task_func_t func, const char* name,
                 void* stack, unsigned stack_size, unsigned priority)
{
  h->_stack_size = stack_size;
  h->_rtos_handle =
      xTaskCreateStatic(_task_stub, name, stack_size, (void*)func, priority,
                        (StackType_t*)stack, &h->_task_struct);
}

unsigned task_get_stack_usage(task_handle_t* h)
{
  return (h->_stack_size - uxTaskGetStackHighWaterMark(h->_rtos_handle)) *
         sizeof(StackType_t);
}

unsigned task_get_stack_size(task_handle_t* h)
{
  return h->_stack_size * sizeof(StackType_t);
}

void mutex_create(mutex_handle_t* h)
{
  h->_rtos_handle = xSemaphoreCreateMutexStatic(&h->_mutex_struct);
  mutex_unlock(h);
}

static inline bool _lock_mutex(mutex_handle_t* h, TickType_t xTickToWait)
{
  return xSemaphoreTake(h->_rtos_handle, xTickToWait) == pdTRUE;
}

bool mutex_lock(mutex_handle_t* h) { return _lock_mutex(h, portMAX_DELAY); }
bool mutex_trylock(mutex_handle_t* h) { return _lock_mutex(h, (TickType_t)0); }
void mutex_unlock(mutex_handle_t* h) { xSemaphoreGive(h->_rtos_handle); }
