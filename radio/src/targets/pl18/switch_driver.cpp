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
#include "opentx_constants.h"
#include "myeeprom.h"
#include "hal/adc_driver.h"

#include <stdlib.h>

#define __weak __attribute__((weak))

static const stm32_switch_t _switch_defs[] = {
/*  {
    "SW1L",
    nullptr, 0,
    nullptr, 0,
    SWITCH_HW_2POS, 0
  },
  {
    "SW1R",
    nullptr, 0,
    nullptr, 0,
    SWITCH_HW_2POS, 0
  },*/
  {
    "SW2",
    nullptr, 0,
    nullptr, 0,
    SWITCH_HW_2POS, 0
  },
  {
    "SW3",
    nullptr, 0,
    nullptr, 0,
    SWITCH_HW_2POS, 0
  }
};

constexpr uint8_t n_switches = 2;
constexpr uint8_t n_fct_switches = 0;

constexpr swconfig_t _switch_default_config = (swconfig_t)0
    | ((swconfig_t)SWITCH_2POS << (0 * SW_CFG_BITS))
    | ((swconfig_t)SWITCH_2POS << (1 * SW_CFG_BITS))
/*    | ((swconfig_t)SWITCH_2POS << (2 * SW_CFG_BITS))
    | ((swconfig_t)SWITCH_2POS << (3 * SW_CFG_BITS))*/
;

const switch_display_pos_t _switch_display[] = {
    { 0, 0 },
    { 0, 0 },
/*    { 0, 0 },
    { 0, 0 },*/
};

SwitchHwPos boardSwitchGetPosition(uint8_t cat, uint8_t idx)
{
  if (idx <= 1) {
//    uint16_t swVal = getAnalogValue(4);
    uint16_t swVal = getAnalogValue(5);
    if (idx == 0 && (swVal < 512 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
    if (idx == 1 && (swVal >= 3584 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
  } /*else if (idx <= 3) {
    uint16_t swVal = getAnalogValue(5);
    if (idx == 3 && (swVal < 512 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
    if (idx == 2 && (swVal >= 3584 || (swVal >= 1536 && swVal < 2560))) {
      return SWITCH_HW_DOWN;
    }
  }*/
  return SWITCH_HW_UP;
}

const char* boardSwitchGetName(uint8_t cat, uint8_t idx)
{  
  return _switch_defs[idx].name;
}

SwitchHwType boardSwitchGetType(uint8_t cat, uint8_t idx)
{
  return _switch_defs[idx].type;
}

uint8_t boardGetMaxSwitches() { return n_switches; }
uint8_t boardGetMaxFctSwitches() { return n_fct_switches; }

swconfig_t boardSwitchGetDefaultConfig() { return _switch_default_config; }

switch_display_pos_t switchGetDisplayPosition(uint8_t idx)
{
  // TODO: find a solution for FLEX switches so they can be displayed on main view
  if (idx >= DIM(_switch_display)) return {0, 0};

  return _switch_display[idx];
}
