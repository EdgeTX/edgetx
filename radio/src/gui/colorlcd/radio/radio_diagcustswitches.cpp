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
#include "libopenui.h"
#include "edgetx.h"

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
#include "color_list.h"
#include "hal/rgbleds.h"
#endif

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
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
  static LAYOUT_VAL(FS_1ST_COLUMN, 95, 62, LS(95))
  static LAYOUT_VAL(FS_2ND_COLUMN, 160, 107, LS(160))
  static LAYOUT_VAL(FS_3RD_COLUMN, 260, 173, LS(260))
  static LAYOUT_VAL(FS_LBL_WIDTH, 60, 40, LS(60))
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  static LAYOUT_VAL(FS_COLOR_WIDTH, 30, 30, LS(30))
  static LAYOUT_VAL(FS_COLOR_HEIGHT, 15, 15, LS(15))
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
    for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
      coord_t y = (i + 2) * EdgeTxStyles::STD_FONT_HEIGHT;
      std::string s(STR_CHAR_SWITCH);
      s += switchGetName(i + switchGetMaxSwitches());
      new StaticText(this, {PAD_LARGE, y, FS_LBL_WIDTH, LV_SIZE_CONTENT}, s);
      new DynamicText(
          this, {FS_1ST_COLUMN + PAD_LARGE, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
          [=]() {
            return getFSPhysicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP;
          });
      new DynamicText(
          this, {FS_2ND_COLUMN + PAD_LARGE, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
          [=]() { return getFSLogicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP; });

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
      colorBox[i] = new ColorSwatch(this, {FS_3RD_COLUMN, y, FS_COLOR_WIDTH,
                                           FS_COLOR_HEIGHT}, getLedColor(i));
#else
      new DynamicText(this,
                {FS_3RD_COLUMN, y, FS_LBL_WIDTH, LV_SIZE_CONTENT},
                [=]() { return STR_OFFON[getFSLedState(i)]; });
#endif
    }
  }

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  void checkEvents() {
    Window::checkEvents();
    for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
      colorBox[i]->setColor(getLedColor(i));
    }
  }
#endif

 protected:
};

void RadioCustSwitchesDiagsPage::buildHeader(Window *window)
{
  header->setTitle(STR_RADIO_SETUP);
  header->setTitle2(STR_MENU_FSWITCH);
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
