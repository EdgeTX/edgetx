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
#include "screen_setup.h"
#include "view_main.h"
#include "quick_menu.h"

PageDef screensMenuItems[] = {
  { ICON_RADIO_EDIT_THEME, STR_QM_THEMES, STR_MAIN_MENU_THEMES, PAGE_CREATE, QuickMenu::UI_THEMES, [](PageDef& pageDef) { return new ThemeSetupPage(pageDef); }, radioThemesEnabled},
  { ICON_THEME_SETUP, STR_QM_TOP_BAR, STR_USER_INTERFACE, PAGE_CREATE, QuickMenu::UI_SETUP, [](PageDef& pageDef) { return new ScreenUserInterfacePage(pageDef); }},
  { ICON_THEME_VIEW1, STR_QM_SCREEN_1, TR_MAIN_VIEW_X "1", PAGE_CREATE, QuickMenu::UI_SCREEN1, [](PageDef& pageDef) { return new ScreenSetupPage(0, pageDef); }},
  { ICON_THEME_VIEW2, STR_QM_SCREEN_2, TR_MAIN_VIEW_X "2", PAGE_CREATE, QuickMenu::UI_SCREEN2, [](PageDef& pageDef) { return new ScreenSetupPage(1, pageDef); }, []() { return customScreens[1] != nullptr; }},
  { ICON_THEME_VIEW3, STR_QM_SCREEN_3, TR_MAIN_VIEW_X "3", PAGE_CREATE, QuickMenu::UI_SCREEN3, [](PageDef& pageDef) { return new ScreenSetupPage(2, pageDef); }, []() { return customScreens[2] != nullptr; }},
  { ICON_THEME_VIEW4, STR_QM_SCREEN_4, TR_MAIN_VIEW_X "4", PAGE_CREATE, QuickMenu::UI_SCREEN4, [](PageDef& pageDef) { return new ScreenSetupPage(3, pageDef); }, []() { return customScreens[3] != nullptr; }},
  { ICON_THEME_VIEW5, STR_QM_SCREEN_5, TR_MAIN_VIEW_X "5", PAGE_CREATE, QuickMenu::UI_SCREEN5, [](PageDef& pageDef) { return new ScreenSetupPage(4, pageDef); }, []() { return customScreens[4] != nullptr; }},
  { ICON_THEME_VIEW6, STR_QM_SCREEN_6, TR_MAIN_VIEW_X "6", PAGE_CREATE, QuickMenu::UI_SCREEN6, [](PageDef& pageDef) { return new ScreenSetupPage(5, pageDef); }, []() { return customScreens[5] != nullptr; }},
  { ICON_THEME_VIEW7, STR_QM_SCREEN_7, TR_MAIN_VIEW_X "7", PAGE_CREATE, QuickMenu::UI_SCREEN7, [](PageDef& pageDef) { return new ScreenSetupPage(6, pageDef); }, []() { return customScreens[6] != nullptr; }},
  { ICON_THEME_VIEW8, STR_QM_SCREEN_8, TR_MAIN_VIEW_X "8", PAGE_CREATE, QuickMenu::UI_SCREEN8, [](PageDef& pageDef) { return new ScreenSetupPage(7, pageDef); }, []() { return customScreens[7] != nullptr; }},
  { ICON_THEME_VIEW9, STR_QM_SCREEN_9, TR_MAIN_VIEW_X "9", PAGE_CREATE, QuickMenu::UI_SCREEN9, [](PageDef& pageDef) { return new ScreenSetupPage(8, pageDef); }, []() { return customScreens[8] != nullptr; }},
  { ICON_THEME_VIEW10, STR_QM_SCREEN_10, TR_MAIN_VIEW_X "10", PAGE_CREATE, QuickMenu::UI_SCREEN10, [](PageDef& pageDef) { return new ScreenSetupPage(9, pageDef); }, []() { return customScreens[9] != nullptr; }},
  { ICON_THEME_ADD_VIEW, STR_QM_ADD_SCREEN, nullptr, PAGE_ACTION, QuickMenu::UI_ADD_PG, nullptr, []() { return customScreens[9] == nullptr; },
      [](QuickSubMenu* subMenu) {
        int newIdx = 1;
        for (; newIdx < MAX_CUSTOM_SCREENS; newIdx += 1)
        if (customScreens[newIdx] == nullptr)
          break;

        TRACE("Add screen: add screen: newIdx = %d", newIdx);

        auto& screen = customScreens[newIdx];
        auto& screenData = g_model.screenData[newIdx];

        const LayoutFactory* factory = defaultLayout;
        if (factory) {
          TRACE("Add screen: add screen: factory = %p", factory);

          auto viewMain = ViewMain::instance();
          screen = factory->create(viewMain, &screenData.layoutData);
          viewMain->addMainView(screen, newIdx);

          strncpy(screenData.LayoutId, factory->getId(),
                  sizeof(screenData.LayoutId));
          TRACE("Add screen: add screen: LayoutId = %s",
                screenData.LayoutId);

          subMenu->onPress(newIdx + ScreenSetupPage::FIRST_SCREEN_OFFSET);
        } else {
          TRACE("Add screen: factory is NULL");
        }
      }
  },
  { EDGETX_ICONS_COUNT }
};
