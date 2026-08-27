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

#include "time.h"

#include <atomic>

struct timer_handle_t;
typedef void (*timer_func_t)(timer_handle_t*);
typedef void (*timer_async_func_t)(void*, uint32_t);

using time_point = std::chrono::time_point<std::chrono::steady_clock>;

struct timer_handle_t {
  timer_func_t func;
  const char*  name;
  unsigned     period;
  bool         repeat;

  time_point_t next_trigger;
  std::atomic_bool active;
};

#define TIMER_INITIALIZER {0}

