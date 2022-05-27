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

void choice_changed_cb(lv_event_t *e)
{
  auto code = lv_event_get_code(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *target = lv_event_get_target(e);
    if (target != nullptr) {
      ChoiceBase *cb = (ChoiceBase*)lv_obj_get_user_data(target);
      if (cb) {
        std::string text = cb->getLabelText();
        lv_label_set_text(cb->label, text.c_str());
      }
    }
  }
}

ChoiceBase::ChoiceBase(Window* parent, const rect_t& rect, ChoiceType type,
                       WindowFlags windowFlags) :
    FormField(parent, rect, windowFlags), type(type)
{
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  lv_obj_set_width(lvobj, LV_SIZE_CONTENT);
  lv_obj_set_layout(lvobj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW);

  lv_obj_add_event_cb(lvobj, choice_changed_cb, LV_EVENT_VALUE_CHANGED, lvobj);
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

  // focused
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_set_style_pad_left(label, FIELD_PADDING_LEFT, LV_PART_MAIN);
  lv_obj_set_style_pad_top(label, FIELD_PADDING_TOP, LV_PART_MAIN);

  // add the image
  lv_obj_t *img = lv_img_create(lvobj);
  lv_img_set_src(img, type == CHOICE_TYPE_DROPOWN ? LV_SYMBOL_DOWN : LV_SYMBOL_DIRECTORY);
  lv_obj_set_align(img, LV_ALIGN_RIGHT_MID);
}

std::string Choice::getLabelText()
{
  std::string text;

  if (textHandler != nullptr)
    text = textHandler(getValue());
  else if (getValue) {
    int val = getValue();
    val -= vmin;
    if (val >= 0 && val < (int)values.size()) {
      text = values[val];
    }
  }
  
  return text;
}

Choice::Choice(Window* parent, const rect_t & rect, int vmin, int vmax,
  std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect, const char* const values[],
               int vmin, int vmax, std::function<int()> getValue,
               std::function<void(int)> setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    vmin(vmin),
    vmax(vmax),
    getValue(std::move(getValue)),
    setValue(std::move(setValue))
{
  setValues(values);
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect,
               std::vector<std::string> values, int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue,
               WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    values(std::move(values)),
    vmin(vmin),
    vmax(vmax),
    getValue(std::move(getValue)),
    setValue(std::move(setValue))
{
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect, const char* values, int vmin,
               int vmax, std::function<int()> getValue,
               std::function<void(int)> setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    vmin(vmin),
    vmax(vmax),
    getValue(std::move(getValue)),
    setValue(std::move(setValue))
{
  if (values) {
    uint8_t len = values[0];
    const char* value = &values[1];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(
          std::string(value, min<uint8_t>(len, strlen(value))));
      value += len;
    }
  }
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
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

void Choice::onClicked()
{
  onKeyPress();
  openMenu();
}

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
        lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
      });
    }
    else if (unsigned(i - vmin) < values.size()) {
      menu->addLine(values[i - vmin], [=]() {
        setValue(i);
        lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
      });
    }
    else {
      menu->addLine(std::to_string(i), [=]() {
        setValue(i);
        lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
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
