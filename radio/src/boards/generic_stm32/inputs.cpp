/*
 * Copyright (C) EdgeTx
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

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "stm32_keys.inc"

void keysInit()
{
  _init_keys();
  _init_trims();
}

uint32_t readKeys()
{
  return _read_keys();
}

uint32_t readTrims()
{
  uint32_t trims = _read_trims();
#if defined(PCBXLITE)
  if (_read_keys() & (1 << KEY_SHIFT))
    return ((trims & 0x03) << 6) | ((trims & 0x0c) << 2);
#endif
  return trims;
}
