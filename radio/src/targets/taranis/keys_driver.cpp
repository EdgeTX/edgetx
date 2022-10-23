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

#include "board.h"

uint32_t readKeys()
{
  uint32_t result = 0;

  if (~KEYS_GPIO_REG_ENTER & KEYS_GPIO_PIN_ENTER)
    result |= 1 << KEY_ENTER;

#if defined(KEYS_GPIO_PIN_MENU)
  if (~KEYS_GPIO_REG_MENU & KEYS_GPIO_PIN_MENU)
    result |= 1 << KEY_MENU;
#endif

#if defined(KEYS_GPIO_PIN_PAGE)
  if (~KEYS_GPIO_REG_PAGE & KEYS_GPIO_PIN_PAGE)
    result |= 1 << KEY_PAGE;
#endif

#if defined(KEYS_GPIO_PIN_PAGEUP)
  if (~KEYS_GPIO_REG_PAGEUP & KEYS_GPIO_PIN_PAGEUP)
    result |= 1 << KEY_PAGEUP;
#endif

#if defined(KEYS_GPIO_PIN_PAGEDN)
  if (~KEYS_GPIO_REG_PAGEDN & KEYS_GPIO_PIN_PAGEDN)
    result |= 1 << KEY_PAGEDN;
#endif

#if defined(KEYS_GPIO_PIN_SYS)
  if (~KEYS_GPIO_REG_SYS & KEYS_GPIO_PIN_SYS)
    result |= 1 << KEY_SYS;
#endif

#if defined(KEYS_GPIO_PIN_MDL)
  if (~KEYS_GPIO_REG_MDL & KEYS_GPIO_PIN_MDL)
    result |= 1 << KEY_MODEL;
#endif

#if defined(KEYS_GPIO_PIN_TELE)
  if (~KEYS_GPIO_REG_TELE & KEYS_GPIO_PIN_TELE)
    result |= 1 << KEY_TELE;
#endif

  if (~KEYS_GPIO_REG_EXIT & KEYS_GPIO_PIN_EXIT)
    result |= 1 << KEY_EXIT;

#if defined(KEYS_GPIO_PIN_PLUS)
  if (~KEYS_GPIO_REG_PLUS & KEYS_GPIO_PIN_PLUS)
    result |= 1 << KEY_PLUS;
  if (~KEYS_GPIO_REG_MINUS & KEYS_GPIO_PIN_MINUS)
    result |= 1 << KEY_MINUS;
#endif

#if defined(KEYS_GPIO_PIN_LEFT)
  if (~KEYS_GPIO_REG_LEFT & KEYS_GPIO_PIN_LEFT)
    result |= 1 << KEY_LEFT;
  if (~KEYS_GPIO_REG_RIGHT & KEYS_GPIO_PIN_RIGHT)
    result |= 1 << KEY_RIGHT;
  if (~KEYS_GPIO_REG_UP & KEYS_GPIO_PIN_UP)
    result |= 1 << KEY_UP;
  if (~KEYS_GPIO_REG_DOWN & KEYS_GPIO_PIN_DOWN)
    result |= 1 << KEY_DOWN;
#endif

#if defined(KEYS_GPIO_PIN_SHIFT)
  if (~KEYS_GPIO_REG_SHIFT & KEYS_GPIO_PIN_SHIFT)
    result |= 1 << KEY_SHIFT;
#endif

#if defined(KEYS_GPIO_PIN_BIND)
  if (~KEYS_GPIO_REG_BIND & KEYS_GPIO_PIN_BIND)
    result |= 1 << KEY_BIND;
#endif
  // if (result != 0) TRACE("readKeys(): result=0x%02x", result);

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  if (~TRIMS_GPIO_REG_LHL & TRIMS_GPIO_PIN_LHL)
    result |= 0x01;
  if (~TRIMS_GPIO_REG_LHR & TRIMS_GPIO_PIN_LHR)
    result |= 0x02;
  if (~TRIMS_GPIO_REG_LVD & TRIMS_GPIO_PIN_LVD)
    result |= 0x04;
  if (~TRIMS_GPIO_REG_LVU & TRIMS_GPIO_PIN_LVU)
    result |= 0x08;

#if defined(PCBXLITE)
  if (IS_SHIFT_PRESSED())
    result = ((result & 0x03) << 6) | ((result & 0x0c) << 2);
#else
  if (~TRIMS_GPIO_REG_RVD & TRIMS_GPIO_PIN_RVD)
    result |= 0x10;
  if (~TRIMS_GPIO_REG_RVU & TRIMS_GPIO_PIN_RVU)
    result |= 0x20;
  if (~TRIMS_GPIO_REG_RHL & TRIMS_GPIO_PIN_RHL)
    result |= 0x40;
  if (~TRIMS_GPIO_REG_RHR & TRIMS_GPIO_PIN_RHR)
    result |= 0x80;
#endif

  // TRACE("readTrims(): result=0x%02x", result);

  return result;
}

bool trimDown(uint8_t idx)
{
  return readTrims() & (1 << idx);
}

bool keyDown()
{
  return readKeys() || readTrims();
}

void keysInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;

#if defined(KEYS_GPIOA_PINS)
  INIT_KEYS_PINS(GPIOA);
#endif

#if defined(KEYS_GPIOB_PINS)
  INIT_KEYS_PINS(GPIOB);
#endif

#if defined(KEYS_GPIOC_PINS)
  INIT_KEYS_PINS(GPIOC);
#endif

#if defined(KEYS_GPIOD_PINS)
  INIT_KEYS_PINS(GPIOD);
#endif

#if defined(KEYS_GPIOE_PINS)
  INIT_KEYS_PINS(GPIOE);
#endif

#if defined(KEYS_GPIOF_PINS)
  INIT_KEYS_PINS(GPIOF);
#endif

#if defined(KEYS_GPIOG_PINS)
  INIT_KEYS_PINS(GPIOG);
#endif
}

#if !defined(BOOT)

#include <stdlib.h>

static const stm32_switch_t _switch_defs[] = {
  #include "hw_switches.inc"
};

uint8_t switchGetMaxSwitches()
{
  return DIM(_switch_defs);
}
  
// returns state (0 / 1) of a specific switch position
uint32_t switchState(uint8_t pos_idx)
{
  auto d = div(pos_idx, 3);
  return stm32_switch_get_state(&_switch_defs[d.quot], (SwitchHwPos)d.rem);
}

SwitchHwPos switchGetPosition(uint8_t idx)
{
  if (idx >= DIM(_switch_defs)) return SWITCH_HW_UP;
  return stm32_switch_get_position(&_switch_defs[idx]);
}

#endif
