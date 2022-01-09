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

#if defined(KEYS_GPIO_PIN_PGUP)
constexpr uint8_t KEY_START = 0;
#else
constexpr uint8_t KEY_START = 1;
#endif

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

    void displayKeyState(BitmapBuffer * dc, coord_t x, coord_t y, uint8_t key)
    {
      uint8_t t = keys[key].state();
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
#if !defined(PCBNV14) && !defined(PCBPL18) // TODO! Remove PL18 after rotating the screen
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

#if !defined(PCBNV14) && !defined(PCBPL18) // TODO! Check if can be removed !defined(PCBPL18) here
      // KEYS
      for (uint8_t i = KEY_START; i <= 6; i++) {
        coord_t y = 1 + FH * (i - KEY_START);
        dc->drawTextAtIndex(KEY_COLUMN, y, STR_VKEYS, i, COLOR_THEME_PRIMARY1);
        displayKeyState(dc, 70, y, i);
      }
#if defined(ROTARY_ENCODER_NAVIGATION)
      coord_t y = FH * (8 - KEY_START);
      dc->drawText(KEY_COLUMN, y, STR_ROTARY_ENCODER, COLOR_THEME_PRIMARY1);
      dc->drawNumber(70, y, rotencValue, COLOR_THEME_PRIMARY1);
#endif
#else // defined(PCBNV14) || defined(PCBPL18)
      // KEYS (both radios do NOT have physical keys, only remapped trim keys)
      {
        coord_t y = 1;
        dc->drawTextAtIndex(KEY_COLUMN, y, STR_VKEYS, KEY_ENTER, COLOR_THEME_PRIMARY1);
        displayKeyState(dc, 70, y, KEY_ENTER);
        y += FH;
        dc->drawTextAtIndex(KEY_COLUMN, y, STR_VKEYS, KEY_EXIT, COLOR_THEME_PRIMARY1);
        displayKeyState(dc, 70, y, KEY_EXIT);
      }      
#endif
      // SWITCHES
      for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
        if (SWITCH_EXISTS(i)) {
          coord_t y = 1 + FH * i;
          getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
          getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
          drawSwitch(dc, SWITCHES_COLUMN, y, sw, COLOR_THEME_PRIMARY1);
        }
      }

      // TRIMS
      for (uint8_t i = 0; i < NUM_TRIMS_KEYS; i++) {
#if NUM_TRIMS_KEYS == 12
        const uint8_t trimMap[NUM_TRIMS_KEYS] = {6, 7, 4, 5, 2, 3, 0, 1, 8, 9, 10, 11};
#else
        const uint8_t trimMap[NUM_TRIMS_KEYS] = {6, 7, 4, 5, 2, 3, 0, 1};
#endif
        coord_t y = 1 + FH + FH * (i / 2);
        if (i & 1) {
          dc->drawText(TRIM_COLUMN, y, "T", COLOR_THEME_PRIMARY1);
          dc->drawNumber(TRIM_COLUMN + 10, y, i / 2 + 1, COLOR_THEME_PRIMARY1);
        }
        displayKeyState(dc, i & 1 ? TRIM_PLUS_COLUMN : TRIM_MINUS_COLUMN, y, TRM_BASE + trimMap[i]);
      }
    };

  protected:
};

void RadioKeyDiagsPage::buildHeader(Window * window)
{
  new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_MENU_RADIO_SWITCHES, 0, COLOR_THEME_PRIMARY2);
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
  setFocus(SET_FOCUS_DEFAULT);
}
