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
#include "edgetx.h"
#include "screen_setup.h"
#include "storage/storage.h"
#include "view_channels.h"
#include "view_main.h"

ScreenMenu::ScreenMenu(int8_t tabIdx) : TabsGroup(ICON_THEME)
{
  addTab(new ScreenUserInterfacePage(this));

  for (int index = 0; index < MAX_CUSTOM_SCREENS; index++) {
    if (customScreens[index]) {
      addTab(new ScreenSetupPage(this, index));
    } else {
      addTab(new ScreenAddPage(this, tabCount()));
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

  setCloseHandler([] {
    ViewMain::instance()->updateTopbarVisibility();
    storageDirty(EE_MODEL);
  });
}

#if defined(HARDWARE_KEYS)
void ScreenMenu::onPressSYS()
{
  onCancel();
  new RadioMenu();
}
void ScreenMenu::onLongPressSYS()
{
  onCancel();
  // Radio setup
  (new RadioMenu())->setCurrentTab(2);
}
void ScreenMenu::onPressMDL()
{
  onCancel();
  new ModelMenu();
}
void ScreenMenu::onLongPressMDL()
{
  onCancel();
  new ModelLabelsWindow();
}
void ScreenMenu::onLongPressTELE()
{
  onCancel();
  new ChannelsViewMenu();
}
#endif
