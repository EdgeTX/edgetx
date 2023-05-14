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

#include "keyboard_number.h"
#include "numberedit.h"

constexpr coord_t KEYBOARD_HEIGHT = 90;
NumberKeyboard * NumberKeyboard::_instance = nullptr;

static const char* number_kb_map[] = {
   "<<",  "-",   "+",   ">>",  "\n",
   "MIN", "DEF", "+/-", "MAX", ""
};

#define LV_KB_BTN(width) LV_BTNMATRIX_CTRL_POPOVER | width
#define LV_KB_CTRL(width) LV_KEYBOARD_CTRL_BTN_FLAGS | width

static const lv_btnmatrix_ctrl_t number_kb_ctrl_map[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_CTRL(4), LV_KB_BTN(4)
};

static void on_key(lv_event_t* e)
{
  lv_obj_t * obj = lv_event_get_target(e);
  NumberEdit* edit = (NumberEdit*)lv_event_get_user_data(e);
  if (!obj || !edit) return;

  uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
  if(btn_id == LV_BTNMATRIX_BTN_NONE) return;

  const char * txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
  if(txt == NULL) return;

  if (strcmp(txt, "<<") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_BACKWARD);
  } else if (strcmp(txt, "-") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_MINUS);
  } else if (strcmp(txt, "+") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_PLUS);
  } else if (strcmp(txt, ">>") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_FORWARD);
  } else if (strcmp(txt, "MIN") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_MIN);
  } else if (strcmp(txt, "DEF") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_DEFAULT);
  } else if (strcmp(txt, "MAX") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_MAX);
  } else if (strcmp(txt, "+/-") == 0) {
    edit->onEvent(EVT_VIRTUAL_KEY_SIGN);
  }
}

NumberKeyboard::NumberKeyboard() :
  Keyboard(KEYBOARD_HEIGHT)
{
  // setup custom keyboard
  lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                      number_kb_map, number_kb_ctrl_map);

  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
}

NumberKeyboard::~NumberKeyboard()
{
  _instance = nullptr;
}

void NumberKeyboard::show(NumberEdit* field)
{
  if (!_instance) _instance = new NumberKeyboard();
  
  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);

  _instance->setField(field);

  auto kb = _instance->keyboard;
  lv_keyboard_set_textarea(kb, nullptr);

  lv_obj_remove_event_cb(kb, on_key);
  lv_obj_add_event_cb(kb, on_key, LV_EVENT_VALUE_CHANGED, field);
}
