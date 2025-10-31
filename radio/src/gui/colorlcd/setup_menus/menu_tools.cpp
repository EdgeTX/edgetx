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
#include "radio_sdmanager.h"
#include "radio_tools.h"
#include "view_statistics.h"
#include "view_logical_switches.h"
#include "view_channels.h"

PageDef toolsMenuItems[] = {
  { ICON_TOOLS_APPS, STR_DEF(STR_QM_APPS), STR_DEF(STR_MAIN_MENU_APPS), PAGE_CREATE, QuickMenu::TOOLS_APPS, [](PageDef& pageDef) { return new RadioToolsPage(pageDef); }},
  { ICON_RADIO_SD_MANAGER, STR_DEF(STR_QM_STORAGE), STR_DEF(STR_SD_CARD), PAGE_CREATE, QuickMenu::TOOLS_STORAGE, [](PageDef& pageDef) { return new RadioSdManagerPage(pageDef); }},
  { ICON_TOOLS_RESET, STR_DEF(STR_QM_RESET), nullptr, PAGE_ACTION, QuickMenu::TOOLS_RESET, nullptr, nullptr,
    []() {
      Menu* resetMenu = new Menu();
      resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
      resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
      resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
      resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
      resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
    }
  },
  { ICON_MONITOR, STR_DEF(STR_QM_CHAN_MON), nullptr, PAGE_ACTION, QuickMenu::TOOLS_CHAN_MON, nullptr, nullptr,
    []() {
      new ChannelsViewMenu();
    }
  },
  { ICON_MONITOR_LOGICAL_SWITCHES, STR_DEF(STR_QM_LS_MON), STR_DEF(STR_MONITOR_SWITCHES), PAGE_CREATE, QuickMenu::TOOLS_LS_MON, [](PageDef& pageDef) { return new LogicalSwitchesViewPage(pageDef); }},
  { ICON_STATS, STR_DEF(STR_QM_STATS), STR_DEF(STR_MAIN_MENU_STATISTICS), PAGE_CREATE, QuickMenu::TOOLS_STATS, [](PageDef& pageDef) { return new StatisticsViewPage(pageDef); }},
  { ICON_STATS_DEBUG, STR_DEF(STR_QM_DEBUG), STR_DEF(STR_DEBUG), PAGE_CREATE, QuickMenu::TOOLS_DEBUG, [](PageDef& pageDef) { return new DebugViewPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};
