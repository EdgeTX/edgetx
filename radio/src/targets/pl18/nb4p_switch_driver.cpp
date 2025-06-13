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

#include "hal/switch_driver.h"
#include "stm32_switch_driver.h"
//#include "stm32_gpio_driver.h"

#include "definitions.h"
#include "edgetx_constants.h"
#include "myeeprom.h"
#include "hal/adc_driver.h"

#include <stdlib.h>

static const char _switch_names[][4] = {"SW2", "SW3"};

SwitchHwPos boardSwitchGetPosition(SwitchCategory cat, uint8_t idx)
{
  if (idx <= 1) {
//    uint16_t swVal = getAnalogValue(4);
    uint16_t swVal = getAnalogValue(5);
    if (idx == 0 && (swVal >= 3584 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
    if (idx == 1 && (swVal < 512 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
/*  } else if (idx <= 3) {
    uint16_t swVal = getAnalogValue(5);
    if (idx == 3 && (swVal < 512 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
    if (idx == 2 && (swVal >= 3584 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }*/
  }
  return SWITCH_HW_UP;
}

const char* boardSwitchGetName(SwitchCategory cat, uint8_t idx)
{  
  return _switch_names[idx];
}
