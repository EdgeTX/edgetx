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
#include "timer_native_impl.h"

bool async_call(async_func_t func, volatile bool* excl_flag, void* param1,
                uint32_t param2)
{
  if (excl_flag && *excl_flag) return false;
  timer_queue::instance().pend_function(func, param1, param2);
  if (excl_flag) *excl_flag = true;
  return true;
}

bool async_call_isr(async_func_t func, volatile bool* excl_flag, void* param1,
                    uint32_t param2)
{
  return async_call(func, excl_flag, param1, param2);
}
