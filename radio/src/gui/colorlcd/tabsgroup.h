/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include "bitmaps.h"
#include "button.h"

class TabsGroup;
class TabsGroupHeader;
class TabsCarousel;

class PageTab
{
  friend class TabsCarousel;
  friend class TabsGroup;

 public:
  PageTab(PaddingSize padding = PAD_MEDIUM) : padding(padding) {}

  PageTab(std::string title, EdgeTxIcon icon,
          PaddingSize padding = PAD_MEDIUM) :
      title(std::move(title)), icon(icon), padding(padding)
  {
  }

  virtual bool isVisible() const { return true; }

  virtual ~PageTab() = default;

  virtual void build(Window* window) = 0;

  virtual void checkEvents() {}

  void setTitle(std::string value) { title = std::move(value); }

  void setIcon(EdgeTxIcon icon) { this->icon = icon; }
  EdgeTxIcon getIcon() const { return icon; }

  virtual void update(uint8_t index) {}
  virtual void cleanup() {}

 protected:
  std::string title;
  EdgeTxIcon icon;
  PaddingSize padding;
};

class TabsGroup : public NavWindow
{
  friend class TabsCarousel;

 public:
  explicit TabsGroup(EdgeTxIcon icon);

  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroup"; }
#endif

  uint8_t tabCount() const;

  void addTab(PageTab* page);

  void removeTab(unsigned index);

  void setCurrentTab(unsigned index);

  void checkEvents() override;

  void onClicked() override;
  void onCancel() override;

  static LAYOUT_VAL(MENU_TITLE_TOP, 48, 48)
  static LAYOUT_VAL(MENU_TITLE_HEIGHT, 21, 21)
  static constexpr coord_t MENU_BODY_TOP = MENU_TITLE_TOP + MENU_TITLE_HEIGHT;
  static constexpr coord_t MENU_BODY_HEIGHT = LCD_H - MENU_BODY_TOP;

 protected:
  TabsGroupHeader* header = nullptr;
  Window* body = nullptr;
  PageTab* currentTab = nullptr;

  void setVisibleTab(PageTab* tab);

#if defined(HARDWARE_KEYS)
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif

#if defined(PCBNV14) || defined(PCBPL18)
  void addGoToMonitorsButton(void);
#endif
};
