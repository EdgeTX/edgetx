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

#include "choice.h"
#include "menu.h"
#include "theme.h"

void choicePaintCallback(lv_event_t *e)
{
  auto code = lv_event_get_code(e);

  if (code == LV_EVENT_DRAW_MAIN_BEGIN) {
    lv_obj_t *target = lv_event_get_target(e);
    if (target != nullptr) {
      ChoiceBase *cb = (ChoiceBase*)lv_obj_get_user_data(target);
      std::string text = cb->getLabelText();
      lv_label_set_text(cb->label, text.c_str());
    }
  }
}

ChoiceBase::ChoiceBase(FormGroup * parent, const rect_t & rect, ChoiceType type, WindowFlags windowFlags):
  FormField(parent, rect, windowFlags),
  type(type)
{
  lv_obj_add_event_cb(lvobj, choicePaintCallback, LV_EVENT_DRAW_MAIN_BEGIN, lvobj);
  label = lv_label_create(lvobj);
  lv_group_t * def_group = lv_group_get_default();
  if(def_group) {
      lv_group_add_obj(def_group, lvobj);
  }

  // Normal font and background color
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_pad_right(lvobj, 5, LV_PART_MAIN);
  lv_obj_set_style_border_width(lvobj, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN);
  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY1), LV_PART_MAIN);
  lv_obj_set_style_text_font(lvobj, &lv_font_roboto_13, LV_PART_MAIN);

  // focused
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_set_width(label, lv_pct(80));
  lv_obj_set_style_pad_left(label, FIELD_PADDING_LEFT, LV_PART_MAIN);
  lv_obj_set_style_pad_top(label, FIELD_PADDING_TOP, LV_PART_MAIN);


  // add the image
  lv_obj_t *img = lv_img_create(lvobj);
  lv_img_set_src(img, type == CHOICE_TYPE_DROPOWN ? LV_SYMBOL_DOWN : LV_SYMBOL_DIRECTORY);
  lv_obj_set_align(img, LV_ALIGN_RIGHT_MID);
}


std::string Choice::getLabelText()
{
  std::string text = "";
  if (textHandler != nullptr)
    text = textHandler(getValue());
  else {
    int val = getValue();
    val -= vmin;
    if (val >= 0 && val < (int)values.size()) {
      text = values[val];
    }
  }
  
  return text;
}

Choice::Choice(FormGroup * parent, const rect_t & rect, int vmin, int vmax,
  std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * const values[], int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  setValues(values);
}

Choice::Choice(FormGroup * parent, const rect_t & rect, std::vector<std::string> values, int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  values(std::move(values)),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * values, int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  if (values) {
    uint8_t len = values[0];
    const char * value = &values[1];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(std::string(value, min<uint8_t>(len, strlen(value))));
      value += len;
    }
  }
}

void Choice::addValue(const char * value)
{
  values.emplace_back(value);
  vmax += 1;
}

void Choice::addValues(const char * const values[], uint8_t count)
{
  this->values.reserve(this->values.size() + count);
  for (uint8_t i = 0; i < count; i++)
    this->values.emplace_back(values[i]);
  vmax += count;
}

void Choice::setValues(std::vector<std::string> values)
{
  this->values = std::move(values);
}

void Choice::setValues(const char * const values[])
{
  this->values.clear();
  if (values) {
    auto value = &values[0];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(*value++);
    }
  }
}

#if defined(HARDWARE_KEYS)
void Choice::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    openMenu();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

void Choice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  auto menu = new Menu(this);
  if (!menuTitle.empty())
    menu->setTitle(menuTitle);

  auto value = getValue();
  int count = 0;
  int current = -1;

  for (int i = vmin; i <= vmax; ++i) {
    if (isValueAvailable && !isValueAvailable(i))
      continue;
    if (textHandler) {
      menu->addLine(textHandler(i), [=]() {
        setValue(i);
      });
    }
    else if (unsigned(i - vmin) < values.size()) {
      menu->addLine(values[i - vmin], [=]() {
        setValue(i);
      });
    }
    else {
      menu->addLine(std::to_string(i), [=]() {
        setValue(i);
      });
    }
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }

  if (beforeDisplayMenuHandler) {
    beforeDisplayMenuHandler(menu);
  }

  menu->setCloseHandler([=]() {
    setEditMode(false);
  });
}

#if defined(HARDWARE_TOUCH)
bool Choice::onTouchEnd(coord_t, coord_t)
{
  if (enabled) {
    if (!hasFocus()) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onKeyPress();
    openMenu();
  }
  return true;
}
#endif
