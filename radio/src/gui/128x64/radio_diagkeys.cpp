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
#include "hal/rotary_encoder.h"
#include "hal/switch_driver.h"
#include "hal/key_driver.h"
#include "switches.h"

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

#if defined(FUNCTION_SWITCHES)
void menuRadioDiagFS(event_t event)
{
  constexpr coord_t FS_1ST_COLUMN = 40;
  constexpr coord_t FS_2ND_COLUMN = 70;
  constexpr coord_t FS_3RD_COLUMN = 100;
  SIMPLE_SUBMENU(STR_MENU_FSWITCH, 1);
  lcdDrawText(FS_1ST_COLUMN, MENU_HEADER_HEIGHT + 1, "Phys");
  lcdDrawText(FS_2ND_COLUMN, MENU_HEADER_HEIGHT + 1, "Log");
  lcdDrawText(FS_3RD_COLUMN, MENU_HEADER_HEIGHT + 1, "Led");

  for(uint8_t i=0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    coord_t y = 2*FH + i*FH;
    lcdDrawText(INDENT_WIDTH, y, STR_CHAR_SWITCH, 0);
    lcdDrawText(lcdNextPos, y, switchGetName(i+switchGetMaxSwitches()), 0);
    lcdDrawNumber(FS_1ST_COLUMN + 2, y, getFSPhysicalState(i));
    lcdDrawNumber(FS_2ND_COLUMN + 5, y, getFSLogicalState(i));
    lcdDrawNumber(FS_3RD_COLUMN + 5, y, getFSLedState(i));
  }
}
#endif

void menuRadioDiagKeys(event_t event)
{
  SIMPLE_SUBMENU(STR_MENU_RADIO_SWITCHES, 1);

  lcdDrawText(14*FW, MENU_HEADER_HEIGHT + 1, STR_VTRIM);

  for (uint8_t i = 0; i < 10; i++) {
    coord_t y;

    if (i < keysGetMaxTrims() * 2) {
      y = MENU_HEADER_HEIGHT + 1 + FH + FH * (i / 2);
      if (i & 1) lcdDraw1bitBitmap(14 * FW, y, sticks, i / 2, 0);
      displayTrimState(i & 1 ? 20 * FW : 18 * FW, y, i);
    }

    if (i < keysGetMaxKeys()) {
      auto k = get_ith_key(i);
      if (i >= 7) { // max 7 lines on display
        y = MENU_HEADER_HEIGHT + 1 + FH * 6;
        lcdDrawText(8, y, keysGetLabel(k), 0);
        displayKeyState(lcdNextPos + 10, y, i);
      }
      else {
        y = MENU_HEADER_HEIGHT + 1 + FH * i;
        lcdDrawText(0, y, keysGetLabel(k), 0);
        displayKeyState(5 * FW + 2, y, i);
      }
    }

    if (i < switchGetMaxSwitches()) {
      if (SWITCH_EXISTS(i)) {
        y = (i > 4) ? FH * (i - 4) + 1 : MENU_HEADER_HEIGHT + FH * i + 1;
        getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
        getvalue_t sw =
            ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
        drawSwitch(i > 4 ? 11 * FW - 5 : 8 * FW - 9, y, sw, 0, false);
      }
    }
  }

#if defined(AUTOSWITCH)
  lcdDrawText(13*FW+FWNUM, LCD_H - FH + 1,"Last");
  swsrc_t swtch = getMovedSwitch();
  if (swtch)
    drawSwitch(17*FW+FWNUM+2, LCD_H - FH + 1, swtch, 0);
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  coord_t y = LCD_H - FH + 1;
  lcdDrawText(8*FW-9, y, STR_ROTARY_ENCODER);
  lcdDrawNumber(12*FW+FWNUM+2, y, rotaryEncoderGetValue(), RIGHT);
#endif
}
