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

#include "hal/key_driver.h"

#include "bsp_io.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_i2c_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "keys.h"

#include "stm32_keys.inc"
#include "debug.h"

void keysInit() {
  _init_keys();
  _init_trims();
}

uint32_t readKeys() { return _read_keys(); }

/* The output bit-order has to be:
   0  LHL  TR3L (Left equals down)
   1  LHR  TR3R
   2  LVD  TR1D
   3  LVU  TR1U
   4  RVD  TR2D
   5  RVU  TR2U
   6  RHL  TR4L
   7  RHR  TR4R
*/

enum PhysicalTrims {
  TR3L = 0,
  TR3R,
  TR1D = 2,
  TR1U,
  TR2D = 4,
  TR2U,
  TR4L = 6,
  TR4R,
  TR5U = 8,
  TR5D,
  TR6U = 10,
  TR6D
};

uint32_t readTrims()
{
  uint32_t result = 0;
  uint32_t keys = bsp_get_fs_switches();
  
#define _TRIM(t) \
  if ((keys & BSP_##t) == 0) result |= 1 << t;

  _TRIM(TR1U);
  _TRIM(TR1D);
  _TRIM(TR3L);
  _TRIM(TR3R);
  _TRIM(TR2U);
  _TRIM(TR2D);
  _TRIM(TR4L);
  _TRIM(TR4R);
  if (!LL_GPIO_IsInputPinSet(GPIOH, LL_GPIO_PIN_15)) result |= (1 << TR5U);
  if (!LL_GPIO_IsInputPinSet(GPIOH, LL_GPIO_PIN_13)) result |= (1 << TR5D);
  _TRIM(TR6U);
  _TRIM(TR6D);

#undef _TRIM

  return result;
}
