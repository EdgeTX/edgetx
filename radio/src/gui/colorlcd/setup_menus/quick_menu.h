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

#pragma once

#include "window.h"
#include "bitmaps.h"
#include <vector>

class QuickMenuGroup;
class PageGroup;
class PageGroupBase;
class ButtonBase;
class QuickSubMenu;
struct PageDef;

class QuickMenu : public Window
{
 public:
  enum SubMenu {
    NONE = 0,
    MODEL_FIRST,
    MODEL_SETUP = MODEL_FIRST,
    MODEL_HELI,
    MODEL_FLIGHTMODES,
    MODEL_INPUTS,
    MODEL_MIXES,
    MODEL_OUTPUTS,
    MODEL_CURVES,
    MODEL_GVARS,
    MODEL_LS,
    MODEL_SF,
    MODEL_SCRIPTS,
    MODEL_TELEMETRY,
    MODEL_LAST = MODEL_TELEMETRY,
    RADIO_FIRST,
    RADIO_SETUP = RADIO_FIRST,
    RADIO_GF,
    RADIO_TRAINER,
    RADIO_HARDWARE,
    RADIO_VERSION,
    RADIO_LAST = RADIO_VERSION,
    SCREENS_FIRST,
    RADIO_THEMES = SCREENS_FIRST,
    SCREENS_UI_SETUP,
    SCREENS_PG1,
    SCREENS_PG2,
    SCREENS_PG3,
    SCREENS_PG4,
    SCREENS_PG5,
    SCREENS_PG6,
    SCREENS_PG7,
    SCREENS_PG8,
    SCREENS_PG9,
    SCREENS_PG10,
    SCREENS_ADD_PG,
    SCREENS_LAST = SCREENS_ADD_PG,
    TOOLS_FIRST,
    RADIO_TOOLSCRIPTS = TOOLS_FIRST,
    RADIO_SD,
    TOOLS_RESET,
    TOOLS_CHAN_MON,
    TOOLS_LS_MON,
    TOOLS_STATS,
    TOOLS_DEBUG,
    TOOLS_LAST = TOOLS_DEBUG,
  };

  QuickMenu(Window* parent, std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler = nullptr,
            PageGroupBase* pageGroup = nullptr, SubMenu curPage = NONE);

  void onCancel() override;
  void onSelect(bool close);
  void closeMenu();
  void deleteLater(bool detach = true, bool trash = true) override;
  void onEvent(event_t event) override;

  void setFocus(SubMenu selection);

  void enableSubMenu();

  SubMenu currentPage() const { return curPage; }

 protected:
  std::function<void()> cancelHandler = nullptr;
  std::function<void(bool close)> selectHandler = nullptr;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
  std::vector<QuickSubMenu*> subMenus;
  Window* box = nullptr;
  PageGroupBase* pageGroup = nullptr;
  SubMenu curPage;

  void buildMainMenu(int viewSubX, int viewSubY, int viewSubCols, int viewSubRows);

  void onClicked() override;
};

class QuickSubMenu
{
 public:
  QuickSubMenu(Window* parent, PageGroupBase* pageGroup, QuickMenu* quickMenu, QuickMenuGroup* topMenu,
               EdgeTxIcon icon, const char* title, QuickMenu::SubMenu first, QuickMenu::SubMenu last,
               std::function<PageGroup*()> create, PageDef* items,
               int viewSubX, int viewSubY, int viewSubCols, int viewSubRows) :
    parent(parent), pageGroup(pageGroup), quickMenu(quickMenu), topMenu(topMenu),
    icon(icon), title(title), first(first), last(last),
    create(std::move(create)), items(items),
    viewSubX(viewSubX), viewSubY(viewSubY),
    viewSubCols(viewSubCols), viewSubRows(viewSubRows)
  {}

  bool isSubMenu(QuickMenu::SubMenu n) { return (n >= first) && (n <= last); }

  ButtonBase* addButton();
  void enableSubMenu();
  void setDisabled(bool all);
  void setCurrent(int b);
  void buildSubMenu();
  uint8_t onPress(int n);
  void onSelect(bool close);
  int getPageNumber(int iconNumber);

 protected:
  Window* parent;
  PageGroupBase* pageGroup;
  QuickMenu* quickMenu;
  QuickMenuGroup* topMenu;
  EdgeTxIcon icon;
  const char* title;
  QuickMenu::SubMenu first;
  QuickMenu::SubMenu last;
  std::function<PageGroup*()> create;
  PageDef* items;
  QuickMenuGroup* subMenu = nullptr;
  ButtonBase* menuButton = nullptr;
  int viewSubX = 0;
  int viewSubY = 0;
  int viewSubCols = 0;
  int viewSubRows = 0;
};
