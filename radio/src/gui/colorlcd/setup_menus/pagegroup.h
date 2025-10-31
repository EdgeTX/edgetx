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
class PageGroupItem;
class PageGroupBase;

//-----------------------------------------------------------------------------

enum PageDefAction {
  PAGE_CREATE,
  PAGE_ACTION
};

struct PageDef {
  EdgeTxIcon icon;
  STR_TYP qmTitle;
  STR_TYP title;
  PageDefAction pageAction;
  QMPage qmPage;
  std::function<PageGroupItem*(PageDef& pageDef)> create;
  std::function<bool()> enabled;
  std::function<void()> action;
};

enum QMTopDefAction {
  QM_SUBMENU,
  QM_ACTION
};

struct QMTopDef {
  EdgeTxIcon icon;
  const char* qmTitle;
  const char* title;
  QMTopDefAction pageAction;
  QMPage qmPage;
  PageDef* subMenuItems;
  std::function<uint8_t(void)> action;
};

extern QMTopDef qmTopItems[];

//-----------------------------------------------------------------------------

class PageGroupItem
{
 public:
  PageGroupItem(std::string title, QMPage qmPage = QM_NONE) :
      title(std::move(title)), icon(ICON_EDGETX), qmPageId(qmPage), padding(PAD_SMALL)
  {}

  PageGroupItem(PageDef& pageDef, PaddingSize padding = PAD_SMALL) :
      title(STR_VAL(pageDef.title)), icon(pageDef.icon), qmPageId(pageDef.qmPage),
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

  QMPage subMenu() const { return qmPageId; }

 protected:
  std::string title;
  EdgeTxIcon icon;
  QMPage qmPageId = QM_NONE;
  PaddingSize padding;
};

//-----------------------------------------------------------------------------

class PageGroupHeaderBase : public Window
{
 public:
  PageGroupHeaderBase(Window* parent, coord_t height, EdgeTxIcon icon, const char* parentTitle, PageGroupBase* menu);

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

  bool hasSubMenu(QMPage n);

  PageGroupItem* pageTab(uint8_t idx) const { return pages[idx]; }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return pages.size(); }

 protected:
  uint8_t currentIndex = 0;
  lv_obj_t* titleLabel = nullptr;
  lv_obj_t* parentLabel = nullptr;
  HeaderIcon* hdrIcon = nullptr;
  std::vector<PageGroupItem*> pages;
  PageGroupBase* menu;
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

  bool hasSubMenu(QMPage n);

  coord_t getScrollY();
  void setScrollY(coord_t y);

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
  void doKeyShortcut(event_t event);
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
  explicit PageGroup(EdgeTxIcon icon, const char* title, PageDef* pages);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "PageGroup"; }
#endif

  void removeTab(unsigned index);

  PageGroupItem* getCurrentTab() const { return currentTab; }

  bool isPageGroup() override { return true; }

  static LAYOUT_VAL_SCALED(PAGE_TOP_BAR_H, 45)

 protected:

  void openMenu() override;
};

//-----------------------------------------------------------------------------

class TabsGroup : public PageGroupBase
{
 public:
  explicit TabsGroup(EdgeTxIcon icon, const char* parentTitle);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroup"; }
#endif

  void hidePageButtons();

  static LAYOUT_ORIENTATION_SCALED(PAGE_TOP_BAR_H, 45, 48)
  static LAYOUT_ORIENTATION(PAGE_ALT_TITLE_H, 0, EdgeTxStyles::STD_FONT_HEIGHT)
  static constexpr coord_t TABS_GROUP_BODY_Y = PAGE_TOP_BAR_H + PAGE_ALT_TITLE_H;

 protected:

  void openMenu() override;
};
