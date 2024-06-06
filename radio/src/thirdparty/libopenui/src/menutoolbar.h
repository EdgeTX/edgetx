/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#pragma once

#include "button.h"
#include "choice.h"
#include "listbox.h"

class Menu;

class MenuToolbarButton : public ButtonBase
{
 public:
  MenuToolbarButton(Window* parent, const rect_t& rect, const char* picto);

 protected:
};

class MenuToolbar : public Window
{
  // friend Menu;

 public:
  MenuToolbar(Choice* choice, Menu* menu, const int columns);
  ~MenuToolbar();

  void resetFilter();
  void onEvent(event_t event) override;

  virtual void longPress() {}

  static LAYOUT_VAL(MENUS_TOOLBAR_BUTTON_WIDTH, 36, 36)
  static LAYOUT_VAL(MENUS_MAX_HEIGHT, ListBox::MENUS_LINE_HEIGHT * 7 + 8, ListBox::MENUS_LINE_HEIGHT * 10)

 protected:
  Choice* choice;
  Choice::FilterFct filter;
  Menu* menu;
  int nxtBtnPos = 0;
  int filterColumns = 0;
  MenuToolbarButton* allBtn = nullptr;

  lv_group_t* group;

  void addButton(const char* picto, int16_t filtermin, int16_t filtermax,
                 const Choice::FilterFct& filterFunc = nullptr,
                 const char* title = nullptr, bool wideButton = false);
  bool filterMenu(MenuToolbarButton* btn, int16_t filtermin, int16_t filtermax,
                  const Choice::FilterFct& filterFunc, const char* title);

  rect_t getButtonRect(bool wideButton);
};
