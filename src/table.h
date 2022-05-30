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

#include "form.h"

class TableField : public Window
{
 public:
  TableField(Window* parent, const rect_t& rect,
             WindowFlags windowFlags = OPAQUE | FORM_NO_BORDER);

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

  virtual void onDrawBegin(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc) {}
  virtual void onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc) {}

  static void event_cb(lv_event_t *e);
  static void clicked(lv_event_t *e);
  static void draw_begin(lv_event_t *e);
  static void draw_end(lv_event_t *e);
};
