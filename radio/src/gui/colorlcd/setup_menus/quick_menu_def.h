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

enum QMPage {
  QM_NONE = 0,
  QM_OPEN_QUICK_MENU,
  QM_MANAGE_MODELS,
  // Model menu
  QM_MODEL_SETUP,
  QM_MODEL_FLIGHTMODES,
  QM_MODEL_INPUTS,
  QM_MODEL_MIXES,
  QM_MODEL_OUTPUTS,
  QM_MODEL_CURVES,
  QM_MODEL_GVARS,
  QM_MODEL_LS,
  QM_MODEL_SF,
  QM_MODEL_SCRIPTS,
  QM_MODEL_TELEMETRY,
  QM_MODEL_NOTES,
  // Radio menu
  QM_RADIO_SETUP,
  QM_RADIO_GF,
  QM_RADIO_TRAINER,
  QM_RADIO_HARDWARE,
  QM_RADIO_VERSION,
  // UI menu
  QM_UI_THEMES,
  QM_UI_SETUP,
  QM_UI_SCREEN1,
  QM_UI_SCREEN2,
  QM_UI_SCREEN3,
  QM_UI_SCREEN4,
  QM_UI_SCREEN5,
  QM_UI_SCREEN6,
  QM_UI_SCREEN7,
  QM_UI_SCREEN8,
  QM_UI_SCREEN9,
  QM_UI_SCREEN10,
  QM_UI_ADD_PG,
  // Tools menu
  QM_TOOLS_APPS,
  QM_TOOLS_STORAGE,
  QM_TOOLS_RESET,
  QM_TOOLS_CHAN_MON,
  QM_TOOLS_LS_MON,
  QM_TOOLS_STATS,
  QM_TOOLS_DEBUG,
};
