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

#include "table.h"

#include "etx_lv_theme.h"

// Table
const lv_style_const_prop_t table_cell_props[] = {
    LV_STYLE_CONST_BORDER_WIDTH(1),
    LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM),
    LV_STYLE_CONST_PAD_TOP(7),  LV_STYLE_CONST_PAD_BOTTOM(7),
    LV_STYLE_CONST_PAD_LEFT(4), LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(table_cell, table_cell_props);

static void table_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->pad_zero, LV_PART_MAIN);

  etx_scrollbar(obj);

  etx_solid_bg(obj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_ITEMS);
  etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX, LV_PART_ITEMS);
  etx_obj_add_style(obj, table_cell, LV_PART_ITEMS);
  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_SECONDARY2_INDEX], LV_PART_ITEMS);
  etx_obj_add_style(obj, styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX, LV_PART_ITEMS | LV_STATE_EDITED);
  etx_txt_color(obj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_ITEMS | LV_STATE_EDITED);
}

static const lv_obj_class_t table_class = {
    .base_class = &lv_table_class,
    .constructor_cb = table_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = TableField::table_event,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_table_t),
};

static lv_obj_t* table_create(lv_obj_t* parent)
{
  return etx_create(&table_class, parent);
}

void TableField::table_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  lv_obj_t* obj = lv_event_get_target(e);
  if (obj) {
    TableField* tf = (TableField*)lv_obj_get_user_data(obj);
    if (tf) {
      lv_event_code_t code = lv_event_get_code(e);
      switch (code) {
        case LV_EVENT_VALUE_CHANGED: {
          uint16_t row;
          uint16_t col;
          lv_table_get_selected_cell(obj, &row, &col);
          if (row != LV_TABLE_CELL_NONE && col != LV_TABLE_CELL_NONE) {
            lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
            if (indev_type == LV_INDEV_TYPE_ENCODER) {
              // Encoder send VALUE_CHANGED when selection changes
              tf->adjustScroll();
              tf->onSelected(row, col);
            } else {
              // Otherwise it's a click
              if (lv_group_get_editing((lv_group_t*)lv_obj_get_group(obj)) ||
                  indev_type == LV_INDEV_TYPE_POINTER) {
                tf->onPress(row, col);
              } else {
                tf->onClicked();
              }
              // Note: VALUE_CHANGED is generated on RELEASED
              //
              //   It must be avoided that CLICKED be generated afterwards
              //   in case the object has been deleted meanwhile and to
              //   avoid onClicked() being called.
              //
              lv_indev_reset(lv_indev_get_act(), nullptr);
            }
          }
        } break;
        case LV_EVENT_DRAW_PART_BEGIN: {
          lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
          if (dsc->part == LV_PART_ITEMS) {
            uint16_t cols = lv_table_get_col_cnt(obj);
            if (cols) {
              uint16_t row = dsc->id / cols;
              uint16_t col = dsc->id % cols;
              tf->onDrawBegin(row, col, dsc);
            }
          }
        } break;
        case LV_EVENT_DRAW_PART_END: {
          lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
          if (dsc->part == LV_PART_ITEMS) {
            uint16_t cols = lv_table_get_col_cnt(obj);
            if (cols) {
              uint16_t row = dsc->id / cols;
              uint16_t col = dsc->id % cols;
              tf->onDrawEnd(row, col, dsc);
            }
          }
        } break;
        case LV_EVENT_DRAW_POST: {
          bool has_focus = lv_obj_has_state(obj, LV_STATE_FOCUS_KEY);
          bool is_edited =
              lv_group_get_editing((lv_group_t*)lv_obj_get_group(obj));

          if (has_focus && !is_edited) {
            lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_opa = LV_OPA_TRANSP;
            rect_dsc.bg_img_opa = LV_OPA_TRANSP;
            rect_dsc.outline_opa = LV_OPA_TRANSP;
            rect_dsc.shadow_opa = LV_OPA_TRANSP;

            rect_dsc.border_color = makeLvColor(COLOR_THEME_FOCUS);
            rect_dsc.border_opa = LV_OPA_100;
            rect_dsc.border_width = lv_dpx(2);

            lv_area_t coords;
            lv_area_copy(&coords, &obj->coords);

            lv_draw_rect(draw_ctx, &rect_dsc, &coords);
          }
        } break;
        case LV_EVENT_RELEASED:
        {
          lv_table_t* table = (lv_table_t*)obj;
          /*From lv_table.c: handler for LV_EVENT_RELEASED*/
          lv_obj_invalidate(obj);
          lv_indev_t* indev = lv_indev_get_act();
          lv_obj_t* scroll_obj = lv_indev_get_scroll_obj(indev);
          if (table->col_act != LV_TABLE_CELL_NONE &&
              table->row_act != LV_TABLE_CELL_NONE && scroll_obj == NULL) {
            lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
          }
          // Do not call base class event handler
          return;
        }
        default:
          break;
      }
    }
  }

  /*Call the ancestor's event handler*/
  lv_obj_event_base(&table_class, e);
}

TableField::TableField(Window* parent, const rect_t& rect) :
    Window(parent, rect, table_create)
{
  setWindowFlag(OPAQUE);

  etx_scrollbar(lvobj);
  lv_table_set_col_cnt(lvobj, 1);
}

void TableField::setRowCount(uint16_t rows)
{
  lv_table_set_row_cnt(lvobj, rows);
}

uint16_t TableField::getRowCount() const { return lv_table_get_row_cnt(lvobj); }

void TableField::setColumnWidth(uint16_t col, coord_t w)
{
  lv_table_set_col_width(lvobj, col, w);
}

void TableField::select(uint16_t row, uint16_t col, bool force)
{
  lv_table_t* table = (lv_table_t*)lvobj;
  if (!force && table->row_act == row && table->col_act == row) return;

  if (row >= table->row_cnt || col >= table->col_cnt) {
    table->col_act = LV_TABLE_CELL_NONE;
    table->row_act = LV_TABLE_CELL_NONE;
  } else {
    table->row_act = row;
    table->col_act = col;
  }

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
    selectNext(1);
  } else if (event == EVT_ROTARY_LEFT) {
    selectNext(-1);
  } else {
    Window::onEvent(event);
  }
}

int TableField::getSelected() const
{
  uint16_t row, col;
  lv_table_get_selected_cell(lvobj, &row, &col);
  if (row != LV_TABLE_CELL_NONE) {
    return row;
  }
  return -1;
}

bool TableField::onLongPress()
{
  TRACE("LONG_PRESS");
  if (longPressHandler) {
    longPressHandler();
    lv_indev_wait_release(lv_indev_get_act());
    return false;
  }
  return true;
}

void TableField::setAutoEdit()
{
  autoedit = true;
  lv_group_t* g = (lv_group_t*)lv_obj_get_group(lvobj);
  if (g) {
    setFocusHandler([=](bool focus) {
      if (focus) {
        lv_group_set_focus_cb(g, TableField::force_editing);
      } else {
        lv_group_set_focus_cb(g, nullptr);
      }
    });
    lv_group_set_editing(g, true);
  }
}
