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

#include "async.h"

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

bool async_call(async_func_t func, void* param1, uint32_t param2)
{
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    return xTimerPendFunctionCall(func, param1, param2, 0 /* do not wait */) ==
           pdPASS;
  }

  func(nullptr, 0);
  return true;
}

void async_call_isr(async_func_t func, void* param1, uint32_t param2)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xTimerPendFunctionCallFromISR(func, nullptr, 0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    func(nullptr, 0);
  }
}
