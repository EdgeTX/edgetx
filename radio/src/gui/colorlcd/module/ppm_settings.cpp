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

#include "ppm_settings.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

template <typename T>
PpmFrameSettings<T>::PpmFrameSettings(Window* parent, T* ppm) :
    Window(parent, rect_t{})
{
  padAll(PAD_TINY);
  setFlexLayout(LV_FLEX_FLOW_ROW);

  // PPM frame length
  auto edit = new NumberEdit(
      this, rect_t{0, 0, 80, 0}, 125, 35 * PPM_STEP_SIZE + PPM_DEF_PERIOD,
      GET_DEFAULT(ppm->frameLength * PPM_STEP_SIZE + PPM_DEF_PERIOD),
      SET_VALUE(ppm->frameLength, (newValue - PPM_DEF_PERIOD) / PPM_STEP_SIZE),
      PREC1);
  edit->setStep(PPM_STEP_SIZE);
  edit->setSuffix(STR_MS);

  // memorize PPM frame length NumberEdit object
  this->ppmFrameLenEditObject = edit;

  // PPM frame delay
  edit = new NumberEdit(this, rect_t{0, 0, 80, 0}, 100, 800,
                        GET_DEFAULT(ppm->delay * 50 + 300),
                        SET_VALUE(ppm->delay, (newValue - 300) / 50));
  edit->setStep(50);
  edit->setSuffix(STR_US);

  // PPM Polarity
  new Choice(this, rect_t{}, STR_PPM_POL, 0, 1, GET_SET_DEFAULT(ppm->pulsePol));
}

// explicit instantiation to make linker happy
template struct PpmFrameSettings<TrainerModuleData>;
template struct PpmFrameSettings<PpmModule>;
