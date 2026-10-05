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
#include "keys.h"
#include "strhelpers.h"

NumberKeyboard* NumberKeyboard::_instance = nullptr;

// NOTE: number_kb_map must be a static (not class) object
//       as the lvgl keyboard keeps a reference to it.
//       A class object will eventually crash after the keyboard
//       is closed and reopened (e.g. USB SD mode)

// Fixed length strings for number keys.
// Key label set on open using step values.
// Ensure uniqueness & length.
static constexpr int NUM_BTN_LEN = 5;
static char decL[NUM_BTN_LEN+1] = "-10";
static char decS[NUM_BTN_LEN+1] = "-1";
static char incS[NUM_BTN_LEN+1] = "+1";
static char incL[NUM_BTN_LEN+1] = "+10";

static const char* number_kb_map[] = {
  decL,  decS,  incS,  incL,  "\n",
  "MIN", "DEF", "+/-", "MAX", ""
};

#define LV_KB_BTN(width) LV_BTNMATRIX_CTRL_POPOVER | width
#define LV_KB_CTRL(width) LV_KEYBOARD_CTRL_BTN_FLAGS | width

// Index of the '+/-' control map value.
static constexpr int CHG_SIGN_IDX = 6;

static lv_btnmatrix_ctrl_t number_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),  LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_CTRL(1), LV_KB_BTN(1)};

static void on_key(lv_event_t* e)
{
  lv_obj_t* obj = lv_event_get_target(e);
  NumberKeyboard* edit = (NumberKeyboard*)lv_event_get_user_data(e);
  if (!obj || !edit) return;

  edit->handleEvent(lv_btnmatrix_get_selected_btn(obj));
}

void NumberKeyboard::handleEvent(uint16_t btnId)
{
  if (btnId != LV_BTNMATRIX_BTN_NONE && eventHandler)
    eventHandler(btnId);
}

#if defined(HARDWARE_KEYS)

void NumberKeyboard::onKey(NumKBEvents e1, NumKBEvents e2)
{
  handleEvent(hasTwoPageKeys ? e1 : e2);
}

void NumberKeyboard::onPressSYS() { onKey(NumKBEvents::DEC_FASTSTEP, NumKBEvents::DEC_STEP); }
void NumberKeyboard::onLongPressSYS() { handleEvent(NumKBEvents::SET_MIN); }
void NumberKeyboard::onPressMDL() { handleEvent(NumKBEvents::INC_FASTSTEP); }
void NumberKeyboard::onLongPressMDL() { onKey(NumKBEvents::SET_MAX, NumKBEvents::CHANGE_SIGN); }
void NumberKeyboard::onPressTELE() { onKey(NumKBEvents::CHANGE_SIGN, NumKBEvents::INC_STEP); }
void NumberKeyboard::onLongPressTELE() { onKey(NumKBEvents::SET_DEFAULT, NumKBEvents::SET_MAX); }
void NumberKeyboard::onPressPGUP() { onKey(NumKBEvents::DEC_STEP, NumKBEvents::SET_DEFAULT); }
void NumberKeyboard::onPressPGDN() { onKey(NumKBEvents::INC_STEP, NumKBEvents::DEC_FASTSTEP); }

#endif

NumberKeyboard::NumberKeyboard() : Keyboard(KEYBOARD_HEIGHT)
{
  lv_obj_add_event_cb(lvobj, on_key, LV_EVENT_VALUE_CHANGED, this);

  lv_keyboard_set_mode(lvobj, LV_KEYBOARD_MODE_USER_1);

  onClosing([=]() {
    _instance = nullptr;
  });
}

void NumberKeyboard::openKeyboard(FormField* field, int stepSmall, int stepLarge, LcdFlags textFlags,
                                  bool hasChangeSign, std::function<void(uint16_t)> onEvent)
{
  strAppend(decL, formatNumberAsString(stepLarge, textFlags, 0, "-").c_str(), NUM_BTN_LEN);
  strAppend(decS, formatNumberAsString(stepSmall, textFlags, 0, "-").c_str(), NUM_BTN_LEN);
  strAppend(incS, formatNumberAsString(stepSmall, textFlags, 0, "+").c_str(), NUM_BTN_LEN);
  strAppend(incL, formatNumberAsString(stepLarge, textFlags, 0, "+").c_str(), NUM_BTN_LEN);

  // Show / hide '+/-' button
  number_kb_ctrl_map[CHG_SIGN_IDX] = hasChangeSign ? LV_KB_CTRL(1) : LV_BTNMATRIX_CTRL_HIDDEN;

  // setup custom keyboard
  lv_keyboard_set_map(lvobj, LV_KEYBOARD_MODE_USER_1,
                      (const char**)number_kb_map, number_kb_ctrl_map);

  eventHandler = onEvent;
  show();

  setField(field, false);
}

void NumberKeyboard::open(FormField* field, int stepSmall, int stepLarge, LcdFlags textFlags,
                          bool hasChangeSign, std::function<void(uint16_t)> onEvent)
{
  if (!_instance) _instance = new NumberKeyboard();

  _instance->openKeyboard(field, stepSmall, stepLarge, textFlags, hasChangeSign, onEvent);
}
