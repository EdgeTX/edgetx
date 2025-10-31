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

#include "edgetx.h"
#include "radio_hardware.h"
#include "radio_setup.h"
#include "radio_trainer.h"
#include "radio_version.h"
#include "special_functions.h"

PageDef radioMenuItems[] = {
  { ICON_RADIO_SETUP, STR_DEF(STR_QM_GEN_SETTINGS), STR_DEF(STR_MAIN_MENU_SETTINGS), PAGE_CREATE, QM_RADIO_SETUP, [](PageDef& pageDef) { return new RadioSetupPage(pageDef); }},
  { ICON_RADIO_GLOBAL_FUNCTIONS, STR_DEF(STR_QM_GLOB_FUNC), STR_DEF(STR_MENUSPECIALFUNCS), PAGE_CREATE, QM_RADIO_GF, [](PageDef& pageDef) { return new GlobalFunctionsPage(pageDef); }, radioGFEnabled},
  { ICON_RADIO_TRAINER, STR_DEF(STR_QM_TRAINER), STR_DEF(STR_MENUTRAINER), PAGE_CREATE, QM_RADIO_TRAINER, [](PageDef& pageDef) { return new RadioTrainerPage(pageDef); }, radioTrainerEnabled},
  { ICON_RADIO_HARDWARE, STR_DEF(STR_QM_HARDWARE), STR_DEF(STR_HARDWARE), PAGE_CREATE, QM_RADIO_HARDWARE, [](PageDef& pageDef) { return new RadioHardwarePage(pageDef); }},
  { ICON_RADIO_VERSION, STR_DEF(STR_QM_ABOUT), STR_DEF(STR_MAIN_MENU_ABOUT_EDGETX), PAGE_CREATE, QM_RADIO_VERSION, [](PageDef& pageDef) { return new RadioVersionPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};
