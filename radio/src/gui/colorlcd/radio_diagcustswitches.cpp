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
#include "radio_diagcustswitches.h"
#include "libopenui.h"
#include "board.h"

#include "hal/rotary_encoder.h"

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

class RadioCustSwitchesDiagsWindow : public Window
{
public:
    RadioCustSwitchesDiagsWindow(Window * parent, const rect_t &rect) :
            Window(parent, rect)
    {
    }

    void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void paint(BitmapBuffer * dc) override
    {
      constexpr coord_t FS_1ST_COLUMN = 95;
      constexpr coord_t FS_2ND_COLUMN = 160;
      constexpr coord_t FS_3RD_COLUMN = 260;

      dc->drawText(FS_1ST_COLUMN, 1, "Phys", COLOR_THEME_PRIMARY1);
      dc->drawText(FS_2ND_COLUMN, 1, "Log", COLOR_THEME_PRIMARY1);
      dc->drawText(FS_3RD_COLUMN, 1, "Led", COLOR_THEME_PRIMARY1);

      for(uint8_t i=0; i < NUM_FUNCTIONS_SWITCHES; i++) {
        coord_t y = 2*FH + i*FH;
        dc->drawText(10, y, STR_CHAR_SWITCH, COLOR_THEME_PRIMARY1);
        dc->drawText(25, y, switchGetName(i+switchGetMaxSwitches()), COLOR_THEME_PRIMARY1);
        dc->drawText(FS_1ST_COLUMN + 10, y, getFSPhysicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP, COLOR_THEME_PRIMARY1);
        dc->drawText(FS_2ND_COLUMN + 8, y, getFSLogicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP, COLOR_THEME_PRIMARY1);
        dc->drawText(FS_3RD_COLUMN, y, STR_OFFON[getFSLedState(i)], COLOR_THEME_PRIMARY1);
      }
    };

protected:
};

void RadioCustSwitchesDiagsPage::buildHeader(Window * window)
{
  header.setTitle(STR_RADIO_SETUP);
  header.setTitle2(STR_MENU_FSWITCH);
}

void RadioCustSwitchesDiagsPage::buildBody(Window * window)
{
  new RadioCustSwitchesDiagsWindow(window, {0, 5, window->width() - 10, window->height() - 10});
}

RadioCustSwitchesDiagsPage::RadioCustSwitchesDiagsPage() :
        Page(ICON_MODEL_SETUP)
{
  buildHeader(&header);
  buildBody(&body);
  // setFocus(SET_FOCUS_DEFAULT);
}
