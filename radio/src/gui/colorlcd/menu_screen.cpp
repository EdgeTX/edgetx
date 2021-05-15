/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "menu_screen.h"
#include "screen_setup.h"
#include "view_main.h"
#include "storage/storage.h"

ScreenMenu::ScreenMenu():
  TabsGroup(ICON_THEME)
{
  updateTabs();

  setCloseHandler([]{
      ViewMain::instance()->updateTopbarVisibility();
      storageDirty(EE_MODEL);
  });
}

void ScreenMenu::updateTabs()
{
  removeAllTabs();

  addTab(new ScreenUserInterfacePage(this));

  for (int index = 0; index < MAX_CUSTOM_SCREENS; index++) {
    if (customScreens[index]) {
      auto tab = new ScreenSetupPage(this, getTabs(), index);
      std::string title(STR_MAIN_VIEW_X);
      title.back() = index + '1';
      tab->setTitle(title);
      tab->setIcon(ICON_THEME_VIEW1 + index);

      addTab(tab);
    }
    else {
      addTab(new ScreenAddPage(this, getTabs()));
      break;
    }
  }
}
