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

#include "edgetx.h"
#include "etx_lv_theme.h"

template <class T>
FMMatrix<T>::FMMatrix(Window* parent, const rect_t& r, T* input) :
    ButtonMatrix(parent, r), input(input)
{
  initBtnMap(FM_COLS, MAX_FLIGHT_MODES);

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    setTextAndState(i);
  }

  update();

  lv_obj_set_width(lvobj, FM_COLS * (FM_BTN_W + PAD_TINY) + PAD_TINY);
  lv_obj_set_height(lvobj, FM_ROWS * (EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY) + PAD_TINY);

  padAll(PAD_SMALL);
}

template <class T>
void FMMatrix<T>::setTextAndState(uint8_t btn_id)
{
  setText(btn_id, std::to_string(btn_id).c_str());
  setChecked(btn_id);
}

template <class T>
void FMMatrix<T>::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_FLIGHT_MODES) return;
  BFBIT_FLIP(input->flightModes, bfBit<uint32_t>(btn_id));
  setTextAndState(btn_id);
  storageDirty(EE_MODEL);
}

template <class T>
bool FMMatrix<T>::isActive(uint8_t btn_id)
{
  if (btn_id >= MAX_FLIGHT_MODES) return false;
  return !bfSingleBitGet(input->flightModes, btn_id);
}

// explicit instantiation
template struct FMMatrix<ExpoData>;
template struct FMMatrix<MixData>;
