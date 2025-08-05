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

class HeaderIcon;
class PageGroup;
class PageGroupItem;

//-----------------------------------------------------------------------------

enum PageDefAction {
  PAGE_CREATE,
  PAGE_ACTION
};

struct PageDef {
  EdgeTxIcon icon;
  const char* qmTitle;
  const char* title;
  PageDefAction pageAction;
  QuickMenu::SubMenu subMenu;
  std::function<PageGroupItem*(PageDef& pageDef)> create;
  std::function<bool()> enabled;
  std::function<void(QuickSubMenu* subMenu)> action;
};

extern PageDef modelMenuItems[];
extern PageDef radioMenuItems[];
extern PageDef screensMenuItems[];
extern PageDef toolsMenuItems[];

//-----------------------------------------------------------------------------

class PageGroupItem
{
 public:
  PageGroupItem(std::string title, QuickMenu::SubMenu subMenu = QuickMenu::NONE) :
      title(std::move(title)), icon(ICON_EDGETX), quickMenuId(subMenu), padding(PAD_SMALL)
  {}

  PageGroupItem(PageDef& pageDef, PaddingSize padding = PAD_SMALL) :
      title(std::move(pageDef.title)), icon(pageDef.icon), quickMenuId(pageDef.subMenu),
      padding(padding)
  {}

  virtual ~PageGroupItem() = default;

  virtual bool isVisible() const { return true; }

  virtual void build(Window* window) = 0;

  virtual void checkEvents() {}

  void setTitle(std::string value) { title = std::move(value); }
  std::string getTitle() const { return title; }

  void setIcon(EdgeTxIcon icon) { this->icon = icon; }
  EdgeTxIcon getIcon() const { return icon; }

  PaddingSize getPadding() const { return padding; }

  virtual void update(uint8_t index) {}
  virtual void cleanup() {}

  QuickMenu::SubMenu subMenu() const { return quickMenuId; }

 protected:
  std::string title;
  EdgeTxIcon icon;
  QuickMenu::SubMenu quickMenuId = QuickMenu::NONE;
  PaddingSize padding;
};

//-----------------------------------------------------------------------------

class PageGroupHeaderBase : public Window
{
 public:
  PageGroupHeaderBase(Window* parent, coord_t height, EdgeTxIcon icon);

  void setTitle(const char* title);
  void setIcon(EdgeTxIcon newIcon);

  virtual void chgTab(int dir) = 0;

  void nextTab() { chgTab(1); }
  void prevTab() { chgTab(-1); }

  virtual void removeTab(unsigned index) {}
  void addTab(PageGroupItem* page);

  void setCurrentIndex(uint8_t index)
  {
    if (index < pages.size()) {
      currentIndex = index;
    }
  }

  PageGroupItem* pageTab(uint8_t idx) const { return pages[idx]; }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return pages.size(); }

 protected:
  uint8_t currentIndex = 0;
  lv_obj_t* titleLabel = nullptr;
  HeaderIcon* hdrIcon = nullptr;
  std::vector<PageGroupItem*> pages;
};

//-----------------------------------------------------------------------------

class PageGroupBase : public NavWindow
{
 public:
  PageGroupBase(coord_t bodyY, EdgeTxIcon icon);

  void setCurrentTab(unsigned index);

  void onClicked() override;
  void onCancel() override;

  uint8_t tabCount() const;

  void addTab(PageGroupItem* page);

 protected:
  PageGroupHeaderBase* header = nullptr;
  Window* body = nullptr;
  PageGroupItem* currentTab = nullptr;
  QuickMenu* quickMenu = nullptr;
  EdgeTxIcon icon;

  virtual void openMenu() = 0;

  void checkEvents() override;

  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(HARDWARE_KEYS)
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPGUP() override;
  void onPressPGDN() override;
  void onLongPressPGUP() override;
  void onLongPressPGDN() override;
  void onLongPressRTN() override;
#endif
};

//-----------------------------------------------------------------------------

class PageGroup : public PageGroupBase
{
 public:
  explicit PageGroup(EdgeTxIcon icon, PageDef* pages);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "PageGroup"; }
#endif

  void removeTab(unsigned index);

  PageGroupItem* getCurrentTab() const { return currentTab; }

  bool isPageGroup() override { return true; }

  static PageGroup* ScreenMenu();
  static PageGroup* RadioMenu();
  static PageGroup* ModelMenu();

  static LAYOUT_VAL_SCALED(MENU_TITLE_TOP, 45)
  static constexpr coord_t MENU_BODY_HEIGHT = LCD_H - MENU_TITLE_TOP;

 protected:

  void openMenu() override;
};

//-----------------------------------------------------------------------------

class TabsGroup : public PageGroupBase
{
 public:
  explicit TabsGroup(EdgeTxIcon icon, const char* name);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroup"; }
#endif

  static LAYOUT_ORIENTATION_SCALED(MENU_TITLE_TOP, 45, 48)
  static LAYOUT_ORIENTATION(MENU_TITLE_HEIGHT, 0, EdgeTxStyles::STD_FONT_HEIGHT)
  static constexpr coord_t MENU_BODY_TOP = MENU_TITLE_TOP + MENU_TITLE_HEIGHT;

 protected:

  void openMenu() override;
};
