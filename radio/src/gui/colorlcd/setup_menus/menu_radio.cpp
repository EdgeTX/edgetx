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

#include "menu_radio.h"

#include "edgetx.h"
#include "radio_hardware.h"
#include "radio_sdmanager.h"
#include "radio_setup.h"
#include "radio_theme.h"
#include "radio_tools.h"
#include "radio_trainer.h"
#include "radio_version.h"
#include "special_functions.h"

PageDef radioMenuItems[] = {
  { ICON_RADIO_TOOLS, STR_MENUTOOLS, QuickMenu::RADIO_TOOLSCRIPTS, [](PageDef& pageDef) { return new RadioToolsPage(pageDef); }},
  { ICON_RADIO_SD_MANAGER, STR_SD_CARD, QuickMenu::RADIO_SD, [](PageDef& pageDef) { return new RadioSdManagerPage(pageDef); }},
  { ICON_RADIO_SETUP, STR_RADIO_SETUP, QuickMenu::RADIO_SETUP, [](PageDef& pageDef) { return new RadioSetupPage(pageDef); }},
  { ICON_RADIO_EDIT_THEME, STR_THEME_EDITOR, QuickMenu::RADIO_THEMES, [](PageDef& pageDef) { return new ThemeSetupPage(pageDef); }, radioThemesEnabled},
  { ICON_RADIO_GLOBAL_FUNCTIONS, STR_MENUSPECIALFUNCS, QuickMenu::RADIO_GF, [](PageDef& pageDef) { return new GlobalFunctionsPage(pageDef); }, radioGFEnabled},
  { ICON_RADIO_TRAINER, STR_MENUTRAINER, QuickMenu::RADIO_TRAINER, [](PageDef& pageDef) { return new RadioTrainerPage(pageDef); }, radioTrainerEnabled},
  { ICON_RADIO_HARDWARE, STR_HARDWARE, QuickMenu::RADIO_HARDWARE, [](PageDef& pageDef) { return new RadioHardwarePage(pageDef); }},
  { ICON_RADIO_VERSION, STR_MENUVERSION, QuickMenu::RADIO_VERSION, [](PageDef& pageDef) { return new RadioVersionPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

RadioMenu::RadioMenu() : PageGroup(ICON_RADIO, radioMenuItems)
{
}

RadioMenu::~RadioMenu() { storageCheck(true); }
