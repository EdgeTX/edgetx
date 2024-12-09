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

#include "menu_channels.h"

#include "edgetx.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "view_channels.h"
#include "view_logical_switches.h"

PageDef channelsMenuItems[] = {
  { ICON_MONITOR_CHANNELS1, STR_MONITOR_CHANNELS[0], QuickMenu::CHANNELS_PG1, [](PageDef& pageDef) { return new ChannelsViewPage(0, pageDef); }},
  { ICON_MONITOR_CHANNELS2, STR_MONITOR_CHANNELS[1], QuickMenu::CHANNELS_PG2, [](PageDef& pageDef) { return new ChannelsViewPage(1, pageDef); }},
  { ICON_MONITOR_CHANNELS3, STR_MONITOR_CHANNELS[2], QuickMenu::CHANNELS_PG3, [](PageDef& pageDef) { return new ChannelsViewPage(2, pageDef); }},
  { ICON_MONITOR_CHANNELS3, STR_MONITOR_CHANNELS[3], QuickMenu::CHANNELS_PG4, [](PageDef& pageDef) { return new ChannelsViewPage(3, pageDef); }},
  { ICON_MONITOR_LOGICAL_SWITCHES, STR_MONITOR_SWITCHES, QuickMenu::CHANNELS_LS, [](PageDef& pageDef) { return new LogicalSwitchesViewPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

ChannelsViewMenu::ChannelsViewMenu(ModelMenu* parent) :
    PageGroup(ICON_MONITOR, channelsMenuItems), parentMenu(parent)
{
}

#if defined(HARDWARE_KEYS)
void ChannelsViewMenu::onLongPressSYS()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  // Radio setup
  (new RadioMenu())->setCurrentTab(2);
}
void ChannelsViewMenu::onPressMDL()
{
  onCancel();
  if (!parentMenu) {
    new ModelMenu();
  }
}
void ChannelsViewMenu::onLongPressMDL()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  new ModelLabelsWindow();
}
void ChannelsViewMenu::onPressTELE()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  new ScreenMenu();
}
#endif
