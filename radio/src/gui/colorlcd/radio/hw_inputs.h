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

#include "button.h"
#include "dialog.h"
#include "form.h"
#include <vector>

class ToggleSwitch;

class HWSticks : public Window
{
 public:
  HWSticks(Window* parent);
};

struct HWPots : public Window
{
 public:
  HWPots(Window* parent);

  // Absolute layout for Pots popup - due to performance issues with lv_textarea
  // in a flex layout
  static LAYOUT_VAL(P_LBL_X, 0, 0)
  static LAYOUT_VAL(P_LBL_W, (DIALOG_DEFAULT_WIDTH - 45) * 2 / 11, (DIALOG_DEFAULT_WIDTH - 18) * 13 / 21)
  static constexpr coord_t P_NM_X = P_LBL_X + P_LBL_W + PAD_MEDIUM;
  static LAYOUT_VAL(P_NM_W, 70, 70)
  static LAYOUT_VAL(P_TYP_X, P_NM_X + P_NM_W, 0)
  static LAYOUT_VAL(P_TYP_W, 160, P_LBL_W)
  static constexpr coord_t P_INV_X = P_TYP_X + P_TYP_W + PAD_MEDIUM;
  static LAYOUT_VAL(P_INV_W, 52, 52)
  static LAYOUT_VAL(P_ROW_H, 36, 72)
  static LAYOUT_VAL(P_OFST_Y, 0, 36)
  #define P_Y(i) (i * P_ROW_H + 2)

 protected:
  bool potsChanged;
  std::vector<ToggleSwitch*> invertToggles;
};

class HWSwitches : public Window
{
 public:
  HWSwitches(Window* parent);
};

template <class T>
struct HWInputDialog : public BaseDialog {
  HWInputDialog(const char* title = nullptr);
};
