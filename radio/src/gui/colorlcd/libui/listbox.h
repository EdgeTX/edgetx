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

#include <set>
#include <vector>

#include "table.h"

// Class for lists of elements with names
class ListBox : public TableField
{
  std::function<void()> pressHandler = nullptr;
  std::function<void(std::set<uint32_t>, std::set<uint32_t>)>
      _multiSelectHandler = nullptr;
  std::function<const char*(uint16_t row)> getSelectedSymbol = nullptr;

 public:
  ListBox(Window* parent, const rect_t& rect,
          const std::vector<std::string>& names,
          uint8_t lineHeight = MENUS_LINE_HEIGHT);

  void setName(uint16_t idx, const std::string& name);
  void setNames(const std::vector<std::string>& names);
  void setLineHeight(uint8_t height);

  virtual void setSelected(int selected, bool force = false);
  virtual void setSelected(std::set<uint32_t> selected);

  bool isRowSelected(uint16_t row);
  std::set<uint32_t> getSelection();

  void setMultiSelect(bool mode) { multiSelect = mode; }

  virtual void setActiveItem(int item);
  int getActiveItem() const;

  void setMultiSelectHandler(
      std::function<void(std::set<uint32_t>, std::set<uint32_t>)> handler)
  {
    _multiSelectHandler = std::move(handler);
  }

  void setGetSelectedSymbol(std::function<const char*(uint16_t)> handler)
  {
    getSelectedSymbol = std::move(handler);
  }

  void setPressHandler(std::function<void()> handler)
  {
    pressHandler = std::move(handler);
  }

  void setSmallSelectMarker() { smallSelectMarker = true; }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ListBox"; }
#endif

  static LAYOUT_VAL(MENUS_LINE_HEIGHT, 35, 35)

 protected:
  static void event_cb(lv_event_t* e);
  int activeItem = -1;
  bool multiSelect = false;
  bool smallSelectMarker = false;

  void onPress(uint16_t row, uint16_t col) override;

  void onClicked() override;
  void onCancel() override;

  void onDrawEnd(uint16_t row, uint16_t col,
                 lv_obj_draw_part_dsc_t* dsc) override;
};
