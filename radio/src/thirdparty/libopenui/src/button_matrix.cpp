/*
 * Copyright (C) EdgeTX
 *
 * Source:
 *  https://github.com/EdgeTX/libopenui
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

#include "button_matrix.h"

#include <stdlib.h>
#include <string.h>

static const char _filler[] = "0";
static const char _newline[] = "\n";
static const char _map_end[] = "";

static void btn_matrix_event(lv_event_t *e)
{
  static lv_area_t draw_area;

  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    auto btn_id = (uint32_t *)lv_event_get_param(e);
    auto btnm = (ButtonMatrix*)lv_event_get_user_data(e);

    btnm->onPress((uint8_t)*btn_id);
  }
  else if (code == LV_EVENT_DRAW_PART_BEGIN) {

    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part != LV_PART_ITEMS) return;

    auto btnm = (ButtonMatrix*)lv_event_get_user_data(e);
    if (btnm->isActive((uint8_t)dsc->id)) {
      dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_ACTIVE);
      dsc->label_dsc->color = makeLvColor(COLOR_THEME_PRIMARY2);
    } else {
      dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_PRIMARY2);
      dsc->label_dsc->color = makeLvColor(COLOR_THEME_SECONDARY1);
    }

    // backup draw area as it gets modified for the label
    lv_area_copy(&draw_area, dsc->draw_area);
  }
}

ButtonMatrix::ButtonMatrix(Window* parent, const rect_t& r) :
  FormField(parent, r, 0, 0, lv_btnmatrix_create)
{
  lv_obj_add_event_cb(lvobj, btn_matrix_event, LV_EVENT_ALL, this);
}

ButtonMatrix::~ButtonMatrix()
{
  deallocate();
}

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

  uint8_t rows = ((btns - 1) / 4) + 1;
  txt_cnt = btns + rows;
  btn_cnt = btns;

  lv_btnm_map = (char**)malloc(sizeof(char*) * txt_cnt);
  txt_index = (uint8_t*)malloc(sizeof(uint8_t) * btns);

  uint8_t col = 0;
  uint8_t btn = 0;
  uint8_t txt_i = 0;

  while (btn < btns) {
    
    if (col == cols) {
      lv_btnm_map[txt_i++] = (char*)_newline;
      col = 0;
    }
    
    txt_index[btn] = txt_i;
    lv_btnm_map[txt_i++] = (char*)_filler;
    btn++;
    col++;
  }
  lv_btnm_map[txt_i++] = (char*)_map_end;
}

void ButtonMatrix::setText(uint8_t btn_id, const char* txt)
{
  if (btn_id >= btn_cnt) return;

  uint8_t txt_i = txt_index[btn_id];
  lv_btnm_map[txt_i] = strdup(txt);
}

void ButtonMatrix::update()
{
  lv_btnmatrix_set_map(lvobj, (const char**)lv_btnm_map);

  lv_btnmatrix_ctrl_t ctrl = LV_BTNMATRIX_CTRL_CLICK_TRIG |
                             LV_BTNMATRIX_CTRL_NO_REPEAT;

  lv_btnmatrix_set_btn_ctrl_all(lvobj, ctrl);
}

void ButtonMatrix::onEvent(event_t event) {}
void ButtonMatrix::setFocus(uint8_t flag, Window* from) {}
