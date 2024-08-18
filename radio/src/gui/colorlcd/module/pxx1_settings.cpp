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

#include "pxx1_settings.h"
#include "choice.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

PXX1AntennaSettings::PXX1AntennaSettings(Window* parent,
                                         const FlexGridLayout& g,
                                         uint8_t moduleIdx) :
    Window(parent, rect_t{}), md(&g_model.moduleData[moduleIdx])
{
  FlexGridLayout grid(g);
  setFlexLayout();

  auto line = newLine(grid);
  new StaticText(line, rect_t{}, STR_ANTENNA);

  if (md->pxx.antennaMode == ANTENNA_MODE_PER_MODEL) {
    md->pxx.antennaMode = ANTENNA_MODE_INTERNAL;
    SET_DIRTY();
  }

  auto antennaChoice = new Choice(
      line, rect_t{}, STR_ANTENNA_MODES, ANTENNA_MODE_INTERNAL,
      ANTENNA_MODE_EXTERNAL, GET_DEFAULT(md->pxx.antennaMode),
      [=](int32_t antenna) -> void {
        if (!isExternalAntennaEnabled() && (antenna == ANTENNA_MODE_EXTERNAL)) {
          if (confirmationDialog(STR_ANTENNACONFIRM1, STR_ANTENNACONFIRM2)) {
            md->pxx.antennaMode = antenna;
            SET_DIRTY();
          }
        } else {
          md->pxx.antennaMode = antenna;
          SET_DIRTY();
          checkExternalAntenna();
        }
      });

  antennaChoice->setAvailableHandler(
      [=](int8_t mode) { return mode != ANTENNA_MODE_PER_MODEL; });
}
