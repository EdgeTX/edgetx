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

#include "edgetx.h"

#include "hal/rotary_encoder.h"
#include "hal/switch_driver.h"

void displayKeyState(uint8_t x, uint8_t y, uint8_t key)
{
  uint8_t t = keysGetState(key);
  lcdDrawChar(x, y, t+'0', t ? INVERS : 0);
}

void displayTrimState(uint8_t x, uint8_t y, uint8_t trim)
{
  uint8_t t = keysGetTrimState(trim);
  lcdDrawChar(x, y, t+'0', t ? INVERS : 0);
}

static EnumKeys get_ith_key(uint8_t i)
{
  auto supported_keys = keysGetSupported();
  for (uint8_t k = 0; k < MAX_KEYS; k++) {
    if (supported_keys & (1 << k)) {
      if (i-- == 0) return (EnumKeys)k;
    }
  }

  // should not get here,
  // we assume: i < keysGetMaxKeys()
  return (EnumKeys)0;
}

void menuRadioDiagKeys(event_t event)
{
  SIMPLE_SUBMENU(STR_MENU_RADIO_SWITCHES, 1);

  lcdDrawText(24*FW, MENU_HEADER_HEIGHT + 1, STR_VTRIM);

  for (uint8_t i = 0; i < keysGetMaxTrims() * 2; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + FH + FH * (i / 2);
    if (i & 1) lcdDraw1bitBitmap(24 * FW, y, sticks, i / 2, 0);
    displayTrimState(i & 1 ? 30 * FW : 28 * FW, y, i);
  }

  for (uint8_t i = 0; i < keysGetMaxKeys(); i++) {
    auto k = get_ith_key(i);
    coord_t y = MENU_HEADER_HEIGHT + 1 + FH * i;
    lcdDrawText(0, y, keysGetLabel(k), 0);
    displayKeyState(5 * FW + 2, y, k);
  }

  for (uint8_t i = 0, cnt = 0; i < switchGetMaxSwitches(); i++) {
    if (SWITCH_EXISTS(i)) {
      div_t qr = div(cnt++, 6);
      coord_t x = 4 * FH * qr.quot;
      coord_t y = MENU_HEADER_HEIGHT + 1 + FH * qr.rem;
      getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
      getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
      drawSwitch(8 * FW + x, y, sw, 0);
    }
  }

#if defined(ROTARY_ENCODER_NAVIGATION)
  coord_t y = MENU_HEADER_HEIGHT + 1 + FH * MAX_KEYS;
  lcdDrawText(0, y, STR_ROTARY_ENCODER);
  lcdDrawNumber(5 * FW + FWNUM + 2, y, rotaryEncoderGetValue(), RIGHT);
#endif
}
