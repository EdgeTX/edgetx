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

#include "throttle_params.h"

#include "edgetx.h"
#include "getset_helpers.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "toggleswitch.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static SetupLineDef setupLines[] = {
  {
    // Throttle reversed
    STR_DEF(STR_THROTTLEREVERSE),
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(g_model.throttleReversed));
    }
  },
  {
    // Throttle source
    STR_DEF(STR_TTRACE),
    [](Window* parent, coord_t x, coord_t y) {
      auto sc = new SourceChoice(parent, {x, y, 0, 0},
                                []() { return g_model.thrTraceSrc; },
                                [](SourceRef ref) {
                                    g_model.thrTraceSrc = ref;
                                    SET_DIRTY();
                                });
      sc->setAvailableHandler([](SourceRef ref) {
        constexpr SourceTypeMask allowed =
            SRC_TYPE_BIT(SOURCE_TYPE_NONE) | SRC_TYPE_BIT(SOURCE_TYPE_STICK) |
            SRC_TYPE_BIT(SOURCE_TYPE_POT) | SRC_TYPE_BIT(SOURCE_TYPE_CHANNEL);
        return (allowed & SRC_TYPE_BIT(ref.type)) && isSourceAvailable(ref);
      });
    }
  },
  {
    // Throttle trim
    STR_DEF(STR_TTRIM),
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(g_model.thrTrim));
    }
  },
  {
    // Throttle trim source
    STR_DEF(STR_TTRIM_SW),
    [](Window* parent, coord_t x, coord_t y) {
      auto sc = new SourceChoice(
          parent, {x, y, 0, 0},
          []() { return g_model.getThrottleStickTrimSourceRef(); },
          [](SourceRef ref) {
            g_model.setThrottleStickTrimSourceRef(ref);
            SET_DIRTY();
          });
      sc->setAvailableHandler([](SourceRef ref) {
        return ref.type == SOURCE_TYPE_TRIM;
      });
    }
  },
};

ThrottleParams::ThrottleParams() : SubPage(ICON_MODEL_SETUP, STR_MAIN_MENU_MODEL_SETTINGS, STR_THROTTLE_LABEL, setupLines, DIM(setupLines))
{
}
