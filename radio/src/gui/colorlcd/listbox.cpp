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
#include "listbox.h"

// TODO:
// - split this into 2 handlers:
//   - key_cb: PRESSED / LONG_PRESSED (always on)
//   - focus_cb: for FOCUSED / DEFOCUSED (only w/ 'auto-edit' mode)
// - add 'auto-edit' mode (add / remove 'focus_cb')
//   - when turned on, ESC gets out of the edit-mode
//     and ENTER gets into edit-mode
//
void ListBase::event_cb(lv_event_t* e)
{
  static bool _nested = false;
  if (_nested) return;
  
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = lv_event_get_target(e);
  if (!obj) return;

  ListBase* lb = (ListBase*)lv_event_get_user_data(e);
  if (!lb) return;

  if (code == LV_EVENT_FOCUSED && lb->getAutoEdit()) {
    lv_group_set_editing((lv_group_t*)lv_obj_get_group(obj), true);
  } else if (code == LV_EVENT_DEFOCUSED) {
    // Hack to get rid of 'FOCUSED' event sent
    // when calling 'lv_group_set_editing()'
    _nested = true;
    lv_group_set_editing((lv_group_t*)lv_obj_get_group(obj), false);
    _nested = false;
  }
  else if (code == LV_EVENT_LONG_PRESSED) {
    lb->onLongPressed();
    lv_obj_clear_state(obj, LV_STATE_PRESSED);
    lv_indev_wait_release(lv_indev_get_act());
  }
}

ListBase::ListBase(Window* parent, const rect_t& rect,
                   const std::vector<std::string>& names, uint8_t lineHeight,
                   WindowFlags windowFlags) :
    TableField(parent, rect, windowFlags)
{
  lv_obj_add_event_cb(lvobj, ListBase::event_cb, LV_EVENT_ALL, this);

  setColumnCount(1);
  setColumnWidth(0, rect.w);

  setLineHeight(lineHeight);
  setNames(names);
}

void ListBase::setAutoEdit(bool enable)
{
  if (autoEdit == enable) return;

  autoEdit = enable;
  if (autoEdit && hasFocus()) {
    lv_group_t* g = (lv_group_t*)lv_obj_get_group(lvobj);
    if (g) lv_group_set_editing(g, true);    
  }
}

void ListBase::setName(uint16_t idx, const std::string& name)
{
  lv_table_set_cell_value(lvobj, idx, 0, name.c_str());  
}

void ListBase::setNames(const std::vector<std::string>& names)
{
  setRowCount(names.size());

  uint16_t row = 0;
  for (const auto& name: names) {
    setName(row, name);
    row++;
  }  
}

void ListBase::setLineHeight(uint8_t height)
{
  lv_obj_set_style_max_height(lvobj, height, LV_PART_ITEMS);
}

void ListBase::setSelected(int selected)
{
  select(selected, 0);
}

int ListBase::getSelected() const
{
  uint16_t row, col;
  lv_table_get_selected_cell(lvobj, &row, &col);
  if (row != LV_TABLE_CELL_NONE) { return row; }
  return -1;
}

void ListBase::setActiveItem(int item)
{
  if (item != activeItem) {
    activeItem = item;
    invalidate();
  }
}

int ListBase::getActiveItem() const
{
  return activeItem;
}

void ListBase::onPress(uint16_t row, uint16_t col)
{
  if (row == LV_TABLE_CELL_NONE) return;

  TRACE("SHORT_PRESS");
  if (pressHandler) { pressHandler(); }
}

void ListBase::onLongPressed()
{
  TRACE("LONG_PRESS");
  if (longPressHandler) { longPressHandler(); }
}

// TODO: !auto-edit
void ListBase::onClicked()
{
  if (!getAutoEdit()) {
    lv_group_set_editing((lv_group_t*)lv_obj_get_group(lvobj), true);
  } else {
    TableField::onClicked();
  }
}

void ListBase::onCancel()
{
  lv_group_t* g = (lv_group_t*)lv_obj_get_group(lvobj);
  if (!getAutoEdit() && lv_group_get_editing(g)) {
    lv_group_set_editing(g, false);
  } else {
    TableField::onCancel();
  }
}

void ListBase::onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc)
{
  if (row != activeItem) return;

  lv_area_t coords;
  lv_coord_t area_h = lv_area_get_height(dsc->draw_area);

  lv_coord_t cell_right = lv_obj_get_style_pad_right(lvobj, LV_PART_ITEMS);
  lv_coord_t font_h = getFontHeight(FONT(STD));

  coords.x1 = dsc->draw_area->x2 - cell_right - font_h;
  coords.x2 = coords.x1 + font_h;
  coords.y1 = dsc->draw_area->y1 + (area_h - font_h) / 2;
  coords.y2 = coords.y1 + font_h - 1;

  lv_draw_label(dsc->draw_ctx, dsc->label_dsc, &coords, LV_SYMBOL_OK, nullptr);
}
