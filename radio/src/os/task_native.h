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

#include "time_native.h"

#include <thread>
#include <mutex>
#include <memory>

#define TASK_DEFINE_STACK(name, size) void* name

using _thread_ptr_t = std::unique_ptr<std::thread>;

struct task_handle_t {
  _thread_ptr_t _thread_handle;
  uint32_t      _stack_size;
};

typedef std::mutex mutex_handle_t;

bool task_running();

void task_sleep_ms(uint32_t ms);
void task_sleep_until(time_point_t* tp, uint32_t ts_ms);

void task_shutdown_all();
