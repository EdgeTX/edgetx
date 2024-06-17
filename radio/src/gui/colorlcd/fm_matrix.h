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

#include "button_matrix.h"

struct ExpoData;
struct MixData;

template<class T>
struct FMMatrix : public ButtonMatrix {
  T* input;
  FMMatrix(Window* parent, const rect_t& rect, T* input);
  void onPress(uint8_t btn_id);
  bool isActive(uint8_t btn_id);
  void setTextAndState(uint8_t btn_id);

  static LAYOUT_VAL(FM_BTN_W, 48, 48)
  static LAYOUT_VAL(FM_COLS, 5, 3)
  static LAYOUT_VAL(FM_ROWS, 2, 3)
};

extern template struct FMMatrix<ExpoData>;
extern template struct FMMatrix<MixData>;
