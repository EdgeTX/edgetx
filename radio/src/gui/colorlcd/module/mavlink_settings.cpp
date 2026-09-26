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

#include "mavlink_settings.h"
#include "choice.h"
#include "edgetx.h"

#include "mixer_scheduler.h"
#include "pulses/mavlink.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

MavlinkSettings::MavlinkSettings(Window* parent, const FlexGridLayout& g,
                                 uint8_t moduleIdx) :
    Window(parent, rect_t{}),
    md(&g_model.moduleData[moduleIdx]),
    moduleIdx(moduleIdx)
{
  FlexGridLayout grid(g);
  setFlexLayout();

  // Baudrate
  {
    auto line = newLine(grid);
    new StaticText(line, rect_t{}, STR_BAUDRATE);
    new Choice(
        line, rect_t{}, STR_MAVLINK_BAUDRATE, 0, MAVLINK_BAUDRATE_COUNT - 1,
        [=]() -> int { return md->mavlink.telemetryBaudrate; },
        [=](int newValue) {
          md->mavlink.telemetryBaudrate = newValue;
          SET_DIRTY();
          restartModule(moduleIdx);
        });
  }

  // Status
  {
    auto line = newLine(grid);
    new StaticText(line, rect_t{}, STR_STATUS);
    new DynamicText(line, rect_t{}, [=] {
      char msg[32] = "";
      uint16_t period = getMixerSchedulerPeriod();
      uint16_t hz = period ? (1000000 / period) : 0;
      snprintf(msg, sizeof(msg), "%d Hz", hz);
      return std::string(msg);
    });
  }
}
