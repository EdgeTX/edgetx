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
#include "radio_diagkeys.h"
#include "libopenui.h"

#include "hal/rotary_encoder.h"

#if defined(PCBPL18)
static const uint8_t _trimMap[MAX_TRIMS * 2] = {8, 9, 10, 11, 12, 13, 14, 15, 2, 3, 4, 5, 0, 1, 6, 7};
#else
static const uint8_t _trimMap[MAX_TRIMS * 2] = {6, 7, 4, 5, 2, 3, 0, 1, 8, 9, 10, 11};
#endif

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

class RadioKeyDiagsWindow : public Window
{
  public:
    RadioKeyDiagsWindow(Window * parent, const rect_t &rect) :
      Window(parent, rect)
    {
    }

    void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void displayTrimState(BitmapBuffer * dc, coord_t x, coord_t y, uint8_t trim)
    {
      uint8_t t = keysGetTrimState(trim);
      // TODO use drawChar when done
      char status[2];
      status[0] = t + '0';
      status[1] = '\0';
      // TODO INVERS?
      dc->drawText(x, y, status, COLOR_THEME_PRIMARY1);
    }

    void displayKeyState(BitmapBuffer * dc, coord_t x, coord_t y, uint8_t key)
    {
      uint8_t t = keysGetState(key);
      // TODO use drawChar when done
      char status[2];
      status[0] = t + '0';
      status[1] = '\0';
      // TODO INVERS?
      dc->drawText(x, y, status, COLOR_THEME_PRIMARY1);
    }

    void paint(BitmapBuffer * dc) override
    {
      constexpr coord_t KEY_COLUMN = 6;
#if !defined(PCBNV14)
      constexpr coord_t SWITCHES_COLUMN = LCD_W / 2 - 20;
      constexpr coord_t TRIM_COLUMN = LCD_W - 120;
#else
      constexpr coord_t SWITCHES_COLUMN = LCD_W / 3;
      constexpr coord_t TRIM_COLUMN = 2 * LCD_W / 3;
#endif
      constexpr coord_t TRIM_MINUS_COLUMN = TRIM_COLUMN + 60;
      constexpr coord_t TRIM_PLUS_COLUMN = TRIM_MINUS_COLUMN + 20;

      dc->drawText(TRIM_COLUMN, 1, "Trims", COLOR_THEME_PRIMARY1);
      dc->drawText(TRIM_MINUS_COLUMN, 1, "-", COLOR_THEME_PRIMARY1);
      dc->drawText(TRIM_PLUS_COLUMN, 1, "+", COLOR_THEME_PRIMARY1);

      // KEYS
      coord_t y = 1;
      for (uint8_t i = 0; i < keysGetMaxKeys(); i++) {
        auto k = get_ith_key(i);
        dc->drawText(KEY_COLUMN, y, keysGetLabel(k), COLOR_THEME_PRIMARY1);
        displayKeyState(dc, 70, y, k);
        y += FH;
      }
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
      y += FH;
      dc->drawText(KEY_COLUMN, y, STR_ROTARY_ENCODER, COLOR_THEME_PRIMARY1);
      dc->drawNumber(70, y, rotaryEncoderGetValue(), COLOR_THEME_PRIMARY1);
#endif
      // SWITCHES
      y = 1;
      for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
        if (SWITCH_EXISTS(i)) {
          getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
          getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
          drawSwitch(dc, SWITCHES_COLUMN, y, sw, COLOR_THEME_PRIMARY1);
          y +=FH;
        }
      }

      // TRIMS
      for (uint8_t i = 0; i < keysGetMaxTrims() * 2; i++) {
        coord_t y = 1 + FH + FH * (i / 2);
        if (i & 1) {
#if defined(PCBPL18)
          dc->drawText(TRIM_COLUMN, y, "TR", COLOR_THEME_PRIMARY1);
          dc->drawNumber(TRIM_COLUMN + 20, y, i / 2 + 1, COLOR_THEME_PRIMARY1);
#else
          dc->drawText(TRIM_COLUMN, y, "T", COLOR_THEME_PRIMARY1);
          dc->drawNumber(TRIM_COLUMN + 10, y, i / 2 + 1, COLOR_THEME_PRIMARY1);
#endif
        }
        displayTrimState(dc, i & 1 ? TRIM_PLUS_COLUMN : TRIM_MINUS_COLUMN, y, _trimMap[i]);
      }
    };

  protected:
};

void RadioKeyDiagsPage::buildHeader(Window * window)
{
  header.setTitle(STR_RADIO_SETUP);
  header.setTitle2(STR_MENU_RADIO_SWITCHES);
}

void RadioKeyDiagsPage::buildBody(Window * window)
{
  new RadioKeyDiagsWindow(window, {0, 5, window->width() - 10, window->height() - 10});
}

RadioKeyDiagsPage::RadioKeyDiagsPage() :
  Page(ICON_MODEL_SETUP)
{
  buildHeader(&header);
  buildBody(&body);
  // setFocus(SET_FOCUS_DEFAULT);
}
