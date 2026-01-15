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
#include "menu.h"
#include "model_curves.h"
#include "model_flightmodes.h"
#include "model_gvars.h"
#include "model_heli.h"
#include "model_inputs.h"
#include "model_logical_switches.h"
#include "model_mixer_scripts.h"
#include "model_mixes.h"
#include "model_outputs.h"
#include "model_select.h"
#include "model_setup.h"
#include "model_telemetry.h"
#include "pagegroup.h"
#include "radio_hardware.h"
#include "radio_sdmanager.h"
#include "radio_setup.h"
#include "radio_tools.h"
#include "radio_trainer.h"
#include "radio_version.h"
#include "screen_setup.h"
#include "special_functions.h"
#include "view_channels.h"
#include "view_logical_switches.h"
#include "view_main.h"
#include "view_statistics.h"
#include "view_text.h"

const PageDef modelMenuItems[] = {
  { ICON_MODEL_SETUP, STR_DEF(STR_QM_MODEL_SETTINGS), STR_DEF(STR_MAIN_MODEL_SETTINGS), PAGE_CREATE, QM_MODEL_SETUP, [](const PageDef& pageDef) { return new ModelSetupPage(pageDef); }},
#if defined(FLIGHT_MODES)
  { ICON_MODEL_FLIGHT_MODES, STR_DEF(STR_QM_FLIGHT_MODES), STR_DEF(STR_MENUFLIGHTMODES), PAGE_CREATE, QM_MODEL_FLIGHTMODES, [](const PageDef& pageDef) { return new ModelFlightModesPage(pageDef); }, modelFMEnabled},
#endif
  { ICON_MODEL_INPUTS, STR_DEF(STR_QM_INPUTS), STR_DEF(STR_MENUINPUTS), PAGE_CREATE, QM_MODEL_INPUTS, [](const PageDef& pageDef) { return new ModelInputsPage(pageDef); }},
  { ICON_MODEL_MIXER, STR_DEF(STR_QM_MIXES), STR_DEF(STR_MIXES), PAGE_CREATE, QM_MODEL_MIXES, [](const PageDef& pageDef) { return new ModelMixesPage(pageDef); }},
  { ICON_MODEL_OUTPUTS, STR_DEF(STR_QM_OUTPUTS), STR_DEF(STR_MENULIMITS), PAGE_CREATE, QM_MODEL_OUTPUTS, [](const PageDef& pageDef) { return new ModelOutputsPage(pageDef); }},
  { ICON_MODEL_CURVES, STR_DEF(STR_QM_CURVES), STR_DEF(STR_MENUCURVES), PAGE_CREATE, QM_MODEL_CURVES, [](const PageDef& pageDef) { return new ModelCurvesPage(pageDef); }, modelCurvesEnabled},
#if defined(GVARS)
  { ICON_MODEL_GVARS, STR_DEF(STR_QM_GLOBAL_VARS), STR_DEF(STR_MENU_GLOBAL_VARS), PAGE_CREATE, QM_MODEL_GVARS, [](const PageDef& pageDef) { return new ModelGVarsPage(pageDef); }, modelGVEnabled},
#endif
  { ICON_MODEL_LOGICAL_SWITCHES, STR_DEF(STR_QM_LOGICAL_SW), STR_DEF(STR_MENULOGICALSWITCHES), PAGE_CREATE, QM_MODEL_LS, [](const PageDef& pageDef) { return new ModelLogicalSwitchesPage(pageDef); }, modelLSEnabled},
  { ICON_MODEL_SPECIAL_FUNCTIONS, STR_DEF(STR_QM_SPEC_FUNC), STR_DEF(STR_MENUCUSTOMFUNC), PAGE_CREATE, QM_MODEL_SF, [](const PageDef& pageDef) { return new SpecialFunctionsPage(pageDef); }, modelSFEnabled},
#if defined(LUA_MODEL_SCRIPTS)
  { ICON_MODEL_LUA_SCRIPTS, STR_DEF(STR_QM_CUSTOM_LUA), STR_DEF(STR_MENUCUSTOMSCRIPTS), PAGE_CREATE, QM_MODEL_SCRIPTS, [](const PageDef& pageDef) { return new ModelMixerScriptsPage(pageDef); }, modelCustomScriptsEnabled},
#endif
  { ICON_MODEL_TELEMETRY, STR_DEF(STR_QM_TELEM), STR_DEF(STR_MENUTELEMETRY), PAGE_CREATE, QM_MODEL_TELEMETRY, [](const PageDef& pageDef) { return new ModelTelemetryPage(pageDef); }, modelTelemetryEnabled},
  { ICON_MODEL_NOTES, STR_DEF(STR_MAIN_MENU_MODEL_NOTES), STR_DEF(STR_MAIN_MENU_MODEL_NOTES), PAGE_CREATE, QM_MODEL_NOTES, [](const PageDef& pageDef) { return new ModelNotesPage(pageDef); }, modelHasNotes},
  { EDGETX_ICONS_COUNT }
};

