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

#include "etx_lv_theme.h"
#include "numberedit.h"
#include "keys.h"

constexpr coord_t KEYBOARD_HEIGHT = LCD_H;
constexpr coord_t HEADER_HEIGHT = LAYOUT_SCALE(72);
constexpr coord_t TITLE_Y = PAD_LARGE;
constexpr coord_t TITLE_H = LAYOUT_SCALE(18);
constexpr coord_t VALUE_Y = TITLE_Y + TITLE_H + PAD_TINY;
constexpr coord_t VALUE_H = HEADER_HEIGHT - VALUE_Y - PAD_MEDIUM;
NumberKeyboard* NumberKeyboard::_instance = nullptr;

#define LV_KB_BTN(width) LV_BTNMATRIX_CTRL_POPOVER | width
#define LV_KB_CTRL(width) LV_KEYBOARD_CTRL_BTN_FLAGS | width

static char edit_value[32] = "";
static char edit_title[32] = "";

static const char* const positive_integer_kb_map[] = {
    "1", "2", "3", LV_SYMBOL_OK, "\n",
    "4", "5", "6", LV_SYMBOL_BACKSPACE, "\n",
    "7", "8", "9", LV_SYMBOL_LEFT, "\n",
    "0", LV_SYMBOL_RIGHT, ""};

static const lv_btnmatrix_ctrl_t positive_integer_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_CTRL(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(3), LV_KB_BTN(1)};

static const char* const signed_integer_kb_map[] = {
    "1", "2", "3", LV_SYMBOL_OK, "\n",
    "4", "5", "6", LV_SYMBOL_BACKSPACE, "\n",
    "7", "8", "9", LV_SYMBOL_LEFT, "\n",
    "+/-", "0", LV_SYMBOL_RIGHT, ""};

static const lv_btnmatrix_ctrl_t signed_integer_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_CTRL(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(2), LV_KB_BTN(1)};

static const char* const positive_decimal_kb_map[] = {
    "1", "2", "3", LV_SYMBOL_OK, "\n",
    "4", "5", "6", LV_SYMBOL_BACKSPACE, "\n",
    "7", "8", "9", LV_SYMBOL_LEFT, "\n",
    "0", ".", LV_SYMBOL_RIGHT, ""};

static const lv_btnmatrix_ctrl_t positive_decimal_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_CTRL(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(2), LV_KB_BTN(1), LV_KB_BTN(1)};

static const char* const signed_decimal_kb_map[] = {
    "1", "2", "3", LV_SYMBOL_OK, "\n",
    "4", "5", "6", LV_SYMBOL_BACKSPACE, "\n",
    "7", "8", "9", LV_SYMBOL_LEFT, "\n",
    "+/-", "0", ".", LV_SYMBOL_RIGHT, ""};

static const lv_btnmatrix_ctrl_t signed_decimal_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_CTRL(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1)};

static const char* const adjust_kb_map[] = {
    "<<", "-", "+", ">>", "\n",
    "MIN", "DEF", "+/-", "MAX", "\n",
    LV_SYMBOL_OK, ""};

static const lv_btnmatrix_ctrl_t adjust_kb_ctrl_map[] = {
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
    LV_KB_CTRL(4)};

static bool directMode = true;

static void set_edit_title(const char* value)
{
  if (!value || !value[0]) value = "Value";
  snprintf(edit_title, sizeof(edit_title), "%s", value);
  if (NumberKeyboard::instance())
    NumberKeyboard::instance()->setTitleText(edit_title);
}

static void set_edit_value(const char* value)
{
  if (!value) value = "";
  snprintf(edit_value, sizeof(edit_value), "%s", value);
  if (NumberKeyboard::instance())
    NumberKeyboard::instance()->setValueText(edit_value);
}

static void refresh_edit_value(lv_obj_t* keyboard, NumberEdit* edit = nullptr)
{
  if (directMode) {
    auto textarea = lv_keyboard_get_textarea(keyboard);
    if (!textarea) return;
    set_edit_value(lv_textarea_get_text(textarea));
  } else {
    set_edit_value(edit ? edit->getDisplayVal().c_str() : "");
  }
  lv_obj_invalidate(keyboard);
}

static void on_keyboard_event(lv_event_t* e)
{
  if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

  auto keyboard = lv_event_get_target(e);
  auto numberKeyboard = (NumberKeyboard*)lv_event_get_user_data(e);
  if (!numberKeyboard) return;

  if (directMode) {
    refresh_edit_value(keyboard);
  } else {
    uint16_t btn_id = lv_btnmatrix_get_selected_btn(keyboard);
    if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

    const char* txt = lv_btnmatrix_get_btn_text(keyboard, btn_id);
    if (!txt) return;

    numberKeyboard->handleEvent(txt);
    refresh_edit_value(keyboard, numberKeyboard->getNumberEdit());
  }
}

