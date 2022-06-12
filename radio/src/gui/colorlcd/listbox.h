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

#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <set>

#include "bitmapbuffer.h"
#include "libopenui.h"
#include "touch.h"

// base class for lists of elements with names
class ListBase : public TableField
{
  std::function<void()> longPressHandler = nullptr;
  std::function<void()> pressHandler = nullptr;
  bool autoEdit = false;

 public:
  ListBase(Window* parent, const rect_t& rect, const std::vector<std::string>& names,
           uint8_t lineHeight = MENUS_LINE_HEIGHT, WindowFlags windowFlags = 0);

  bool getAutoEdit() const { return autoEdit; }
  void setAutoEdit(bool enable);

  void setName(uint16_t idx, const std::string& name);
  void setNames(const std::vector<std::string>& names);
  void setLineHeight(uint8_t height);

  virtual void setSelected(int selected);
  int getSelected() const;

  virtual void setActiveItem(int item);
  int getActiveItem() const;
  
  void setLongPressHandler(std::function<void()> handler)
  {
    longPressHandler = std::move(handler);
  }

  void setPressHandler(std::function<void()> handler)
  {
    pressHandler = std::move(handler);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ListBox"; }
#endif

 protected:
  static void event_cb(lv_event_t* e);
  int activeItem = -1;

  void onPress(uint16_t row, uint16_t col) override;
  void onLongPressed();

  void onClicked() override;
  void onCancel() override;

  void onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc) override;
};

class ListBox : public ListBase
{
  std::string title;

 public:
  using ListBase::ListBase;
  inline void setTitle(std::string title) { this->title = title; }
};