const PageDef radioMenuItems[] = {
#if VERSION_MAJOR == 2
  { ICON_TOOLS_APPS, STR_DEF(STR_QM_APPS), STR_DEF(STR_MAIN_MENU_APPS), PAGE_CREATE, QM_TOOLS_APPS, [](const PageDef& pageDef) { return new RadioToolsPage(pageDef); }},
  { ICON_RADIO_SD_MANAGER, STR_DEF(STR_QM_STORAGE), STR_DEF(STR_SD_CARD), PAGE_CREATE, QM_TOOLS_STORAGE, [](const PageDef& pageDef) { return new RadioSdManagerPage(pageDef); }},
#endif
  { ICON_RADIO_SETUP, STR_DEF(STR_QM_RADIO_SETTINGS), STR_DEF(STR_MAIN_RADIO_SETTINGS), PAGE_CREATE, QM_RADIO_SETUP, [](const PageDef& pageDef) { return new RadioSetupPage(pageDef); }},
#if VERSION_MAJOR == 2
  { ICON_RADIO_EDIT_THEME, STR_DEF(STR_QM_THEMES), STR_DEF(STR_MAIN_MENU_THEMES), PAGE_CREATE, QM_UI_THEMES, [](const PageDef& pageDef) { return new ThemeSetupPage(pageDef); }, radioThemesEnabled},
#endif
  { ICON_RADIO_GLOBAL_FUNCTIONS, STR_DEF(STR_QM_GLOB_FUNC), STR_DEF(STR_MENUSPECIALFUNCS), PAGE_CREATE, QM_RADIO_GF, [](const PageDef& pageDef) { return new GlobalFunctionsPage(pageDef); }, radioGFEnabled},
  { ICON_RADIO_TRAINER, STR_DEF(STR_QM_TRAINER), STR_DEF(STR_MENUTRAINER), PAGE_CREATE, QM_RADIO_TRAINER, [](const PageDef& pageDef) { return new RadioTrainerPage(pageDef); }, radioTrainerEnabled},
  { ICON_RADIO_HARDWARE, STR_DEF(STR_QM_HARDWARE), STR_DEF(STR_HARDWARE), PAGE_CREATE, QM_RADIO_HARDWARE, [](const PageDef& pageDef) { return new RadioHardwarePage(pageDef); }},
  { ICON_RADIO_VERSION, STR_DEF(STR_QM_ABOUT), STR_DEF(STR_MAIN_MENU_ABOUT_EDGETX), PAGE_CREATE, QM_RADIO_VERSION, [](const PageDef& pageDef) { return new RadioVersionPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

const PageDef screensMenuItems[] = {
#if VERSION_MAJOR > 2
  { ICON_RADIO_EDIT_THEME, STR_DEF(STR_QM_THEMES), STR_DEF(STR_MAIN_MENU_THEMES), PAGE_CREATE, QM_UI_THEMES, [](const PageDef& pageDef) { return new ThemeSetupPage(pageDef); }, radioThemesEnabled},
#endif
  { ICON_THEME_SETUP, STR_DEF(STR_QM_TOP_BAR), STR_DEF(STR_USER_INTERFACE), PAGE_CREATE, QM_UI_SETUP, [](const PageDef& pageDef) { return new ScreenUserInterfacePage(pageDef); }},
  { ICON_THEME_VIEW1, STR_DEF(STR_QM_SCREEN_1), STR_DEF(STR_MAIN_VIEW_1), PAGE_CREATE, QM_UI_SCREEN1, [](const PageDef& pageDef) { return new ScreenSetupPage(0, pageDef); }},
  { ICON_THEME_VIEW2, STR_DEF(STR_QM_SCREEN_2), STR_DEF(STR_MAIN_VIEW_2), PAGE_CREATE, QM_UI_SCREEN2, [](const PageDef& pageDef) { return new ScreenSetupPage(1, pageDef); }, []() { return customScreens[1] != nullptr; }},
  { ICON_THEME_VIEW3, STR_DEF(STR_QM_SCREEN_3), STR_DEF(STR_MAIN_VIEW_3), PAGE_CREATE, QM_UI_SCREEN3, [](const PageDef& pageDef) { return new ScreenSetupPage(2, pageDef); }, []() { return customScreens[2] != nullptr; }},
  { ICON_THEME_VIEW4, STR_DEF(STR_QM_SCREEN_4), STR_DEF(STR_MAIN_VIEW_4), PAGE_CREATE, QM_UI_SCREEN4, [](const PageDef& pageDef) { return new ScreenSetupPage(3, pageDef); }, []() { return customScreens[3] != nullptr; }},
  { ICON_THEME_VIEW5, STR_DEF(STR_QM_SCREEN_5), STR_DEF(STR_MAIN_VIEW_5), PAGE_CREATE, QM_UI_SCREEN5, [](const PageDef& pageDef) { return new ScreenSetupPage(4, pageDef); }, []() { return customScreens[4] != nullptr; }},
  { ICON_THEME_VIEW6, STR_DEF(STR_QM_SCREEN_6), STR_DEF(STR_MAIN_VIEW_6), PAGE_CREATE, QM_UI_SCREEN6, [](const PageDef& pageDef) { return new ScreenSetupPage(5, pageDef); }, []() { return customScreens[5] != nullptr; }},
  { ICON_THEME_VIEW7, STR_DEF(STR_QM_SCREEN_7), STR_DEF(STR_MAIN_VIEW_7), PAGE_CREATE, QM_UI_SCREEN7, [](const PageDef& pageDef) { return new ScreenSetupPage(6, pageDef); }, []() { return customScreens[6] != nullptr; }},
  { ICON_THEME_VIEW8, STR_DEF(STR_QM_SCREEN_8), STR_DEF(STR_MAIN_VIEW_8), PAGE_CREATE, QM_UI_SCREEN8, [](const PageDef& pageDef) { return new ScreenSetupPage(7, pageDef); }, []() { return customScreens[7] != nullptr; }},
  { ICON_THEME_VIEW9, STR_DEF(STR_QM_SCREEN_9), STR_DEF(STR_MAIN_VIEW_9), PAGE_CREATE, QM_UI_SCREEN9, [](const PageDef& pageDef) { return new ScreenSetupPage(8, pageDef); }, []() { return customScreens[8] != nullptr; }},
  { ICON_THEME_VIEW10, STR_DEF(STR_QM_SCREEN_10), STR_DEF(STR_MAIN_VIEW_10), PAGE_CREATE, QM_UI_SCREEN10, [](const PageDef& pageDef) { return new ScreenSetupPage(9, pageDef); }, []() { return customScreens[9] != nullptr; }},
#if VERSION_MAJOR == 2
  { ICON_THEME_ADD_VIEW, STR_DEF(STR_QM_ADD_SCREEN), STR_DEF(STR_QM_ADD_SCREEN), PAGE_CREATE, QM_UI_ADD_PG, [](const PageDef& pageDef) { return new ScreenAddPage(pageDef); }, []() { return customScreens[9] == nullptr; }},
#else
  { ICON_THEME_ADD_VIEW, STR_DEF(STR_QM_ADD_SCREEN), STR_DEF(STR_QM_ADD_SCREEN), PAGE_ACTION, QM_UI_ADD_PG, nullptr, []() { return customScreens[9] == nullptr; }, []() { ScreenSetupPage::addScreen(); } },
#endif
  { EDGETX_ICONS_COUNT }
};

#if VERSION_MAJOR == 2

const PageDef statsMenuItems[] = {
  { ICON_STATS, STR_DEF(STR_QM_STATS), STR_DEF(STR_MAIN_MENU_STATISTICS), PAGE_CREATE, QM_TOOLS_STATS, [](const PageDef& pageDef) { return new StatisticsViewPage(pageDef); }},
  { ICON_STATS_DEBUG, STR_DEF(STR_QM_DEBUG), STR_DEF(STR_DEBUG), PAGE_CREATE, QM_TOOLS_DEBUG, [](const PageDef& pageDef) { return new DebugViewPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

const QMTopDef qmTopItems[] = {
  { ICON_MODEL_SELECT, STR_DEF(STR_QM_MANAGE_MODELS), STR_DEF(STR_MANAGE_MODELS), QM_ACTION, QM_MANAGE_MODELS, nullptr, []() { new ModelLabelsWindow(); }},
  { ICON_MODEL_NOTES, STR_DEF(STR_MAIN_MENU_MODEL_NOTES), STR_DEF(STR_MAIN_MENU_MODEL_NOTES), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_MODEL_NOTES); }, modelHasNotes},
  { ICON_MONITOR, STR_DEF(STR_QM_CHAN_MON), STR_DEF(STR_QM_CHAN_MON), QM_ACTION, QM_TOOLS_CHAN_MON, nullptr, []() { new ChannelsViewMenu(); }
  },
  { ICON_MODEL_SETUP, STR_DEF(STR_QM_MODEL_SETTINGS), STR_DEF(STR_MAIN_MODEL_SETTINGS), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_MODEL_SETUP); }},
  { ICON_RADIO_SETUP, STR_DEF(STR_QM_RADIO_SETTINGS), STR_DEF(STR_MAIN_RADIO_SETTINGS), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_RADIO_SETUP); }},
  { ICON_THEME, STR_DEF(STR_MAIN_MENU_SCREEN_SETTINGS), STR_DEF(STR_MAIN_MENU_SCREEN_SETTINGS), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_UI_SCREEN1); }},
  { ICON_TOOLS_RESET, STR_DEF(STR_QM_RESET), STR_DEF(STR_QM_RESET), QM_ACTION, QM_TOOLS_RESET, nullptr,
    []() {
      Menu* resetMenu = new Menu();
      resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
      resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
      resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
      resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
      resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
    }
  },
  { ICON_STATS, STR_DEF(STR_QM_STATS), STR_DEF(STR_MAIN_MENU_STATISTICS), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_TOOLS_STATS); }},
  { ICON_RADIO_VERSION, STR_DEF(STR_QM_ABOUT), STR_DEF(STR_MAIN_MENU_ABOUT_EDGETX), QM_ACTION, QM_NONE, nullptr, []() { QuickMenu::openPage(QM_RADIO_VERSION); }},
  // Not displayed - search / run only
  { ICON_MODEL, STR_DEF(STR_QM_MODEL_SETUP), STR_DEF(STR_MAIN_MENU_MODEL_SETTINGS), QM_SUBMENU, QM_NONE, modelMenuItems},
  { ICON_RADIO, STR_DEF(STR_QM_RADIO_SETUP), STR_DEF(STR_MAIN_MENU_RADIO_SETTINGS), QM_SUBMENU, QM_NONE, radioMenuItems},
  { ICON_THEME, STR_DEF(STR_QM_UI_SETUP), STR_DEF(STR_MAIN_MENU_SCREEN_SETTINGS), QM_SUBMENU, QM_NONE, screensMenuItems},
  { ICON_STATS, STR_DEF(STR_QM_STATS), STR_DEF(STR_MAIN_MENU_STATISTICS), QM_SUBMENU, QM_NONE, statsMenuItems},
  { EDGETX_ICONS_COUNT }
};

