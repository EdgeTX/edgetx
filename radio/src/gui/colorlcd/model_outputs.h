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

#include "tabsgroup.h"

class OutputLineButton;

class ModelOutputsPage : public PageTab
{
 public:
  ModelOutputsPage();
  void build(Window* window) override;

  static constexpr coord_t ADD_TRIMS_X = PAD_SMALL;
  static constexpr coord_t ADD_TRIMS_Y = PAD_SMALL;
  static LAYOUT_VAL(ADD_TRIMS_W, (LCD_W / 2) - 10, LCD_W - 12)
  static constexpr coord_t ADD_TRIMS_H = EdgeTxStyles::UI_ELEMENT_HEIGHT;
  static LAYOUT_VAL(EXLIM_XO, 58, 58)
  static constexpr coord_t EXLIMCB_X = LCD_W - EXLIM_XO;
  static LAYOUT_VAL(EXLIMCB_Y, 4, ADD_TRIMS_X + ADD_TRIMS_H + 2)
  static LAYOUT_VAL(EXLIMCB_W, 52, 52)
  static constexpr coord_t EXLIMCB_H = EdgeTxStyles::UI_ELEMENT_HEIGHT;
  static LAYOUT_VAL(EXLIM_X, ADD_TRIMS_X + ADD_TRIMS_W + PAD_SMALL, 6)
  static LAYOUT_VAL(EXLIM_Y, 10, ADD_TRIMS_X + ADD_TRIMS_H + 8)
  static constexpr coord_t EXLIM_W = EXLIMCB_X - EXLIM_X - PAD_SMALL;
  static LAYOUT_VAL(EXLIM_H, 20, 20)
  static constexpr coord_t TRIMB_X = PAD_SMALL;
  static constexpr coord_t TRIMB_Y = EXLIMCB_Y + EXLIMCB_H + PAD_MEDIUM / 2;
  static constexpr coord_t TRIMB_W = LCD_W - PAD_SMALL * 2;

 protected:
  void editOutput(uint8_t channel, OutputLineButton* btn);
};
