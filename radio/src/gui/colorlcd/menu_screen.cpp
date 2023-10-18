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

#include "menu_model.h"
#include "menu_radio.h"
#include "model_select.h"
#include "opentx.h"
#include "screen_setup.h"
#include "storage/storage.h"
#include "view_channels.h"
#include "view_main.h"

ScreenMenu::ScreenMenu(int8_t tabIdx) : TabsGroup(ICON_THEME)
{
  updateTabs(tabIdx);

  setCloseHandler([] {
    ViewMain::instance()->updateTopbarVisibility();
    storageDirty(EE_MODEL);
  });
}

void ScreenMenu::updateTabs(int8_t tabIdx)
{
  removeAllTabs();

  addTab(new ScreenUserInterfacePage(this));

  for (int index = 0; index < MAX_CUSTOM_SCREENS; index++) {
    if (customScreens[index]) {
      auto tab = new ScreenSetupPage(this, getTabs(), index);
      std::string title(STR_MAIN_VIEW_X);
      if (index >= 9) {
        title[title.size() - 2] = '1';
        title.back() = (index - 9) + '0';
      } else {
        title[title.size() - 2] = index + '1';
        title.back() = ' ';
      }
      tab->setTitle(title);
      tab->setIcon(ICON_THEME_VIEW1 + index);

      addTab(tab);
    } else {
      addTab(new ScreenAddPage(this, getTabs()));
      break;
    }
  }

  // set the active tab to the currently shown screen on the MainView
  auto viewMain = ViewMain::instance();
  auto tab = viewMain->getCurrentMainView() + 1;

  if (tabIdx >= 0) {
    tab = tabIdx;
  }

  setCurrentTab(tab);
}

void ScreenMenu::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_BREAK(KEY_MODEL)) {
    onCancel();
    new ModelMenu();
  } else if (event == EVT_KEY_LONG(KEY_MODEL)) {
    onCancel();
    killEvents(KEY_MODEL);
    new ModelLabelsWindow();
  } else if (event == EVT_KEY_BREAK(KEY_SYS)) {
    onCancel();
    new RadioMenu();
  } else if (event == EVT_KEY_LONG(KEY_SYS)) {
    onCancel();
    killEvents(KEY_SYS);
    // Radio setup
    (new RadioMenu())->setCurrentTab(2);
  } else if (event == EVT_KEY_LONG(KEY_TELE)) {
    onCancel();
    killEvents(KEY_TELE);
    new ChannelsViewMenu();
  } else {
    TabsGroup::onEvent(event);
  }
#endif
}
