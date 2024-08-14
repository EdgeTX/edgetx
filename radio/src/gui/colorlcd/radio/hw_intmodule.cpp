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

#include "hw_intmodule.h"

#include "edgetx.h"

#if defined(CROSSFIRE)
#include "telemetry/crossfire.h"
#endif

#define SET_DIRTY() storageDirty(EE_GENERAL)

InternalModuleWindow::InternalModuleWindow(Window *parent, FlexGridLayout& grid)
{
  auto line = parent->newLine(grid);
  new StaticText(line, rect_t{}, STR_TYPE);
  auto internalModule =
      new Choice(line, rect_t{}, STR_MODULE_PROTOCOLS, MODULE_TYPE_NONE,
                 MODULE_TYPE_COUNT - 1, GET_DEFAULT(g_eeGeneral.internalModule),
                 [=](int type) { return setModuleType(type); });

  internalModule->setAvailableHandler(
      [](int module) { return isInternalModuleSupported(module); });

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  ant_box = parent->newLine(grid);
  new StaticText(ant_box, rect_t{}, STR_ANTENNA);
  new Choice(
      ant_box, rect_t{}, STR_ANTENNA_MODES, ANTENNA_MODE_INTERNAL,
      ANTENNA_MODE_EXTERNAL, GET_DEFAULT(g_eeGeneral.antennaMode),
      [](int antenna) {
        if (!isExternalAntennaEnabled() &&
            (antenna == ANTENNA_MODE_EXTERNAL ||
             (antenna == ANTENNA_MODE_PER_MODEL &&
              g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode ==
                  ANTENNA_MODE_EXTERNAL))) {
          if (confirmationDialog(STR_ANTENNACONFIRM1, STR_ANTENNACONFIRM2)) {
            g_eeGeneral.antennaMode = antenna;
            SET_DIRTY();
          }
        } else {
          g_eeGeneral.antennaMode = antenna;
          checkExternalAntenna();
          SET_DIRTY();
        }
      });

  updateAntennaLine();
#endif

#if defined(CROSSFIRE)
  br_box = parent->newLine(grid);
  new StaticText(br_box, rect_t{}, STR_BAUDRATE);
  new Choice(
      br_box, rect_t{}, STR_CRSF_BAUDRATE, 0, CROSSFIRE_MAX_INTERNAL_BAUDRATE,
      [=]() {
        return CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate);
      },
      [=](int val) {
        g_eeGeneral.internalModuleBaudrate = CROSSFIRE_INDEX_TO_STORE(val);
        restartModule(INTERNAL_MODULE);
        SET_DIRTY();
      });

  updateBaudrateLine();
#endif
}

void InternalModuleWindow::setModuleType(int moduleType)
{
  if (g_model.moduleData[INTERNAL_MODULE].type != moduleType) {
    memclear(&g_model.moduleData[INTERNAL_MODULE], sizeof(ModuleData));
    storageDirty(EE_MODEL);
  }
  g_eeGeneral.internalModule = moduleType;
  updateBaudrateLine();
  updateAntennaLine();
  SET_DIRTY();
}

void InternalModuleWindow::updateBaudrateLine()
{
#if defined(CROSSFIRE)
  br_box->show(isInternalModuleCrossfire());
#endif
}

void InternalModuleWindow::updateAntennaLine()
{
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  ant_box->show(isInternalModuleAvailable(MODULE_TYPE_XJT_PXX1));
#endif
}
