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

#include "opentx.h"

void displayKeyState(uint8_t x, uint8_t y, uint8_t key)
{
  uint8_t t = keys[key].state();
  lcdDrawChar(x, y, t+'0', t ? INVERS : 0);
}

#if !defined(PCBTARANIS)
void displaySwitchState(uint8_t x, uint8_t y, uint8_t sw)
{
  swsrc_t t = switchState(sw);
  lcdDrawChar(x, y, (t ? '1' : '0'), t ? INVERS : 0);
}
#endif

void menuRadioDiagKeys(event_t event)
{
  SIMPLE_SUBMENU(STR_MENU_RADIO_SWITCHES, 1);

  lcdDrawText(14*FW, MENU_HEADER_HEIGHT + 1, STR_VTRIM);

  for (uint8_t i=0; i<9; i++) {
    coord_t y;

    if (i < NUM_TRIMS_KEYS) {
      y = MENU_HEADER_HEIGHT + 1 + FH + FH*(i/2);
      if (i&1) lcdDraw1bitBitmap(14*FW, y, sticks, i/2, 0);
      displayKeyState(i&1? 20*FW : 18*FW, y, TRM_BASE+i);
    }

    if (i <= KEY_MAX) {
      if (i == 7) { // T8 8th key???
        y = MENU_HEADER_HEIGHT + 1 + FH * 6;
        lcdDrawTextAtIndex(8, y, STR_VKEYS, i, 0);
        displayKeyState(lcdNextPos + 10, y, i);
      }
      else {
        y = MENU_HEADER_HEIGHT + 1 + FH * i;
        lcdDrawTextAtIndex(0, y, STR_VKEYS, i, 0);
        displayKeyState(5 * FW + 2, y, i);
      }
    }

#if defined(PCBSKY9X)
    if (i != SW_ID0) {
      y = MENU_HEADER_HEIGHT +i*FH - 2*FH;
      drawSwitch(8*FW, y, i+1, 0);
      displaySwitchState(11*FW+2, y, i);
    }
#else
    if (i < NUM_SWITCHES) {
      if (SWITCH_EXISTS(i)) {
        y = (NUM_SWITCHES > 6 ? 0 : MENU_HEADER_HEIGHT) + FH*i;
        getvalue_t val = getValue(MIXSRC_FIRST_SWITCH+i);
        getvalue_t sw = ((val < 0) ? 3*i+1 : ((val == 0) ? 3*i+2 : 3*i+3));
        drawSwitch(8*FW+4, y, sw, 0);
      }
    }
#endif
  }

#if defined(ROTARY_ENCODER_NAVIGATION)
  coord_t y = MENU_HEADER_HEIGHT + 1 + FH*KEY_COUNT;
  lcdDrawText(0, y, STR_ROTARY_ENCODER);
  lcdDrawNumber(5*FW+FWNUM+2, y, rotencValue / ROTARY_ENCODER_GRANULARITY, RIGHT);
#endif
}
