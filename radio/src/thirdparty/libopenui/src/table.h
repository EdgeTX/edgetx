/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "form.h"

class TableField : public Window
{
 public:
  TableField(Window* parent, const rect_t& rect,
             WindowFlags windowFlags = OPAQUE);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Table"; }
#endif

  void setColumnCount(uint16_t cols);
  uint16_t getColumnCount() const;
  
  void setRowCount(uint16_t rows);
  uint16_t getRowCount() const;
  
  void setColumnWidth(uint16_t col, coord_t w);
  coord_t getColumnWidth(uint16_t col) const;

  void select(uint16_t row, uint16_t col);
  virtual void onPress(uint16_t row, uint16_t col) {}

 protected:
  void adjustScroll();
  void selectNext(int16_t dir);
  void onEvent(event_t event) override;

  virtual void onSelected(uint16_t row, uint16_t col) {}
  virtual void onDrawBegin(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc) {}
  virtual void onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc) {}

  static void event_cb(lv_event_t *e);
  static void clicked(lv_event_t *e);
  static void draw_begin(lv_event_t *e);
  static void draw_end(lv_event_t *e);
};
