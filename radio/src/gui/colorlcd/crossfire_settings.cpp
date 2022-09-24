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

#include "crossfire_settings.h"
#include "opentx.h"

#include "mixer_scheduler.h"

#include "telemetry/crossfire.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

CrossfireSettings::CrossfireSettings(Window* parent, const FlexGridLayout& g,
                                     uint8_t moduleIdx) :
    FormGroup(parent, rect_t{}), md(&g_model.moduleData[moduleIdx])
{
  FlexGridLayout grid(g);
  setFlexLayout();

  if (moduleIdx == EXTERNAL_MODULE) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_BAUDRATE, 0, COLOR_THEME_PRIMARY1);
    new Choice(
        line, rect_t{}, STR_CRSF_BAUDRATE, 0, CROSSFIRE_MAX_INTERNAL_BAUDRATE,
        [=]() -> int {
          return CROSSFIRE_STORE_TO_INDEX(md->crsf.telemetryBaudrate);
        },
        [=](int newValue) {
          md->crsf.telemetryBaudrate = CROSSFIRE_INDEX_TO_STORE(newValue);
          SET_DIRTY();
          restartModule(moduleIdx);
        });
  } else {
    //   char buf[6];
    //   new StaticText(this, grid.getFieldSlot(2, 1),
    //                  getStringAtIndex(buf, STR_CRSF_BAUDRATE,
    //                                   CROSSFIRE_STORE_TO_INDEX(
    //                                       g_eeGeneral.internalModuleBaudrate)),
    //                  0, COLOR_THEME_PRIMARY1);
  }

  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_STATUS, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(line, rect_t{}, [=] {
    char msg[64] = "";
    sprintf(msg, "%d Hz %" PRIu32 " Err", 1000000 / getMixerSchedulerPeriod(),
            telemetryErrors);
    return std::string(msg);
  });
}
