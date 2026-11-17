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

  static LAYOUT_VAL_SCALED(DATE_XO, 46)
#if LANDSCAPE_SMALL
  static constexpr coord_t CBT_X = 2;
  static constexpr coord_t CB1_X = 4;
  static constexpr coord_t CB2_X = 44;
  static constexpr coord_t CB_Y = 38;
  static constexpr coord_t CB_W = 38;
  static constexpr coord_t BTN_X = 108;
  static constexpr coord_t BTN1_Y = 38;
  static constexpr coord_t BTN2_Y = 65;
  static constexpr coord_t BTN_W = 80;
  static constexpr coord_t TRIM_X = 0;
  static constexpr coord_t TRIM_Y = 68;
  static constexpr coord_t SLIDER_Y = 85;
  static constexpr coord_t TXT1_Y = 100;
  static constexpr coord_t TXT2_Y = 118;
  static constexpr coord_t TXT_W = 80;
  static constexpr coord_t EDT_Y = 139;
  static constexpr coord_t EDT2_X = CBT_X;
  static constexpr coord_t EDT2_Y = 171;
  static constexpr coord_t EDY_W = 64;
  static constexpr coord_t QM_X = 76;
  static constexpr coord_t QM_Y = 104;
  static constexpr coord_t QM_W = 112;
  static constexpr coord_t QM_H = 82;
#else
  static LAYOUT_VAL_SCALED(CBT_X, 5)
  static LAYOUT_VAL_SCALED(CBT_Y, 54)
  static LAYOUT_VAL_SCALED(CBT_W, 100)
  static LAYOUT_VAL_SCALED(CB1_X, 100)
  static LAYOUT_VAL_SCALED(CB2_X, 154)
  static LAYOUT_VAL_SCALED(CB_Y, 50)
  static LAYOUT_VAL_SCALED(CB_W, 50)
  static LAYOUT_VAL_SCALED(BTN_X, 210)
  static LAYOUT_VAL_SCALED(BTN1_Y, 48)
  static LAYOUT_VAL_SCALED(BTN2_Y, 82)
  static LAYOUT_VAL_SCALED(BTN_W, 100)
  static constexpr coord_t TRIM_X = CBT_X;
  static LAYOUT_VAL_SCALED(TRIM_Y, 89)
  static LAYOUT_VAL_SCALED(SLIDER_Y, 111)
  static LAYOUT_VAL_SCALED(TXT1_Y, 132)
  static LAYOUT_VAL_SCALED(TXT2_Y, 154)
  static LAYOUT_VAL_SCALED(TXT_W, 100)
  static LAYOUT_VAL_SCALED(EDT_Y, 180)
  static LAYOUT_VAL_SCALED(EDT2_X, 94)
  static constexpr coord_t EDT2_Y = EDT_Y;
  static LAYOUT_VAL_SCALED(EDY_W, 80)
  static LAYOUT_VAL_SCALED(QM_X, 180)
  static LAYOUT_VAL_SCALED(QM_Y, 117)
  static LAYOUT_ORIENTATION_SCALED(QM_W, 140, 132)
  static LAYOUT_ORIENTATION_SCALED(QM_H, 104, 97)
#endif
};
