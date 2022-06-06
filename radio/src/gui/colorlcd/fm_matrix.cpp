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

#include "fm_matrix.h"
#include "opentx.h"

template<class T>
FMMatrix<T>::FMMatrix(Window* parent, const rect_t& r, T* input) :
    ButtonMatrix(parent, r), input(input)
{
  initBtnMap(3, MAX_FLIGHT_MODES);
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    setText(i, std::to_string(i).c_str());
  }
  update();

  lv_obj_set_width(lvobj, LV_DPI_DEF * 2);
  lv_obj_set_width(lvobj, LV_DPI_DEF);

  lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);
  // lv_obj_set_style_pad_left(lvobj, LV_DPI_DEF / 10, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);

  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_EDITED);  
}

template<class T>
void FMMatrix<T>::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_FLIGHT_MODES) return;
  BFBIT_FLIP(input->flightModes, bfBit<uint32_t>(btn_id));
  storageDirty(EE_MODEL);
}

template<class T>
bool FMMatrix<T>::isActive(uint8_t btn_id)
{
  if (btn_id >= MAX_FLIGHT_MODES) return false;
  return !bfSingleBitGet(input->flightModes, btn_id);
}

// explicit instantiation
template struct FMMatrix<ExpoData>;
template struct FMMatrix<MixData>;
