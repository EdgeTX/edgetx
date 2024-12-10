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

#include "menu_screen.h"

#include "edgetx.h"
#include "screen_setup.h"
#include "view_main.h"

PageDef screensMenuItems[] = {
  { ICON_THEME_SETUP, STR_USER_INTERFACE, QuickMenu::SCREENS_UI_SETUP, [](PageDef& pageDef) { return new ScreenUserInterfacePage(pageDef); }},
  { ICON_THEME_VIEW1, TR_MAIN_VIEW_X "1", QuickMenu::SCREENS_PG1, [](PageDef& pageDef) { return new ScreenSetupPage(0, pageDef); }},
  { ICON_THEME_VIEW2, TR_MAIN_VIEW_X "2", QuickMenu::SCREENS_PG2, [](PageDef& pageDef) { return new ScreenSetupPage(1, pageDef); }, []() { return customScreens[1] != nullptr; }},
  { ICON_THEME_VIEW3, TR_MAIN_VIEW_X "3", QuickMenu::SCREENS_PG3, [](PageDef& pageDef) { return new ScreenSetupPage(2, pageDef); }, []() { return customScreens[2] != nullptr; }},
  { ICON_THEME_VIEW4, TR_MAIN_VIEW_X "4", QuickMenu::SCREENS_PG4, [](PageDef& pageDef) { return new ScreenSetupPage(3, pageDef); }, []() { return customScreens[3] != nullptr; }},
  { ICON_THEME_VIEW5, TR_MAIN_VIEW_X "5", QuickMenu::SCREENS_PG5, [](PageDef& pageDef) { return new ScreenSetupPage(4, pageDef); }, []() { return customScreens[4] != nullptr; }},
  { ICON_THEME_VIEW6, TR_MAIN_VIEW_X "6", QuickMenu::SCREENS_PG6, [](PageDef& pageDef) { return new ScreenSetupPage(5, pageDef); }, []() { return customScreens[5] != nullptr; }},
  { ICON_THEME_VIEW7, TR_MAIN_VIEW_X "7", QuickMenu::SCREENS_PG7, [](PageDef& pageDef) { return new ScreenSetupPage(6, pageDef); }, []() { return customScreens[6] != nullptr; }},
  { ICON_THEME_VIEW8, TR_MAIN_VIEW_X "8", QuickMenu::SCREENS_PG8, [](PageDef& pageDef) { return new ScreenSetupPage(7, pageDef); }, []() { return customScreens[7] != nullptr; }},
  { ICON_THEME_VIEW9, TR_MAIN_VIEW_X "9", QuickMenu::SCREENS_PG9, [](PageDef& pageDef) { return new ScreenSetupPage(8, pageDef); }, []() { return customScreens[8] != nullptr; }},
  { ICON_THEME_VIEW10, TR_MAIN_VIEW_X "10", QuickMenu::SCREENS_PG10, [](PageDef& pageDef) { return new ScreenSetupPage(9, pageDef); }, []() { return customScreens[9] != nullptr; }},
  { ICON_THEME_ADD_VIEW, STR_ADD_MAIN_VIEW, QuickMenu::SCREENS_ADD_PG, [](PageDef& pageDef) { return new ScreenAddPage(pageDef); }, []() { return customScreens[9] == nullptr; }},
  { EDGETX_ICONS_COUNT }
};

ScreenMenu::ScreenMenu(int8_t tabIdx) : PageGroup(ICON_THEME, screensMenuItems)
{
  // set the active tab to the currently shown screen on the MainView
  auto viewMain = ViewMain::instance();
  auto tab = viewMain->getCurrentMainView() + 1;

  if (tabIdx >= 0) {
    tab = tabIdx;
  }

  setCurrentTab(tab);

  setCloseHandler([] {
    ViewMain::instance()->updateTopbarVisibility();
    storageDirty(EE_MODEL);
  });
}

void ScreenMenu::setCurrentTab(unsigned index)
{
  if (index > MAX_CUSTOM_SCREENS)
    index = tabCount() - 1;  
  PageGroup::setCurrentTab(index);
}
