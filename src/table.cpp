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

#include "table.h"

void TableField::clicked(lv_event_t *e)
{
  lv_obj_t* target = lv_event_get_target(e);
  if (!target) return;

  uint16_t row;
  uint16_t col;
  lv_table_get_selected_cell(target, &row, &col);
    
  TableField* tf = (TableField*)lv_obj_get_user_data(target);
  if (!tf) return;

  tf->onPress(row, col);
}

void TableField::draw_begin(lv_event_t* e)
{
  lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
  if (dsc->part != LV_PART_ITEMS) return;

  lv_obj_t* table = lv_event_get_target(e);
  if (!table) return;

  uint16_t cols = lv_table_get_col_cnt(table);
  if (cols == 0) return;
  
  TableField* tf = (TableField*)lv_obj_get_user_data(table);
  if (!tf) return;

  uint16_t row = dsc->id / cols;
  uint16_t col = dsc->id % cols;
  tf->onDrawBegin(row, col, dsc);
}

void TableField::draw_end(lv_event_t* e)
{
  lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
  if (dsc->part != LV_PART_ITEMS) return;

  lv_obj_t* table = lv_event_get_target(e);
  if (!table) return;

  uint16_t cols = lv_table_get_col_cnt(table);
  if (cols == 0) return;
  
  TableField* tf = (TableField*)lv_obj_get_user_data(table);
  if (!tf) return;

  uint16_t row = dsc->id / cols;
  uint16_t col = dsc->id % cols;
  tf->onDrawEnd(row, col, dsc);
}

TableField::TableField(Window* parent, const rect_t& rect, WindowFlags windowFlags) :
  FormField(parent, rect, windowFlags, 0, lv_table_create)
{
  lv_obj_add_event_cb(lvobj, TableField::clicked, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(lvobj, TableField::draw_begin, LV_EVENT_DRAW_PART_BEGIN, nullptr);
  lv_obj_add_event_cb(lvobj, TableField::draw_end, LV_EVENT_DRAW_PART_END, nullptr);
}

void TableField::setColumnCount(uint16_t cols)
{
  lv_table_set_col_cnt(lvobj, cols);
}

uint16_t TableField::getColumnCount() const
{
  return lv_table_get_col_cnt(lvobj);
}
  
void TableField::setRowCount(uint16_t rows)
{
  lv_table_set_row_cnt(lvobj, rows);
}

uint16_t TableField::getRowCount() const
{
  return lv_table_get_row_cnt(lvobj);
}
  
void TableField::setColumnWidth(uint16_t col, coord_t w)
{
  lv_table_set_col_width(lvobj, col, w);
}

coord_t TableField::getColumnWidth(uint16_t col) const
{
  return lv_table_get_col_width(lvobj, col);
}

void TableField::select(uint16_t row, uint16_t col)
{
  lv_table_t* table = (lv_table_t*)lvobj;
  if (table->row_act == row && table->col_act == row) return;

  if (row >= table->row_cnt || col >= table->col_cnt) {    
    table->col_act = LV_TABLE_CELL_NONE;
    table->row_act = LV_TABLE_CELL_NONE;
    return;
  }

  table->row_act = row;
  table->col_act = col;

  lv_obj_invalidate(lvobj);
  adjustScroll();
}

void TableField::adjustScroll()
{
  lv_table_t* table = (lv_table_t*)lvobj;

  // only vertical scroll for now
  lv_coord_t h_before = 0;
  for (uint16_t i = 0; i < table->row_act; i++) h_before += table->row_h[i];

  lv_coord_t row_h = table->row_h[table->row_act];
  lv_coord_t scroll_y = lv_obj_get_scroll_y(lvobj);

  lv_obj_update_layout(lvobj);
  lv_coord_t h = lv_obj_get_height(lvobj);

  lv_coord_t diff_y = 0;
  if (h_before < scroll_y) {
    diff_y = scroll_y - h_before;
  } else if (scroll_y + h < h_before + row_h) {
    diff_y = scroll_y + h - h_before - row_h;
  } else {
    return;
  }

  lv_obj_scroll_by_bounded(lvobj, 0, diff_y, LV_ANIM_OFF);
}

void TableField::selectNext(int16_t dir)
{
  lv_table_t* table = (lv_table_t*)lvobj;

  if (table->col_act == LV_TABLE_CELL_NONE ||
      table->row_act == LV_TABLE_CELL_NONE) {

    if (table->col_cnt > 0 && table->row_cnt > 0) {
      table->col_act = 0;
      table->row_act = 0;
    }
  } else {
    table->col_act += dir;
    if (table->col_act >= table->col_cnt) {

      table->col_act = 0;
      table->row_act += dir;

      if (table->row_act >= table->row_cnt) {
        table->col_act = LV_TABLE_CELL_NONE;
        table->row_act = LV_TABLE_CELL_NONE;

        // wrap around
        if (table->col_cnt > 0 && table->row_cnt > 0) {

          if (dir < 0) {
            table->col_act = table->col_cnt - 1;
            table->row_act = table->row_cnt - 1;
          } else {
            table->col_act = 0;
            table->row_act = 0;
          }
        }
      }
    }
  }

  lv_obj_invalidate(lvobj);
  adjustScroll();
}

void TableField::onEvent(event_t event)
{
  if (event == EVT_ROTARY_RIGHT) {
    onKeyPress();
    selectNext(1);
  }
  else if (event == EVT_ROTARY_LEFT) {
    onKeyPress();
    selectNext(-1);
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();

    uint16_t row;
    uint16_t col;
    lv_table_get_selected_cell(lvobj, &row, &col);
    
    onPress(row, col);
  }
  else {
    Window::onEvent(event);
  }  
}
