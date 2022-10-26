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

#include "opentx_types.h"
#include "board.h"
#include "keys.h"

uint32_t readKeys()
{
  uint32_t result = 0;

#if defined(KEYS_GPIO_REG_PGUP)
  if (~KEYS_GPIO_REG_PGUP & KEYS_GPIO_PIN_PGUP)
    result |= 1 << KEY_PGUP;
#endif
  if (~KEYS_GPIO_REG_PGDN & KEYS_GPIO_PIN_PGDN)
    result |= 1 << KEY_PGDN;
  if (~KEYS_GPIO_REG_ENTER & KEYS_GPIO_PIN_ENTER)
    result |= 1 << KEY_ENTER;
  if (~KEYS_GPIO_REG_UP & KEYS_GPIO_PIN_UP)
    result |= 1 << KEY_MODEL;
  if (~KEYS_GPIO_REG_DOWN & KEYS_GPIO_PIN_DOWN)
    result |= 1 << KEY_EXIT;
  if (~KEYS_GPIO_REG_RIGHT & KEYS_GPIO_PIN_RIGHT)
    result |= 1 << KEY_TELEM;
  if (~KEYS_GPIO_REG_LEFT & KEYS_GPIO_PIN_LEFT)
    result |= 1 << KEY_RADIO;

  // TRACE("readKeys(): %x", result);

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
  if (~TRIMS_GPIO_REG_RVD & TRIMS_GPIO_PIN_RVD)
    result |= 0x10;
  if (~TRIMS_GPIO_REG_RVU & TRIMS_GPIO_PIN_RVU)
    result |= 0x20;
  if (~TRIMS_GPIO_REG_RHL & TRIMS_GPIO_PIN_RHL)
    result |= 0x40;
  if (~TRIMS_GPIO_REG_RHR & TRIMS_GPIO_PIN_RHR)
    result |= 0x80;
  if (~TRIMS_GPIO_REG_LSD & TRIMS_GPIO_PIN_LSD)
    result |= 0x100;
  if (~TRIMS_GPIO_REG_LSU & TRIMS_GPIO_PIN_LSU)
    result |= 0x200;
  if (~TRIMS_GPIO_REG_RSD & TRIMS_GPIO_PIN_RSD)
    result |= 0x400;
  if (~TRIMS_GPIO_REG_RSU & TRIMS_GPIO_PIN_RSU)
    result |= 0x800;
  // TRACE("readTrims(): result=0x%04x", result);

  return result;
}

bool trimDown(uint8_t idx)
{
  return readTrims() & ((uint32_t)1 << idx);
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

#if defined(KEYS_GPIOG_PINS)
  INIT_KEYS_PINS(GPIOG);
#endif

#if defined(KEYS_GPIOH_PINS)
  INIT_KEYS_PINS(GPIOH);
#endif

#if defined(KEYS_GPIOI_PINS)
  INIT_KEYS_PINS(GPIOI);
#endif

#if defined(KEYS_GPIOJ_PINS)
  INIT_KEYS_PINS(GPIOJ);
#endif
}
