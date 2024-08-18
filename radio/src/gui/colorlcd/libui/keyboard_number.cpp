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
  NumberKeyboard* edit = (NumberKeyboard*)lv_event_get_user_data(e);
  if (!obj || !edit) return;

  uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
  if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

  const char* txt =
      lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
  if (txt == NULL) return;

  edit->handleEvent(txt);
}

void NumberKeyboard::handleEvent(const char* btn)
{
  if (strcmp(btn, "<<") == 0)
    decLarge();
  else if (strcmp(btn, "-") == 0)
    decSmall();
  else if (strcmp(btn, "+") == 0)
    incSmall();
  else if (strcmp(btn, ">>") == 0)
    incLarge();
  else if (strcmp(btn, "MIN") == 0)
    setMIN();
  else if (strcmp(btn, "DEF") == 0)
    setDEF();
  else if (strcmp(btn, "MAX") == 0)
    setMAX();
  else if (strcmp(btn, "+/-") == 0)
    changeSign();
}

void NumberKeyboard::decLarge()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_BACKWARD);
}

void NumberKeyboard::decSmall()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_MINUS);
}

void NumberKeyboard::incSmall()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_PLUS);
}

void NumberKeyboard::incLarge()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_FORWARD);
}

void NumberKeyboard::setMIN()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_MIN);
}

void NumberKeyboard::setMAX()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_MAX);
}

void NumberKeyboard::setDEF()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_DEFAULT);
}

void NumberKeyboard::changeSign()
{
  ((NumberEdit*)field)->onEvent(EVT_VIRTUAL_KEY_SIGN);
}

#if defined(HARDWARE_KEYS)

#if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && \
    !defined(PCBX12S)
// Radios with both PGUP and PGDN buttons except X12S
void NumberKeyboard::onPressSYS() { decLarge(); }
void NumberKeyboard::onLongPressSYS() { setMIN(); }
void NumberKeyboard::onPressMDL() { incLarge(); }
void NumberKeyboard::onLongPressMDL() { setMAX(); }
void NumberKeyboard::onPressTELE() { changeSign(); }
void NumberKeyboard::onLongPressTELE() { setDEF(); }
void NumberKeyboard::onPressPGUP() { decSmall(); }
void NumberKeyboard::onPressPGDN() { incSmall(); }
void NumberKeyboard::onLongPressPGUP() {}
void NumberKeyboard::onLongPressPGDN() {}
#else
// Radios witb only a single PGUP/DN button or X12S
void NumberKeyboard::onPressSYS() { decSmall(); }
void NumberKeyboard::onLongPressSYS() { setMIN(); }
void NumberKeyboard::onPressMDL() { incLarge(); }
void NumberKeyboard::onLongPressMDL() { changeSign(); }
void NumberKeyboard::onPressTELE() { incSmall(); }
void NumberKeyboard::onLongPressTELE() { setMAX(); }
#if defined(PCBX12S)
void NumberKeyboard::onPressPGUP() { decLarge(); }
#else
void NumberKeyboard::onPressPGUP() { setDEF(); }
#endif
void NumberKeyboard::onPressPGDN() { decLarge(); }
void NumberKeyboard::onLongPressPGUP() { setDEF(); }
void NumberKeyboard::onLongPressPGDN() { setDEF(); }
#endif

#endif

NumberKeyboard::NumberKeyboard() : Keyboard(KEYBOARD_HEIGHT)
{
  // setup custom keyboard
  lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                      (const char**)number_kb_map, number_kb_ctrl_map);

  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
}

NumberKeyboard::~NumberKeyboard() { _instance = nullptr; }

void NumberKeyboard::open(FormField* field)
{
  if (!_instance) _instance = new NumberKeyboard();

  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);

  _instance->setField(field);

  auto kb = _instance->keyboard;
  lv_keyboard_set_textarea(kb, nullptr);

  lv_obj_remove_event_cb(kb, on_key);
  lv_obj_add_event_cb(kb, on_key, LV_EVENT_VALUE_CHANGED, _instance);
}
