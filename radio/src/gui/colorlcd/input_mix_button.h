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

#include "list_line_button.h"
#include "opentx_types.h"

class InputMixButton : public ListLineButton
{
  lv_obj_t* fm_canvas = nullptr;
  void* fm_buffer = nullptr;
  uint16_t fm_modes = 0;

 public:
  InputMixButton(Window* parent, uint8_t index);
  ~InputMixButton();

 protected:
  lv_obj_t* weight;
  lv_obj_t* source;
  lv_obj_t* opts;

  void setWeight(gvar_t value, gvar_t min, gvar_t max);
  void setSource(mixsrc_t idx);
  void setFlightModes(uint16_t modes);
};
