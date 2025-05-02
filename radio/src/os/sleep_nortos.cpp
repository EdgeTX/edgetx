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

#include "sleep.h"

#include "timers_driver.h"
    
void sleep_ms(uint32_t ms)
{
  if (!ms) return;

  uint32_t timeout = timersGetMsTick();
  while (timersGetMsTick() - timeout < ms) {}
}

void sleep_until(time_point_t* tp, uint32_t inc)
{
  uint32_t start = *tp;
  *tp = start + inc;

  while (timersGetMsTick() - start < inc) {}
}

