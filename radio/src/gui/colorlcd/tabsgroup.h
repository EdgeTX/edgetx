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
#include "button.h"

class TabsGroup;
class TabsGroupHeader;

class PageTab
{
 public:
  PageTab(PaddingSize padding = PAD_SMALL) : padding(padding) {}

  PageTab(std::string title, EdgeTxIcon icon,
          PaddingSize padding = PAD_SMALL) :
      title(std::move(title)), icon(icon), padding(padding)
  {
  }

  virtual ~PageTab() = default;

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

 protected:
  std::string title;
  EdgeTxIcon icon;
  PaddingSize padding;
};

class TabsGroup : public NavWindow
{
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

#if defined(HARDWARE_KEYS)
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif
};
