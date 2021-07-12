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

#include "opentx.h"
#include "radio_diaganas.h"
#include "libopenui.h"

constexpr coord_t ANA_OFFSET = 150;

class RadioAnalogsDiagsWindow: public Window {
  public:
    RadioAnalogsDiagsWindow(Window * parent, const rect_t & rect):
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
      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
        coord_t y = 1 + (i / 2) * FH;
        uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
        dc->drawNumber(x, y, i + 1, LEADING0 | LEFT, 2);
        dc->drawText(x + 2 * 15 - 2, y, ":");
        drawHexNumber(dc, x + 3 * 15 - 1, y, anaIn(i));
        dc->drawNumber(x + ANA_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT);
      }

#if defined(HARDWARE_TOUCH)
      constexpr coord_t y = MENU_CONTENT_TOP + 5 * FH;

      if (touchState.event != TE_NONE && touchState.event != TE_SLIDE_END) {
        coord_t x = dc->drawText(MENUS_MARGIN_LEFT, y, STR_TOUCH_PANEL);
        x = dc->drawNumber(x + 5, y, touchState.x);
        x = dc->drawText(x, y, ":");
        dc->drawNumber(x, y, touchState.y);
        dc->drawLine(touchState.x - 10, touchState.y - 8 - parent->top(), touchState.x + 10, touchState.y + 8 - parent->top(), SOLID, 0);
        dc->drawLine(touchState.x - 10, touchState.y + 8 - parent->top(), touchState.x + 10, touchState.y - 8- parent->top(), SOLID, 0);
      }
#if !defined(SIMU)
      constexpr coord_t y1 = MENU_CONTENT_TOP + 6 * FH;
      constexpr coord_t x1 = MENUS_MARGIN_LEFT;
      dc->drawText(x1, y1, "Touch GT911 FW ver:");
      dc->drawNumber(x1 + 150, y1, touchGT911fwver, LEFT, 4);
      dc->drawText(x1 + 200, y1, "Hiccups:");
      dc->drawNumber(x1 + 260, y1, touchGT911hiccups, LEFT, 5);
#endif
#endif
    };

  protected:
};

void RadioAnalogsDiagsPage::buildHeader(Window * window)
{
  new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_MENU_RADIO_ANALOGS, 0, MENU_HIGHLIGHT_COLOR);
}

void RadioAnalogsDiagsPage::buildBody(Window * window)
{
  new RadioAnalogsDiagsWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}

RadioAnalogsDiagsPage::RadioAnalogsDiagsPage():
  Page(ICON_MODEL_SETUP)
{
  buildHeader(&header);
  buildBody(&body);
  setFocus(SET_FOCUS_DEFAULT);
}
