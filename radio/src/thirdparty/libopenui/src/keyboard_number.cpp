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

#include "keyboard_number.h"

#include "numberedit.h"

constexpr coord_t KEYBOARD_HEIGHT = 90;
NumberKeyboard* NumberKeyboard::_instance = nullptr;

static const char* const number_kb_map[] = {"<<",  "-",   "+",   ">>",  "\n",
                                            "MIN", "DEF", "+/-", "MAX", ""};

#define LV_KB_BTN(width) LV_BTNMATRIX_CTRL_POPOVER | width
#define LV_KB_CTRL(width) LV_KEYBOARD_CTRL_BTN_FLAGS | width

static const lv_btnmatrix_ctrl_t number_kb_ctrl_map[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_CTRL(4), LV_KB_BTN(4)};

static void on_key(lv_event_t* e)
{
  lv_obj_t* obj = lv_event_get_target(e);
  NumberEdit* edit = (NumberEdit*)lv_event_get_user_data(e);
  if (!obj || !edit) return;

  uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
  if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

  const char* txt =
      lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
  if (txt == NULL) return;

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

#if defined(HARDWARE_KEYS)
void NumberKeyboard::onEvent(event_t event)
{
  NumberEdit* edit = (NumberEdit*)field;

#if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && !defined(PCBX12S)
  // Radios with both PGUP and PGDN buttons except X12S
  switch (event) {
    case EVT_KEY_BREAK(KEY_SYS):
      // "<<"
      edit->onEvent(EVT_VIRTUAL_KEY_BACKWARD);
    break;

    case EVT_KEY_LONG(KEY_SYS):
      killEvents(event);
      // "MIN"
      edit->onEvent(EVT_VIRTUAL_KEY_MIN);
      break;

    case EVT_KEY_BREAK(KEY_MODEL):
      // ">>"
      edit->onEvent(EVT_VIRTUAL_KEY_FORWARD);
      break;

    case EVT_KEY_LONG(KEY_MODEL):
      killEvents(event);
      // "MAX"
      edit->onEvent(EVT_VIRTUAL_KEY_MAX);
      break;

    case EVT_KEY_BREAK(KEY_PAGEDN):
      // "+"
      edit->onEvent(EVT_VIRTUAL_KEY_PLUS);
      break;

    case EVT_KEY_BREAK(KEY_PAGEUP):
      // "-"
      edit->onEvent(EVT_VIRTUAL_KEY_MINUS);
      break;

    case EVT_KEY_BREAK(KEY_TELE): 
      // "+/-"
      edit->onEvent(EVT_VIRTUAL_KEY_SIGN);
    break;

    case EVT_KEY_LONG(KEY_TELE):
      killEvents(event);
      // "DEF"
      edit->onEvent(EVT_VIRTUAL_KEY_DEFAULT);
      break;

    default:
      break;
  }

#else
  // Radios witb only a single PGUP/DN button or X12S
  switch (event) {
    case EVT_KEY_BREAK(KEY_SYS):
      // "-"
      edit->onEvent(EVT_VIRTUAL_KEY_MINUS);
      break;

    case EVT_KEY_LONG(KEY_SYS):
      killEvents(event);
      // "MIN"
      edit->onEvent(EVT_VIRTUAL_KEY_MIN);
      break;

    case EVT_KEY_BREAK(KEY_MODEL):
      // ">>"
      edit->onEvent(EVT_VIRTUAL_KEY_FORWARD);
    break;

    case EVT_KEY_LONG(KEY_MODEL):
      killEvents(event);
      // "+/-"
      edit->onEvent(EVT_VIRTUAL_KEY_SIGN);
      break;

#if defined(PCBX12S)
    case EVT_KEY_BREAK(KEY_PAGEUP):
#endif
    case EVT_KEY_BREAK(KEY_PAGEDN):
      // "<<"
      edit->onEvent(EVT_VIRTUAL_KEY_BACKWARD);
    break;

#if defined(PCBX12S)
    case EVT_KEY_LONG(KEY_PAGEUP):
#endif
    case EVT_KEY_LONG(KEY_PAGEDN):
      killEvents(event);
      // "DEF"
      edit->onEvent(EVT_VIRTUAL_KEY_DEFAULT);
      break;

    case EVT_KEY_BREAK(KEY_TELE):
      // "+"
      edit->onEvent(EVT_VIRTUAL_KEY_PLUS);
    break;

    case EVT_KEY_LONG(KEY_TELE):
      killEvents(event);
      // "MAX"
      edit->onEvent(EVT_VIRTUAL_KEY_MAX);
    break;

    default:
      break;
  }

#endif
}
#endif

NumberKeyboard::NumberKeyboard() : Keyboard(KEYBOARD_HEIGHT)
{
  // setup custom keyboard
  lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                      (const char**)number_kb_map, number_kb_ctrl_map);

  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
}

NumberKeyboard::~NumberKeyboard() { _instance = nullptr; }

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
