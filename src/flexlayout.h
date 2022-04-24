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

#include <lvgl/lvgl.h>

class FlexGridLayout
{
  const lv_coord_t* col_dsc = nullptr;
  const lv_coord_t* row_dsc = nullptr;
  lv_coord_t padding = 0;

  uint8_t col_pos = 0;
  uint8_t col_span = 1;
  uint8_t row_pos = 0;
  uint8_t row_span = 1;

 public:
  FlexGridLayout(const lv_coord_t col_dsc[], const lv_coord_t row_dsc[],
                 lv_coord_t padding = 4) :
      col_dsc(col_dsc), row_dsc(row_dsc), padding(padding)
  {
  }

  void apply(lv_obj_t* obj) {

    // padding
    lv_obj_set_style_pad_all(obj, padding, LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj, padding + PAGE_PADDING, LV_PART_MAIN);

    // layout
    lv_obj_set_layout(obj, LV_LAYOUT_GRID);
    if (col_dsc && row_dsc) {
      lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc);
    }
  }

  void add(lv_obj_t* obj) {
    if (col_dsc && row_dsc) {
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col_pos, col_span,
                             LV_GRID_ALIGN_CENTER, row_pos, row_span);
    }
  }

  void resetPos() { col_pos = 0; row_pos = 0; }

  void nextColumn() { col_pos += col_span; }
  void nextRow() { row_pos += row_span; }

  void nextCell()
  {
    nextColumn();
    if (col_dsc[col_pos] == LV_GRID_TEMPLATE_LAST) {
      nextRow();
      col_pos = 0;
    }
  }

  void setColSpan(uint8_t span) { col_span = span; }
  void setRowSpan(uint8_t span) { row_span = span; }
};
