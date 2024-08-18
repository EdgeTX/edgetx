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

#include "etx_lv_theme.h"
#include "window.h"

class FlexGridLayout
{
 public:
  FlexGridLayout(const lv_coord_t col_dsc[], const lv_coord_t row_dsc[],
                 PaddingSize padding = PAD_SMALL) :
      col_dsc(col_dsc), row_dsc(row_dsc), padding(padding)
  {
  }

  FlexGridLayout(const FlexGridLayout& g) :
      col_dsc(g.col_dsc), row_dsc(g.row_dsc), padding(g.padding)
  {
  }

  void apply(Window* w);

  void add(Window* w);

  void resetPos()
  {
    col_pos = 0;
    row_pos = 0;
  }

  void nextColumn() { col_pos += col_span; }
  void nextRow()
  {
    row_pos += row_span;
    col_pos = 0;
  }

  void nextCell()
  {
    nextColumn();
    if (col_dsc[col_pos] == LV_GRID_TEMPLATE_LAST) {
      nextRow();
    }
  }

  void setColSpan(uint8_t span) { col_span = span; }
  void setRowSpan(uint8_t span) { row_span = span; }

 protected:
  const lv_coord_t* col_dsc = nullptr;
  const lv_coord_t* row_dsc = nullptr;
  PaddingSize padding = PAD_ZERO;

  uint8_t col_pos = 0;
  uint8_t col_span = 1;
  uint8_t row_pos = 0;
  uint8_t row_span = 1;
};

class FormField : public Window
{
 public:
  FormField(Window* parent, const rect_t& rect, LvglCreate objConstruct = nullptr);

  virtual void changeEnd(bool forceChanged = false)
  {
    if (changeHandler) {
      changeHandler();
    }
  }

  void setChangeHandler(std::function<void()> handler)
  {
    changeHandler = std::move(handler);
  }

  inline bool isEditMode() const { return editMode; }
  virtual void setEditMode(bool newEditMode);

  void onClicked() override;
  void onCancel() override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  bool editMode = false;
  bool enabled = true;
  std::function<void()> changeHandler = nullptr;
};

class FormLine : public Window
{
 public:
  FormLine(Window* parent, FlexGridLayout& layout);

 protected:
  FlexGridLayout& layout;

  void addChild(Window* window) override;
};
