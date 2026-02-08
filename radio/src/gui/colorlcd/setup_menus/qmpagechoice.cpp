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

#include "qmpagechoice.h"

#include "edgetx.h"
#include "menu.h"
#include "menutoolbar.h"

class QMPageChoiceMenuToolbar : public MenuToolbar
{
 public:
  QMPageChoiceMenuToolbar(Choice* choice, Menu* menu) :
      MenuToolbar(choice, menu, 3)
  {
    addButton(STR_MAIN_MENU_MODEL_SETTINGS, QM_MODEL_SETUP, QM_MODEL_NOTES, nullptr, STR_MAIN_MENU_MODEL_SETTINGS, true);
    addButton(STR_MAIN_MENU_RADIO_SETTINGS, QM_RADIO_SETUP, QM_RADIO_VERSION, nullptr, STR_MAIN_MENU_RADIO_SETTINGS, true);
    addButton(STR_MAIN_MENU_SCREEN_SETTINGS, QM_UI_THEMES, QM_UI_ADD_PG, nullptr, STR_MAIN_MENU_SCREEN_SETTINGS, true);
    addButton(STR_QM_TOOLS, QM_TOOLS_APPS, QM_TOOLS_DEBUG, nullptr, STR_QM_TOOLS, true);
    addButton(STR_MAIN_MENU_APPS, QM_APP, QM_APP+9999, nullptr, STR_MAIN_MENU_APPS, true);

    addButton(STR_SELECT_MENU_CLR, 0, 0, nullptr, nullptr, true);
  }

 protected:
};

QMPageChoice::QMPageChoice(Window* parent, const rect_t& rect,
              std::vector<std::string> values,
              int16_t vmin, int16_t vmax,
              std::function<int16_t()> getValue,
              std::function<void(int16_t)> setValue,
              const char *title) :
      Choice(parent, rect, values, vmin, vmax, getValue, setValue, title)
{
  setPopupWidth(TABLE_WIDTH);
}

void QMPageChoice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  auto menu = new Menu(false, popupWidth);
  if (menuTitle) menu->setTitle(menuTitle);

  auto tb = new QMPageChoiceMenuToolbar(this, menu);
  menu->setToolbar(tb);

  // fillMenu(menu); - called by MenuToolbar

  menu->setCloseHandler([=]() { setEditMode(false); });
}
