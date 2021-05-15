/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"


// keys:
// KEY_EXIT    PA31 (PC24)
// KEY_MENU    PB6 (PB5)
// KEY_DOWN  LCD5  PC3 (PC5)
// KEY_UP    LCD6  PC2 (PC1)
// KEY_RIGHT LCD4  PC4 (PC4)
// KEY_LEFT  LCD3  PC5 (PC3)
// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
// LCD pins 5 DOWN, 4 RIGHT, 3 LEFT, 1 UP
uint32_t readKeys()
{
  uint32_t x;
  uint32_t result = 0;

  x = lcdLock ? lcdInputs : PIOC->PIO_PDSR; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
  x = ~x;

  if (x & KEYS_GPIO_PIN_RIGHT)
    result |= 0x02 << KEY_RIGHT;
  if (x & KEYS_GPIO_PIN_UP)
    result |= 0x02 << KEY_UP;
  if (x & KEYS_GPIO_PIN_LEFT)
    result |= 0x02 << KEY_LEFT;
  if (x & KEYS_GPIO_PIN_DOWN)
    result |= 0x02 << KEY_DOWN;
  if (x & KEYS_GPIO_PIN_EXIT)
    result |= 0x02 << KEY_EXIT;
  if (~KEYS_GPIO_REG_MENU & KEYS_GPIO_PIN_MENU)
    result |= 0x02 << KEY_MENU;

  // TRACE("readKeys(): %x => %x", x, result);

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  if (~TRIMS_GPIO_REG_LHL & TRIMS_GPIO_PIN_LHL)
    result |= 0x01;
  if (~TRIMS_GPIO_REG_LVD & TRIMS_GPIO_PIN_LVD)
    result |= 0x04;
  if (~TRIMS_GPIO_REG_RVU & TRIMS_GPIO_PIN_RVU)
    result |= 0x20;
  if (~TRIMS_GPIO_REG_RHL & TRIMS_GPIO_PIN_RHL)
    result |= 0x40;
  if (~TRIMS_GPIO_REG_LHR & TRIMS_GPIO_PIN_LHR)
    result |= 0x02;
  if (~TRIMS_GPIO_REG_LVU & TRIMS_GPIO_PIN_LVU)
    result |= 0x08;
  if (~TRIMS_GPIO_REG_RVD & TRIMS_GPIO_PIN_RVD)
    result |= 0x10;
  if (~TRIMS_GPIO_REG_RHR & TRIMS_GPIO_PIN_RHR)
    result |= 0x80;

  return result;
}

bool trimDown(uint8_t idx)
{
  return readTrims() & (1 << idx);
}

bool keyDown()
{
  return readKeys() || REA_DOWN();
}

void readKeysAndTrims()
{
  uint32_t i;

  uint8_t index = 0;
  uint8_t keys_input = readKeys();
  for (i = 1; i < 7; i++) {
    keys[index].input(keys_input & (1 << i));
    ++index;
  }

  uint8_t trims_input = readTrims();
  for (i = 1; i < 256; i <<= 1) {
    keys[index].input(trims_input & i);
    ++index;
  }

  if ((keys_input || trims_input) && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
    // on keypress turn the light on
    resetBacklightTimeout();
  }
}

uint32_t switchState(uint8_t index)
{
  uint32_t a = PIOA->PIO_PDSR;
  uint32_t c = PIOC->PIO_PDSR;

  uint32_t xxx = 0;

  switch (index) {
    case SW_ELE:
      xxx = c & 0x80000000; // ELE_DR   PC31
      break;

    case SW_AIL:
      xxx = a & 0x00000004; // AIL-DR  PA2
      break;

    case SW_RUD:
      xxx = a & 0x00008000; // RUN_DR   PA15
      break;
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0:
      xxx = ~c & 0x00004000; // SW_IDL1     PC14
      break;
    case SW_ID1:
      xxx = (c & 0x00004000);
      if (xxx) xxx = (c & 0x00000800);
      break;
    case SW_ID2:
      xxx = ~c & 0x00000800; // SW_IDL2     PC11
      break;

    case SW_GEA:
      xxx = c & 0x00010000; // SW_GEAR     PC16
      break;

    case SW_THR:
      xxx = c & 0x00100000; // SW_TCUT     PC20
      break;

    case SW_TRN:
      xxx = c & 0x00000100; // SW-TRAIN    PC8
      break;

    default:
      break;
  }

  return xxx;
}
