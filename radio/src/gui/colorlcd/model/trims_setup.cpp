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

#include "trims_setup.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static SetupLineDef setupLines[] = {
  {
    // Reset trims
    nullptr,
    [](Window* parent, coord_t x, coord_t y) {
      new TextButton(parent, {PAD_TINY, y, LCD_W - PAD_MEDIUM * 2, 0}, STR_RESET_BTN, []() -> uint8_t {
        for (auto &fm : g_model.flightModeData) memclear(&fm.trim, sizeof(fm.trim));
        SET_DIRTY();
        AUDIO_WARNING1();
        return 0;
      });
    }
  },
#if defined(USE_HATS_AS_KEYS)
  {
    // Hats mode for NV14/EL18
    STR_HATSMODE,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, TrimsSetup::HATSMODE_W, 0}, STR_HATSOPT, HATSMODE_TRIMS_ONLY, HATSMODE_GLOBAL,
                GET_SET_DEFAULT(g_model.hatsMode));
      new TextButton(parent, {x + TrimsSetup::HATSMODE_W + PAD_SMALL, y, 0, 0}, "?", [=]() {
        new MessageDialog(STR_HATSMODE_KEYS, STR_HATSMODE_KEYS_HELP, "",
                          LEFT);
        return 0;
      });
    }
  },
#endif
  {
    // Trim step
    STR_TRIMINC,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_VTRIMINC, -2, 2,
                GET_SET_DEFAULT(g_model.trimInc));
    }
  },
  {
    // Extended trims
    STR_ETRIMS,
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(g_model.extendedTrims));
    }
  },
  {
    // Display trims
    // TODO: move to "Screen setup" ?
    STR_DISPLAY_TRIMS,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_VDISPLAYTRIMS, 0, 2,
                GET_SET_DEFAULT(g_model.displayTrims));
    }
  },
};

TrimsSetup::TrimsSetup() : SubPage(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, STR_TRIMS, setupLines, DIM(setupLines))
{
}
