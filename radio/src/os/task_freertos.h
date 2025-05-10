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

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/task.h>
#include <FreeRTOS/include/semphr.h>

#include "definitions.h"

#define TASK_DEFINE_STACK(name, size) StackType_t __ALIGNED(8) name[size] __CCMRAM

struct task_handle_t {
  TaskHandle_t _rtos_handle;
  StaticTask_t _task_struct;
  uint32_t     _stack_size;
};

struct mutex_handle_t {
  SemaphoreHandle_t _rtos_handle;
  StaticSemaphore_t _mutex_struct;
};

static inline bool task_running() { return true; }