#else

PageDef favoritesMenuItems[] = {
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT, nullptr, nullptr, PAGE_CREATE, QM_NONE, nullptr},
  { EDGETX_ICONS_COUNT }
};

const PageDef toolsMenuItems[] = {
  { ICON_TOOLS_APPS, STR_DEF(STR_QM_APPS), STR_DEF(STR_MAIN_MENU_APPS), PAGE_CREATE, QM_TOOLS_APPS, [](const PageDef& pageDef) { return new RadioToolsPage(pageDef); }},
  { ICON_RADIO_SD_MANAGER, STR_DEF(STR_QM_STORAGE), STR_DEF(STR_SD_CARD), PAGE_CREATE, QM_TOOLS_STORAGE, [](const PageDef& pageDef) { return new RadioSdManagerPage(pageDef); }},
  { ICON_TOOLS_RESET, STR_DEF(STR_QM_RESET), STR_DEF(STR_QM_RESET), PAGE_ACTION, QM_TOOLS_RESET, nullptr, nullptr,
    []() {
      Menu* resetMenu = new Menu();
      resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
      resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
      resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
      resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
      resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
    }
  },
  { ICON_MONITOR, STR_DEF(STR_QM_CHAN_MON), STR_DEF(STR_QM_CHAN_MON), PAGE_ACTION, QM_TOOLS_CHAN_MON, nullptr, nullptr, []() { new ChannelsViewMenu(); } },
  { ICON_MONITOR_LOGICAL_SWITCHES, STR_DEF(STR_QM_LS_MON), STR_DEF(STR_MONITOR_SWITCHES), PAGE_CREATE, QM_TOOLS_LS_MON, [](const PageDef& pageDef) { return new LogicalSwitchesViewPage(pageDef); }},
  { ICON_STATS, STR_DEF(STR_QM_STATS), STR_DEF(STR_MAIN_MENU_STATISTICS), PAGE_CREATE, QM_TOOLS_STATS, [](const PageDef& pageDef) { return new StatisticsViewPage(pageDef); }},
  { ICON_STATS_DEBUG, STR_DEF(STR_QM_DEBUG), STR_DEF(STR_DEBUG), PAGE_CREATE, QM_TOOLS_DEBUG, [](const PageDef& pageDef) { return new DebugViewPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

const QMTopDef qmTopItems[] = {
  { ICON_QM_FAVORITES, STR_DEF(STR_FAVORITE_LABEL), STR_DEF(STR_FAVORITE_LABEL), QM_SUBMENU, QM_NONE, favoritesMenuItems, nullptr, []() { return favoritesMenuItems[0].icon != EDGETX_ICONS_COUNT; }},
  { ICON_MODEL_SELECT, STR_DEF(STR_QM_MANAGE_MODELS), STR_DEF(STR_MANAGE_MODELS), QM_ACTION, QM_MANAGE_MODELS, nullptr, []() { new ModelLabelsWindow(); }},
  { ICON_MODEL, STR_DEF(STR_QM_MODEL_SETUP), STR_DEF(STR_MAIN_MENU_MODEL_SETTINGS), QM_SUBMENU, QM_NONE, modelMenuItems},
  { ICON_RADIO, STR_DEF(STR_QM_RADIO_SETUP), STR_DEF(STR_MAIN_MENU_RADIO_SETTINGS), QM_SUBMENU, QM_NONE, radioMenuItems},
  { ICON_THEME, STR_DEF(STR_QM_UI_SETUP), STR_DEF(STR_MAIN_MENU_SCREEN_SETTINGS), QM_SUBMENU, QM_NONE, screensMenuItems},
  { ICON_RADIO_TOOLS, STR_DEF(STR_QM_TOOLS), STR_DEF(STR_QM_TOOLS), QM_SUBMENU, QM_NONE, toolsMenuItems},
  { EDGETX_ICONS_COUNT }
};

#endif
