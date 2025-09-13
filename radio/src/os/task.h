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

#pragma once

#include <stdbool.h>

// OS specific implementation
#if defined(NATIVE_THREADS)
  #include "task_native.h"
#elif defined(FREE_RTOS)
  #include "task_freertos.h"
#endif

typedef void (*task_func_t)();

void task_create(task_handle_t* h, task_func_t func, const char* name,
                 void* stack, unsigned stack_size, unsigned priority);

unsigned task_get_stack_usage(task_handle_t* h);
unsigned task_get_stack_size(task_handle_t* h);

void mutex_create(mutex_handle_t* h);
bool mutex_lock(mutex_handle_t* h);
void mutex_unlock(mutex_handle_t* h);
bool mutex_trylock(mutex_handle_t* h);

