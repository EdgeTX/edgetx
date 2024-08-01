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

#include "button_matrix.h"

#include <string.h>

#include "etx_lv_theme.h"

static void btnmatrix_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->bg_opacity_20,
                    LV_PART_MAIN | LV_STATE_FOCUSED);

  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX, LV_PART_MAIN | LV_STATE_FOCUSED);

  etx_std_style(obj, LV_PART_ITEMS, PAD_LARGE);

  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_FOCUS_INDEX],
                    LV_PART_ITEMS | LV_STATE_EDITED);
}

static const lv_obj_class_t btnmatrix_class = {
    .base_class = &lv_btnmatrix_class,
    .constructor_cb = btnmatrix_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_btnmatrix_t),
};

static lv_obj_t* btnmatrix_create(lv_obj_t* parent)
{
  return etx_create(&btnmatrix_class, parent);
}

static const char _filler[] = "0";
static const char _newline[] = "\n";
static const char _map_end[] = "";

static void btn_matrix_event(lv_event_t* e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t* obj = lv_event_get_target(e);
    auto btn_id = *((uint8_t*)lv_event_get_param(e));
    auto btnm = (ButtonMatrix*)lv_event_get_user_data(e);

    bool edited = lv_obj_has_state(obj, LV_STATE_EDITED);
    bool is_pointer =
        lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER;
    if (edited || is_pointer) {
      btnm->onPress(btn_id);
    }
  }
}

ButtonMatrix::ButtonMatrix(Window* parent, const rect_t& r) :
    FormField(parent, r, btnmatrix_create)
{
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  lv_obj_add_event_cb(lvobj, btn_matrix_event, LV_EVENT_ALL, this);
}

ButtonMatrix::~ButtonMatrix() { deallocate(); }

void ButtonMatrix::deallocate()
{
  if (txt_cnt == 0) return;

  for (uint8_t i = 0; i < txt_cnt; i++) {
    char* txt = lv_btnm_map[i];
    if (txt != _filler && txt != _newline && txt != _map_end) free(txt);
  }

  free(lv_btnm_map);
  free(txt_index);

  txt_cnt = 0;
  btn_cnt = 0;
}

void ButtonMatrix::initBtnMap(uint8_t cols, uint8_t btns)
{
  deallocate();

  uint8_t rows = ((btns - 1) / cols) + 1;
  if (rows == 1) cols = btns;
  txt_cnt = (cols + 1) * rows;
  btn_cnt = btns;

  lv_btnm_map = (char**)malloc(sizeof(char*) * txt_cnt);
  txt_index = (uint8_t*)malloc(sizeof(uint8_t) * cols * rows);

  uint8_t col = 0;
  uint8_t btn = 0;
  uint8_t txt_i = 0;

  while (btn < cols * rows) {
    if (col == cols) {
      lv_btnm_map[txt_i++] = (char*)_newline;
      col = 0;
    }

    txt_index[btn] = txt_i;
    lv_btnm_map[txt_i++] = (char*)_filler;
    btn++;
    col++;
  }
  lv_btnm_map[txt_i] = (char*)_map_end;
  update();
}

void ButtonMatrix::setText(uint8_t btn_id, const char* txt)
{
  if (btn_id < btn_cnt) lv_btnm_map[txt_index[btn_id]] = strdup(txt);
}

void ButtonMatrix::update()
{
  lv_btnmatrix_set_map(lvobj, (const char**)lv_btnm_map);
  lv_btnmatrix_set_btn_ctrl_all(
      lvobj, LV_BTNMATRIX_CTRL_CLICK_TRIG | LV_BTNMATRIX_CTRL_NO_REPEAT);
  int btn = 0;
  for (int i = 0; lv_btnm_map[i] != _map_end; i += 1) {
    if (lv_btnm_map[i] == _filler)
      lv_btnmatrix_set_btn_ctrl(lvobj, btn, LV_BTNMATRIX_CTRL_HIDDEN);
    else
      lv_btnmatrix_clear_btn_ctrl(lvobj, btn, LV_BTNMATRIX_CTRL_HIDDEN);
    if (lv_btnm_map[i] != _newline) btn += 1;
  }
}

void ButtonMatrix::onClicked()
{
  lv_group_focus_obj(lvobj);
  setEditMode(true);
}

void ButtonMatrix::setChecked(uint8_t btn_id)
{
  if (isActive(btn_id))
    lv_btnmatrix_set_btn_ctrl(lvobj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
  else
    lv_btnmatrix_clear_btn_ctrl(lvobj, btn_id, LV_BTNMATRIX_CTRL_CHECKED);
}
