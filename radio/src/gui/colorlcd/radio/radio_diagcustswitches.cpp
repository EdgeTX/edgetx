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

#if defined(FUNCTION_SWITCHES)

#include "radio_diagcustswitches.h"

#include "hal/rgbleds.h"
#include "board.h"
#include "edgetx.h"

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
#include "color_list.h"
#include "hal/rgbleds.h"

uint16_t getLedColor(int i)
{
  // Convert RBG888 to RGB565
  uint32_t rgb32 = getFSLedRGBColor(i);
  uint8_t r = GET_RED32(rgb32);
  uint8_t g = GET_GREEN32(rgb32);
  uint8_t b = GET_BLUE32(rgb32);
  return RGB(r, g, b);
}
#endif

class RadioCustSwitchesDiagsWindow : public Window
{
  static LAYOUT_VAL_SCALED(FS_1ST_COLUMN, 95)
  static LAYOUT_VAL_SCALED(FS_2ND_COLUMN, 160)
  static LAYOUT_VAL_SCALED(FS_3RD_COLUMN, 260)
  static LAYOUT_VAL_SCALED(FS_LBL_WIDTH, 60)
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  static LAYOUT_VAL_SCALED(FS_COLOR_WIDTH, 30)
  static LAYOUT_VAL_SCALED(FS_COLOR_HEIGHT, 15)
  ColorSwatch* colorBox[NUM_FUNCTIONS_SWITCHES];
#endif

 public:
  RadioCustSwitchesDiagsWindow(Window *parent, const rect_t &rect) :
      Window(parent, rect)
  {
    new StaticText(this, {FS_1ST_COLUMN, PAD_SMALL, FS_LBL_WIDTH, LV_SIZE_CONTENT},
                   "Phys");
    new StaticText(this, {FS_2ND_COLUMN, PAD_SMALL, FS_LBL_WIDTH, LV_SIZE_CONTENT},
                   "Log");
    new StaticText(this, {FS_3RD_COLUMN, PAD_SMALL, FS_LBL_WIDTH, LV_SIZE_CONTENT},
                   "Led");
    for (uint8_t i = 0, r = 0; i < switchGetMaxSwitches(); i += 1) {
      if (switchIsCustomSwitch(i)) {
        coord_t y = (r + 2) * EdgeTxStyles::STD_FONT_HEIGHT;
          std::string s(CHAR_SWITCH);
          s += switchGetDefaultName(i);
          new StaticText(this, {PAD_LARGE, y, FS_LBL_WIDTH, LV_SIZE_CONTENT}, s);
        new DynamicText(
            this, {FS_1ST_COLUMN + PAD_LARGE, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
            [=]() {
              return getFSPhysicalState(i) ? CHAR_DOWN : CHAR_UP;
            });
        new DynamicText(
            this, {FS_2ND_COLUMN + 10, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
            [=]() { return g_model.cfsState(i) ? CHAR_DOWN : CHAR_UP; });

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
        colorBox[r] = new ColorSwatch(this, {FS_3RD_COLUMN, y, FS_COLOR_WIDTH,
                                             FS_COLOR_HEIGHT}, getLedColor(r));
#else
        new DynamicText(this,
                        {FS_3RD_COLUMN, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
                        [=]() { return STR_OFFON[getFSLedState(i)]; });
#endif
        r += 1;
      }
    }
  }

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  void checkEvents() override {
    Window::checkEvents();
    for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
      if (colorBox[i])
        colorBox[i]->setColor(getLedColor(i));
    }
  }
#endif
};

void RadioCustSwitchesDiagsPage::buildHeader(Window *window)
{
  header->setTitle(STR_HARDWARE);
  header->setTitle2(STR_FUNCTION_SWITCHES);
}

void RadioCustSwitchesDiagsPage::buildBody(Window *window)
{
  body->padAll(PAD_ZERO);
  new RadioCustSwitchesDiagsWindow(window,
                                   {0, 0, window->width(), window->height()});
}

RadioCustSwitchesDiagsPage::RadioCustSwitchesDiagsPage() :
    Page(ICON_MODEL_SETUP)
{
  buildHeader(header);
  buildBody(body);
}

#endif
