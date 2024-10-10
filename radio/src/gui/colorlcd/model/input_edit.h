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

#include "page.h"
#include "curve.h"
#include "choice.h"

struct ExpoData;

class InputEditWindow : public Page
{
 public:
  InputEditWindow(int8_t input, uint8_t index);

  static LAYOUT_VAL(INPUT_EDIT_CURVE_WIDTH, 140, 176)
  static LAYOUT_VAL(INPUT_EDIT_CURVE_HEIGHT, INPUT_EDIT_CURVE_WIDTH, 132)

 protected:
  uint8_t input;
  uint8_t index;
  Curve* preview = nullptr;
  bool updatePreview = false;
  getvalue_t lastWeightVal = 0;
  getvalue_t lastOffsetVal = 0;
  getvalue_t lastCurveVal = 0;
  uint8_t lastActiveIndex = 255;
  StaticText * headerSwitchName = nullptr;

  void setTitle();
  void buildBody(Window *window);

  void checkEvents() override;
};
