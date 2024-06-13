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

class RadioCustSwitchesDiagsWindow : public Window
{
  static constexpr coord_t FS_1ST_COLUMN = 95;
  static constexpr coord_t FS_2ND_COLUMN = 160;
  static constexpr coord_t FS_3RD_COLUMN = 260;

 public:
  RadioCustSwitchesDiagsWindow(Window *parent, const rect_t &rect) :
      Window(parent, rect)
  {
    new StaticText(this, {FS_1ST_COLUMN, PAD_SMALL, 60, LV_SIZE_CONTENT},
                   "Phys");
    new StaticText(this, {FS_2ND_COLUMN, PAD_SMALL, 60, LV_SIZE_CONTENT},
                   "Log");
    new StaticText(this, {FS_3RD_COLUMN, PAD_SMALL, 60, LV_SIZE_CONTENT},
                   "Led");
    for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
      coord_t y = 2 * EdgeTxStyles::PAGE_LINE_HEIGHT +
                  i * EdgeTxStyles::PAGE_LINE_HEIGHT;
      new StaticText(this, {10, y, LV_SIZE_CONTENT, LV_SIZE_CONTENT},
                     STR_CHAR_SWITCH);
      new StaticText(this, {25, y, LV_SIZE_CONTENT, LV_SIZE_CONTENT},
                     switchGetName(i + switchGetMaxSwitches()));
      new DynamicText(
          this, {FS_1ST_COLUMN + 10, y, LV_SIZE_CONTENT, LV_SIZE_CONTENT},
          [=]() {
            return getFSPhysicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP;
          });
      new DynamicText(
          this, {FS_2ND_COLUMN + 10, y, LV_SIZE_CONTENT, LV_SIZE_CONTENT},
          [=]() { return getFSLogicalState(i) ? STR_CHAR_DOWN : STR_CHAR_UP; });
      new DynamicText(this,
                      {FS_3RD_COLUMN + 5, y, LV_SIZE_CONTENT, LV_SIZE_CONTENT},
                      [=]() { return STR_OFFON[getFSLedState(i)]; });
    }
  }

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
