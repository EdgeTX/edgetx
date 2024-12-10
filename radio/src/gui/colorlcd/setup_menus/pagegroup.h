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

#include "bitmaps.h"
#include "quick_menu.h"

class PageTab;
class PageGroupHeader;

//-----------------------------------------------------------------------------

struct PageDef {
  EdgeTxIcon icon;
  const char* title;
  QuickMenu::SubMenu subMenu;
  std::function<PageTab*(PageDef& pageDef)> createPage;
  std::function<bool()> enabled;
};

//-----------------------------------------------------------------------------

class PageGroup : public NavWindow
{
 public:
  explicit PageGroup(EdgeTxIcon icon, PageDef* pages);

  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "PageGroup"; }
#endif

  uint8_t tabCount() const;

  void addTab(PageTab* page);

  void removeTab(unsigned index);

  virtual void setCurrentTab(unsigned index);

  void checkEvents() override;

  void onClicked() override;
  void onCancel() override;

  void openMenu();

  static LAYOUT_VAL(MENU_TITLE_TOP, 45, 45)
  static constexpr coord_t MENU_BODY_HEIGHT = LCD_H - MENU_TITLE_TOP;

 protected:
  PageGroupHeader* header = nullptr;
  Window* body = nullptr;
  PageTab* currentTab = nullptr;
  QuickMenu* quickMenu = nullptr;
  EdgeTxIcon icon;

#if defined(HARDWARE_KEYS)
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif
};