static void set_keyboard_map(lv_obj_t* keyboard, NumberEdit* edit)
{
  directMode = edit == nullptr || edit->useDirectKeyboard();
  set_edit_title(edit ? edit->getEditTitle().c_str() : nullptr);
  if (!directMode) {
    set_edit_value(edit ? edit->getDisplayVal().c_str() : "");
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                        (const char**)adjust_kb_map, adjust_kb_ctrl_map);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
    return;
  }

  bool isSigned = edit && edit->getMin() < 0;
  bool isDecimal = edit && edit->hasDecimalPrecision();

  set_edit_value(edit ? edit->getEditVal().c_str() : "");

  if (isSigned && isDecimal) {
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                        (const char**)signed_decimal_kb_map,
                        signed_decimal_kb_ctrl_map);
  } else if (isSigned) {
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                        (const char**)signed_integer_kb_map,
                        signed_integer_kb_ctrl_map);
  } else if (isDecimal) {
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                        (const char**)positive_decimal_kb_map,
                        positive_decimal_kb_ctrl_map);
  } else {
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1,
                        (const char**)positive_integer_kb_map,
                        positive_integer_kb_ctrl_map);
  }

  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
}

void NumberKeyboard::handleEvent(const char* btn)
{
  if (strcmp(btn, LV_SYMBOL_OK) == 0)
    Keyboard::hide(false);
  else if (strcmp(btn, "<<") == 0)
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

void NumberKeyboard::onPressSYS() { if (hasTwoPageKeys) decLarge(); else decSmall(); }
void NumberKeyboard::onLongPressSYS() { setMIN(); }
void NumberKeyboard::onPressMDL() { incLarge(); }
void NumberKeyboard::onLongPressMDL() { if (hasTwoPageKeys) setMAX(); else changeSign(); }
void NumberKeyboard::onPressTELE() { if (hasTwoPageKeys) changeSign(); else incSmall(); }
void NumberKeyboard::onLongPressTELE() { if (hasTwoPageKeys) setDEF(); else setMAX(); }
void NumberKeyboard::onPressPGUP() { if (hasTwoPageKeys) decSmall(); else setDEF(); }
void NumberKeyboard::onPressPGDN() { if (hasTwoPageKeys) incSmall(); else decLarge(); }

#endif

NumberKeyboard::NumberKeyboard() : Keyboard(KEYBOARD_HEIGHT, true)
{
  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY3_INDEX);

  titleLabel = lv_label_create(lvobj);
  lv_obj_set_pos(titleLabel, 0, TITLE_Y);
  lv_obj_set_size(titleLabel, LCD_W, TITLE_H);
  lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_DOT);
  etx_font(titleLabel, FONT_XS_INDEX);
  etx_txt_color(titleLabel, COLOR_THEME_PRIMARY3_INDEX);

  valueLabel = lv_label_create(lvobj);
  lv_obj_set_pos(valueLabel, 0, VALUE_Y);
  lv_obj_set_size(valueLabel, LCD_W, VALUE_H);
  lv_obj_set_style_text_align(valueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_DOT);
  etx_font(valueLabel, FONT_L_INDEX);
  etx_txt_color(valueLabel, COLOR_THEME_PRIMARY1_INDEX);

  lv_obj_set_size(keyboard, LCD_W, LCD_H - HEADER_HEIGHT);
  lv_obj_align(keyboard, LV_ALIGN_TOP_LEFT, 0, HEADER_HEIGHT);
  lv_obj_add_event_cb(keyboard, on_keyboard_event, LV_EVENT_VALUE_CHANGED, this);
}

NumberKeyboard::~NumberKeyboard() { _instance = nullptr; }

void NumberKeyboard::open(FormField* field, NumberEdit* edit)
{
  if (!_instance) _instance = new NumberKeyboard();

  _instance->numberEdit = edit;
  set_keyboard_map(_instance->keyboard, edit);

  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);

  _instance->setField(field);
  if (!directMode) {
    lv_keyboard_set_textarea(_instance->keyboard, nullptr);
  }
  refresh_edit_value(_instance->keyboard, edit);
}

void NumberKeyboard::setTitleText(const char* value)
{
  if (titleLabel) lv_label_set_text(titleLabel, value ? value : "");
}

void NumberKeyboard::setValueText(const char* value)
{
  if (valueLabel) lv_label_set_text(valueLabel, value ? value : "");
}
