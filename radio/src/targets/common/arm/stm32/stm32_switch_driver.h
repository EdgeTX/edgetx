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

#include "hal.h"
#include "stm32_hal_ll.h"
#include "hal/switch_driver.h"
#include "edgetx_constants.h"

struct stm32_switch_t
{
  const char*   name;

  GPIO_TypeDef* GPIOx_high;
  uint32_t      Pin_high;

  GPIO_TypeDef* GPIOx_low;
  uint32_t      Pin_low;

  SwitchHwType  type;
  bool          inverted;
  SwitchConfig  defaultType;

#if defined(FUNCTION_SWITCHES)
  bool          isCustomSwitch;
  uint8_t       customSwitchIdx;
#endif
};

SwitchHwPos stm32_switch_get_position(const stm32_switch_t* sw);
bool stm32_switch_get_state(const stm32_switch_t* sw, SwitchHwPos pos);
