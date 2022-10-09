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
#include "view_logical_switches.h"
#include "gridlayout.h"

#include <utility>

#if LCD_W > LCD_H // Landscape
  #define BTN_MATRIX_COL 8
#else // Portrait
  #define BTN_MATRIX_COL 4
#endif

class LogicalSwitchDisplayButton : public TextButton
{
  public:
   LogicalSwitchDisplayButton(FormGroup* parent, const rect_t& rect,
                              std::string text, unsigned index) :
       TextButton(parent, rect, std::move(text), nullptr, OPAQUE), index(index)
   {
   }

   void checkEvents() override
   {
     bool newvalue = getSwitch(SWSRC_SW1 + index);
     if (value != newvalue) {
       if (newvalue) {
         lv_obj_add_state(lvobj, LV_STATE_CHECKED);
       } else {
         lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
       }
       value = newvalue;
       invalidate();
     }
     Button::checkEvents();
   }

  protected:
    unsigned index = 0;
    bool value = false;
};

void LogicalSwitchesViewPage::build(FormWindow * window)
{
  constexpr coord_t LSW_VIEW_FOOTER_HEIGHT = 20;
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(BTN_MATRIX_COL);
  window->padAll(0);

  // Footer
  footer = new LogicalSwitchDisplayFooter(
      window, {0, window->height() - LSW_VIEW_FOOTER_HEIGHT, window->width(),
               LSW_VIEW_FOOTER_HEIGHT});

  // LSW table
  std::string lsString("LS64");
  lcdColorTable[CUSTOM_COLOR_INDEX] = RGB(160, 160, 160);
  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {

    strAppendSigned(&lsString[1], i + 1, 2);

    auto button = new LogicalSwitchDisplayButton(
        window, grid.getFieldSlot(BTN_MATRIX_COL, i % BTN_MATRIX_COL), lsString, i);

    button->setFocusHandler([=](bool focus) {
      if (focus) {
        footer->setIndex(i);
        footer->invalidate();
      }
      return 0;
    });
    if ((i + 1) % BTN_MATRIX_COL == 0) grid.nextLine();
  }
}
