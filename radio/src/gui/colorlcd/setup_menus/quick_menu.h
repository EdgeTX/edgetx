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
#include "quick_menu_group.h"

class PageGroup;
class PageGroupBase;
class ButtonBase;
class QuickSubMenu;
struct PageDef;

#define GRP_W(n,p) ((QuickMenuGroup::FAB_BUTTON_WIDTH + PAD_OUTLINE) * n - PAD_OUTLINE + PAD_OUTLINE * p)
#define GRP_H(n,p) ((QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_OUTLINE) * n - PAD_OUTLINE + PAD_OUTLINE * p)

class QuickMenu : public Window
{
 public:
  enum SubMenu {
    NONE = 0,
    // Model menu
    MODEL_SETUP,
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
    // Radio menu
    RADIO_SETUP,
    RADIO_GF,
    RADIO_TRAINER,
    RADIO_HARDWARE,
    RADIO_VERSION,
    // UI menu
    UI_THEMES,
    UI_SETUP,
    UI_SCREEN1,
    UI_SCREEN2,
    UI_SCREEN3,
    UI_SCREEN4,
    UI_SCREEN5,
    UI_SCREEN6,
    UI_SCREEN7,
    UI_SCREEN8,
    UI_SCREEN9,
    UI_SCREEN10,
    UI_ADD_PG,
    // Tools menu
    TOOLS_APPS,
    TOOLS_STORAGE,
    TOOLS_RESET,
    TOOLS_CHAN_MON,
    TOOLS_LS_MON,
    TOOLS_STATS,
    TOOLS_DEBUG,
  };

  QuickMenu(std::function<void()> cancelHandler,
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

  PageGroupBase* getPageGroup() const { return pageGroup; }
  QuickMenuGroup* getTopMenu() const { return mainMenu; }

  static constexpr int QM_MAIN_BTNS = 6;
  static constexpr int QM_SUB_BTNS = 12;
  static LAYOUT_ORIENTATION(QM_POPUP_W, GRP_W(6, 4), GRP_W(4, 4))
  static LAYOUT_ORIENTATION(QM_POPUP_H, GRP_W(3, 4), GRP_W(6, 4))
  static LAYOUT_ORIENTATION(QM_MAIN_W, GRP_W(6, 2), GRP_W(1, 2))
  static LAYOUT_ORIENTATION(QM_MAIN_H, GRP_W(1, 2), GRP_W(6, 2))
  static LAYOUT_ORIENTATION(QM_MAIN_FLOW, LV_FLEX_FLOW_ROW, LV_FLEX_FLOW_COLUMN)
  static LAYOUT_ORIENTATION(QM_SUB_X, GRP_W(0, 1), GRP_W(1, 1))
  static LAYOUT_ORIENTATION(QM_SUB_Y, GRP_W(1, 1), GRP_W(0, 1))
  static LAYOUT_ORIENTATION(QM_SUB_W, GRP_W(6, 2), GRP_W(3, 2))
  static LAYOUT_ORIENTATION(QM_SUB_H, GRP_W(2, 2), GRP_W(6, 2))

 protected:
  std::function<void()> cancelHandler = nullptr;
  std::function<void(bool close)> selectHandler = nullptr;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
  std::vector<QuickSubMenu*> subMenus;
  PageGroupBase* pageGroup = nullptr;
  SubMenu curPage;

  void onClicked() override;
};

class QuickSubMenu
{
 public:
  QuickSubMenu(Window* parent, QuickMenu* quickMenu,
               EdgeTxIcon icon, const char* title,
               PageDef* items):
    parent(parent), quickMenu(quickMenu),
    icon(icon), title(title), items(items)
  {}

  bool isSubMenu(QuickMenu::SubMenu n);
  int getIndex(QuickMenu::SubMenu n);

  ButtonBase* addButton();
  void enableSubMenu();
  void setDisabled(bool all);
  void setCurrent(QuickMenu::SubMenu n);
  void buildSubMenu();
  uint8_t onPress(int n);
  void onSelect(bool close);
  int getPageNumber(int iconNumber);

 protected:
  Window* parent;
  QuickMenu* quickMenu;
  EdgeTxIcon icon;
  const char* title;
  PageDef* items;
  QuickMenuGroup* subMenu = nullptr;
  ButtonBase* menuButton = nullptr;
};
