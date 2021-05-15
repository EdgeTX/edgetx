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

void displayKeyState(uint8_t x, uint8_t y, uint8_t key)
{
  uint8_t t = keys[key].state();
  lcdDrawChar(x, y, t+'0', t ? INVERS : 0);
}

void menuRadioDiagKeys(event_t event)
{
  SIMPLE_SUBMENU(STR_MENU_RADIO_SWITCHES, 1);

  lcdDrawText(24*FW, MENU_HEADER_HEIGHT + 1, STR_VTRIM);

  for (uint8_t i = 0; i < NUM_TRIMS_KEYS; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + FH + FH * (i / 2);
    if (i & 1)
      lcdDraw1bitBitmap(24 * FW, y, sticks, i / 2, 0);
    displayKeyState(i & 1 ? 30 * FW : 28 * FW, y, TRM_BASE + i);
  }

  for (uint8_t i = 0; i <= KEY_MAX; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + FH * i;
    lcdDrawTextAtIndex(0, y, STR_VKEYS, (i), 0);
    displayKeyState(5 * FW + 2, y, i);
  }

  for (uint8_t i = 0, cnt = 0; i < NUM_SWITCHES; i++) {
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
  coord_t y = MENU_HEADER_HEIGHT + 1 + FH*KEY_COUNT;
  lcdDrawText(0, y, STR_ROTARY_ENCODER);
  lcdDrawNumber(5*FW+FWNUM+2, y, rotencValue / ROTARY_ENCODER_GRANULARITY, RIGHT);
#endif
}
