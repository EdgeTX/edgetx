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

#include "stm32_switch_driver.h"
#include "hal/adc_driver.h"

SwitchHwPos stm32_switch_get_position(const stm32_switch_t* sw)
{
  bool inv = sw->flags & SWITCH_HW_INVERTED;
  SwitchHwPos ret = SWITCH_HW_UP;

  switch (sw->type) {
    case SWITCH_HW_2POS:
      if (!LL_GPIO_IsInputPinSet(sw->GPIOx_high, sw->Pin_high))
        ret = SWITCH_HW_DOWN;
      break;

    case SWITCH_HW_3POS: {
      auto hi = LL_GPIO_IsInputPinSet(sw->GPIOx_high, sw->Pin_high);
      auto lo = LL_GPIO_IsInputPinSet(sw->GPIOx_low, sw->Pin_low);

      if (hi && lo)
        ret = SWITCH_HW_MID;
      else if (!hi && lo)
        ret = SWITCH_HW_DOWN;
    } break;

    case SWITCH_HW_ADC: {
      uint16_t value = getAnalogValue(sw->Pin_high);
      if (value > 3 * 1024)
        ret = SWITCH_HW_DOWN;
      else if (value >= 1024)
        ret = SWITCH_HW_MID;
    } break;
  }

  if (inv) {
    return ret == SWITCH_HW_UP     ? SWITCH_HW_DOWN
           : ret == SWITCH_HW_DOWN ? SWITCH_HW_UP
                                   : ret;
  }

  return ret;
}
