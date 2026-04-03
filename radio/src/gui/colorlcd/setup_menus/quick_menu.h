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

//-----------------------------------------------------------------------------

class PageGroupBase;
struct PageDef;
#if VERSION_MAJOR > 2
class QuickSubMenu;
#endif

//-----------------------------------------------------------------------------

enum QMMainDefAction {
  QM_SUBMENU,
  QM_ACTION
};

struct QMMainDef {
  EdgeTxIcon icon;
  STR_TYP qmTitle;
  STR_TYP title;
  QMMainDefAction pageAction;
  QMPage qmPage;
  const PageDef* subMenuItems;
  std::function<void()> action;
  std::function<bool()> enabled;

  bool isSubMenu(QMPage page) const;
  bool isSubMenu(QMPage page, EdgeTxIcon curIcon) const;
  int getIndex(QMPage page) const;
};

extern const QMMainDef qmTopItems[];

//-----------------------------------------------------------------------------

std::string replaceAll(std::string str, const std::string& from, const std::string& to);

//-----------------------------------------------------------------------------

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

  void setFocus(QMPage selection);

  void enableSubMenu();

  static void setCurrentPage(QMPage newPage, EdgeTxIcon newIcon = EDGETX_ICONS_COUNT);

  PageGroupBase* getPageGroup() const { return pageGroup; }
  QuickMenuGroup* getTopMenu() const { return mainMenu; }

  static void openQuickMenu();

  static void shutdownQuickMenu();
  static void openPage(QMPage page);
  static EdgeTxIcon subMenuIcon(QMPage page);
  static int pageIndex(QMPage page);
  static std::vector<std::string>& menuPageNames(bool forFavorites);

#if VERSION_MAJOR > 2
  static void resetFavorites();
#endif

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

#if VERSION_MAJOR == 2
  static LAYOUT_ORIENTATION(QM_MAIN_COLS, 5, 3)
  static LAYOUT_ORIENTATION(QM_MAIN_ROWS, 2, 4)
  static constexpr int QM_MAIN_W = GRP_W(QM_MAIN_COLS);
  static constexpr int QM_MAIN_H = GRP_H(QM_MAIN_ROWS);
  static constexpr coord_t QM_W = QM_MAIN_W + PAD_LARGE * 2;
  static constexpr coord_t QM_H = QM_MAIN_H + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM * 2;
  static constexpr coord_t QM_MAIN_X = (QM_W - QM_MAIN_W) / 2;
  static constexpr coord_t QM_MAIN_Y = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE;
  static constexpr int FIRST_SEARCH_IDX = 0;
#else
  static LAYOUT_ORIENTATION(QM_MAIN_COLS, 6, 1)
  static LAYOUT_ORIENTATION(QM_MAIN_ROWS, 1, 6)
  static LAYOUT_ORIENTATION(QM_SUB_COLS, 6, 3)
  static LAYOUT_ORIENTATION(QM_SUB_ROWS, 2, 6)
  static constexpr int QM_MAIN_W = GRP_W(QM_MAIN_COLS);
  static constexpr int QM_MAIN_H = GRP_H(QM_MAIN_ROWS);
  static constexpr int QM_SUB_W = GRP_W(QM_SUB_COLS);
  static constexpr int QM_SUB_H = GRP_H(QM_SUB_ROWS);
  static constexpr coord_t QM_W = LCD_W;
  static constexpr coord_t QM_H = LCD_H;
  static LAYOUT_ORIENTATION(QM_MAIN_X, (QM_W - QM_MAIN_W) / 2, (QM_W - QM_MAIN_W - QM_SUB_W - PAD_SMALL) / 2)
  static constexpr coord_t QM_MAIN_Y = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE;
  static LAYOUT_ORIENTATION(QM_SUB_X, QM_MAIN_X, QM_MAIN_X + QM_MAIN_W + PAD_SMALL)
  static LAYOUT_ORIENTATION(QM_SUB_Y, QM_MAIN_Y + QM_MAIN_H + PAD_LARGE, QM_MAIN_Y)
  static constexpr int FIRST_SEARCH_IDX = 1;  // Skip favorites
#endif
  static constexpr coord_t QM_X = (LCD_W - QM_W) / 2;
  static constexpr coord_t QM_Y = (LCD_H - QM_H) / 2;

 protected:
  static QuickMenu* instance;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
#if VERSION_MAJOR > 2
  std::vector<QuickSubMenu*> subMenus;
#endif
  PageGroupBase* pageGroup = nullptr;
  QMPage curPage = QM_NONE;
  EdgeTxIcon curIcon = EDGETX_ICONS_COUNT;

  void openQM(PageGroupBase* pageGroup, QMPage curPage);
  void closeQM();

  void focusMainMenu();

  void deleteLater() override;

  static void selected();

#if VERSION_MAJOR > 2
  void updateFavorites();
  bool setupFavorite(int fav, int favBtn);
#endif
};

//-----------------------------------------------------------------------------
