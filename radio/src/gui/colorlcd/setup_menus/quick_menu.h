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
#include "quick_menu_def.h"

class PageGroup;
class PageGroupBase;
class ButtonBase;
class QuickSubMenu;
struct PageDef;

#define GRP_W(n) ((QuickMenuGroup::QM_BUTTON_WIDTH + PAD_MEDIUM) * n - PAD_MEDIUM + PAD_OUTLINE * 2)
#if LANDSCAPE
#define GRP_H(n) ((QuickMenuGroup::QM_BUTTON_HEIGHT + PAD_MEDIUM) * n - PAD_MEDIUM + PAD_OUTLINE * 2)
#else
#define GRP_H(n) ((QuickMenuGroup::QM_BUTTON_HEIGHT + PAD_SMALL) * n - PAD_SMALL + PAD_OUTLINE * 2)
#endif

class QuickMenu : public NavWindow
{
 public:

  QuickMenu();

  void onCancel() override;
  void onSelect(bool close);
  void closeMenu();

  void setFocus(QMPage selection);

  void enableSubMenu();

  static void setCurrentPage(QMPage newPage, EdgeTxIcon newIcon = EDGETX_ICONS_COUNT);

  PageGroupBase* getPageGroup() const { return pageGroup; }
  QuickMenuGroup* getTopMenu() const { return mainMenu; }

  static QuickMenu* openQuickMenu(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler = nullptr,
            PageGroupBase* pageGroup = nullptr, QMPage curPage = QM_NONE);

  static void shutdownQuickMenu();
  static void selected();
  static void openPage(QMPage page);
  static EdgeTxIcon pageIcon(QMPage page);
  static int pageIndex(QMPage page);
  static std::vector<std::string> menuPageNames(bool forFavorites);
  static void setupFavorite(QMPage page, int f);

#if defined(HARDWARE_KEYS)
  void doKeyShortcut(event_t event);
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onLongPressRTN() override;
  void onPressPGDN() override;
  void onPressPGUP() override;
  void afterPG();
#endif

  static constexpr int QM_MAIN_BTNS = 6;
  static constexpr int QM_SUB_BTNS = 12;
  static LAYOUT_ORIENTATION(QM_MAIN_COLS, 6, 1)
  static LAYOUT_ORIENTATION(QM_MAIN_ROWS, 1, 6)
  static LAYOUT_ORIENTATION(QM_SUB_COLS, 6, 3)
  static LAYOUT_ORIENTATION(QM_SUB_ROWS, 2, 6)
  static constexpr int QM_MAIN_W = GRP_W(QM_MAIN_COLS);
  static constexpr int QM_MAIN_H = GRP_H(QM_MAIN_ROWS);
  static constexpr int QM_SUB_W = GRP_W(QM_SUB_COLS);
  static constexpr int QM_SUB_H = GRP_H(QM_SUB_ROWS);
  static LAYOUT_ORIENTATION(QM_MAIN_X, (LCD_W - QM_MAIN_W) / 2, (LCD_W - QM_MAIN_W - QM_SUB_W - PAD_SMALL) / 2)
  static constexpr coord_t QM_MAIN_Y = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY;
  static LAYOUT_ORIENTATION(QM_SUB_X, QM_MAIN_X, QM_MAIN_X + QM_MAIN_W + PAD_SMALL)
  static LAYOUT_ORIENTATION(QM_SUB_Y, QM_MAIN_Y + QM_MAIN_H + PAD_LARGE, QM_MAIN_Y)

 protected:
  static QuickMenu* instance;
  std::function<void()> cancelHandler = nullptr;
  std::function<void(bool close)> selectHandler = nullptr;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
  std::vector<QuickSubMenu*> subMenus;
  PageGroupBase* pageGroup = nullptr;
  static QMPage curPage;
  static EdgeTxIcon curIcon;

  void openQM(std::function<void()> cancelHandler,
              std::function<void(bool close)> selectHandler,
              PageGroupBase* pageGroup, QMPage curPage);

  void focusMainMenu();

  void deleteLater(bool detach = true, bool trash = true) override;
};
