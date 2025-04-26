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

  static LAYOUT_VAL_SCALED(ICON_X1, 5)
  static LAYOUT_VAL_SCALED(ICON_X2, 38)
  static LAYOUT_VAL_SCALED(ICON_X3, 71)
  static LAYOUT_VAL_SCALED(ICON_Y, 5)
  static LAYOUT_VAL_SCALED(DATE_XO, 44)
  static LAYOUT_VAL_SCALED(CBT_Y, 44)
  static LAYOUT_VAL_SCALED(CBT_W, 100)
  static LAYOUT_VAL_SCALED(CB1_X, 100)
  static LAYOUT_VAL_SCALED(CB2_X, 154)
  static LAYOUT_VAL_SCALED(CB_Y, 40)
  static LAYOUT_VAL_SCALED(CB_W, 50)
  static LAYOUT_VAL_SCALED(BTN_X, 210)
  static LAYOUT_VAL_SCALED(BTN1_Y, 40)
  static LAYOUT_VAL_SCALED(BTN2_Y, 79)
  static LAYOUT_VAL_SCALED(BTN_W, 100)
  static LAYOUT_VAL_SCALED(TRIM_Y, 79)
  static LAYOUT_VAL_SCALED(SLIDER_Y, 101)
  static LAYOUT_VAL_SCALED(TXT1_Y, 122)
  static LAYOUT_VAL_SCALED(TXT2_Y, 144)
  static LAYOUT_VAL_SCALED(TXT_W, 100)
  static LAYOUT_VAL_SCALED(EDT2_X, 114)
  static LAYOUT_VAL_SCALED(EDT_Y, 170)

 protected:
  tmr10ms_t ticks;
};
