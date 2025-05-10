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

#include <stdint.h>
#include <stdbool.h>

// OS specific implementation
#if defined(POSIX_THREADS)
  #include "timer_pthread.h"
#elif defined(FREE_RTOS)
  #include "timer_freertos.h"
#endif

typedef void (*timer_func_t)(timer_handle_t*);

int timer_create(timer_handle_t* h, timer_func_t func, const char* name,
                 unsigned period, bool repeat);

bool timer_is_created(timer_handle_t* h);
bool timer_is_active(timer_handle_t* h);

int timer_start(timer_handle_t* h);
int timer_stop(timer_handle_t* h);

int timer_set_period(timer_handle_t* h, unsigned period);
