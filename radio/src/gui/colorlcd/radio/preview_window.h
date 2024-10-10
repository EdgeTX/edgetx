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

#pragma once

#include "window.h"
#include "theme_manager.h"

class HeaderDateTime;

// display controls using the appropriate theme.
class PreviewWindow : public Window
{
 public:
  PreviewWindow(Window *window, rect_t rect, std::vector<ColorEntry> colorList);

  void setColorList(std::vector<ColorEntry> colorList);

  static LAYOUT_VAL(ICON_X1, 5, 5)
  static LAYOUT_VAL(ICON_X2, 38, 38)
  static LAYOUT_VAL(ICON_X3, 71, 71)
  static LAYOUT_VAL(ICON_Y, 5, 5)
  static LAYOUT_VAL(DATE_XO, 44, 44)
  static LAYOUT_VAL(CBT_Y, 44, 44)
  static LAYOUT_VAL(CBT_W, 100, 100)
  static LAYOUT_VAL(CB1_X, 100, 100)
  static LAYOUT_VAL(CB2_X, 154, 154)
  static LAYOUT_VAL(CB_Y, 40, 40)
  static LAYOUT_VAL(CB_W, 50, 50)
  static LAYOUT_VAL(BTN_X, 210, 210)
  static LAYOUT_VAL(BTN1_Y, 40, 40)
  static LAYOUT_VAL(BTN2_Y, 79, 79)
  static LAYOUT_VAL(BTN_W, 100, 100)
  static LAYOUT_VAL(TRIM_Y, 79, 79)
  static LAYOUT_VAL(TRIM_H, 20, 20)
  static LAYOUT_VAL(SLIDER_Y, 101, 101)
  static LAYOUT_VAL(TXT1_Y, 122, 122)
  static LAYOUT_VAL(TXT2_Y, 144, 144)
  static LAYOUT_VAL(TXT_W, 100, 100)
  static LAYOUT_VAL(EDT2_X, 114, 114)
  static LAYOUT_VAL(EDT_Y, 170, 170)

 protected:
  tmr10ms_t ticks;
};
