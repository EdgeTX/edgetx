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

#include "choice.h"

#include "menu.h"
#include "theme.h"

void choice_changed_cb(lv_event_t* e)
{
  auto code = lv_event_get_code(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t* target = lv_event_get_target(e);
    if (target != nullptr) {
      ChoiceBase* cb = (ChoiceBase*)lv_obj_get_user_data(target);
      if (cb) {
        std::string text = cb->getLabelText();
        lv_label_set_text(cb->label, text.c_str());
      }
    }
  }
}

ChoiceBase::ChoiceBase(Window* parent, const rect_t& rect, ChoiceType type,
                       WindowFlags windowFlags) :
    FormField(parent, rect, windowFlags, 0, etx_choice_create), type(type)
{
  //   lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  //   lv_obj_set_width(lvobj, LV_SIZE_CONTENT);
  lv_obj_set_layout(lvobj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW);

  lv_obj_add_event_cb(lvobj, choice_changed_cb, LV_EVENT_VALUE_CHANGED, lvobj);
  label = lv_label_create(lvobj);

  lv_group_t* def_group = lv_group_get_default();
  if (def_group) {
    lv_group_add_obj(def_group, lvobj);
  }

  lv_obj_set_style_pad_left(label, FIELD_PADDING_LEFT, LV_PART_MAIN);
  lv_obj_set_style_pad_top(label, FIELD_PADDING_TOP, LV_PART_MAIN);
  lv_obj_set_style_pad_right(label, CHOICE_LABEL_MARGIN_RIGHT, LV_PART_MAIN);
  // lv_obj_set

  // add the image
  lv_obj_t* img = lv_img_create(lvobj);
  lv_img_set_src(
      img, type == CHOICE_TYPE_DROPOWN ? LV_SYMBOL_DOWN : LV_SYMBOL_DIRECTORY);
  lv_obj_set_align(img, LV_ALIGN_RIGHT_MID);
}

std::string Choice::getLabelText()
{
  std::string text;

  if (textHandler != nullptr) {
    if (_getValue) {
      text = textHandler(_getValue());
    }
  } else if (_getValue) {
    int val = _getValue();
    val -= vmin;
    if (val >= 0 && val < (int)values.size()) {
      text = values[val];
    }
  }

  return text;
}

Choice::Choice(Window* parent, const rect_t& rect, int vmin, int vmax,
               std::function<int()> _getValue,
               std::function<void(int)> _setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(_getValue)),
    _setValue(std::move(_setValue)),
    longPressData({})
{
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect, const char* const values[],
               int vmin, int vmax, std::function<int()> _getValue,
               std::function<void(int)> _setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(_getValue)),
    _setValue(std::move(_setValue)),
    longPressData({})
{
  setValues(values);
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect,
               std::vector<std::string> values, int vmin, int vmax,
               std::function<int()> _getValue,
               std::function<void(int)> _setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    values(std::move(values)),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(_getValue)),
    _setValue(std::move(_setValue)),
    longPressData({})
{
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

Choice::Choice(Window* parent, const rect_t& rect, const char* values, int vmin,
               int vmax, std::function<int()> _getValue,
               std::function<void(int)> _setValue, WindowFlags windowFlags) :
    ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(_getValue)),
    _setValue(std::move(_setValue)),
    longPressData({})
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

void Choice::addValue(const char* value)
{
  values.emplace_back(value);
  vmax += 1;
}

void Choice::addValues(const char* const values[], uint8_t count)
{
  this->values.reserve(this->values.size() + count);
  for (uint8_t i = 0; i < count; i++) this->values.emplace_back(values[i]);
  vmax += count;
}

void Choice::setValues(std::vector<std::string> values)
{
  this->values = std::move(values);
}

void Choice::setValues(const char* const values[])
{
  this->values.clear();
  if (values) {
    auto value = &values[0];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(*value++);
    }
  }
}

void Choice::setValue(int val)
{
  if (_setValue) {
    _setValue(val);
    lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
  }
}

void Choice::onClicked()
{
  if (!longPressData.isLongPressed) openMenu();
}

void Choice::fillMenu(Menu* menu, const FilterFct& filter)
{
  menu->removeLines();
  auto value = getIntValue();

  int count = 0;
  int selectedIx = -1;
  selectedIx0 = -1;
  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i)) continue;
    if (isValueAvailable && !isValueAvailable(inverted ? -i : i)) continue;
    if (textHandler) {
      menu->addLineBuffered(textHandler(i), [=]() { setValue(i); });
    } else if (unsigned(i - vmin) < values.size()) {
      menu->addLineBuffered(values[i - vmin], [=]() { setValue(i); });
    } else {
      menu->addLineBuffered(std::to_string(i), [=]() { setValue(i); });
    }
    if (value == i) {
      selectedIx = count;
    }
    if (i == 0) {
      selectedIx0 = count;
    }
    ++count;
  }
  if (fillMenuHandler) {
    fillMenuHandler(menu, value, selectedIx);
  }
  menu->updateLines();
  // Force update - in case selected row is first row
  menu->select(-1);
  if (selectedIx >= 0)
    menu->select(selectedIx);
  else if (selectedIx0 >= 0)
    menu->select(selectedIx0);
  else {
    menu->select(0);
  }
}

void Choice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  auto menu = new Menu(this);
  if (!menuTitle.empty()) menu->setTitle(menuTitle);

  if (beforeDisplayMenuHandler) {
    beforeDisplayMenuHandler(menu);
  }

  fillMenu(menu);

  menu->setCloseHandler([=]() { setEditMode(false); });
}

static void localLongPressHandler(lv_event_t* e)
{
  lv_eventData_t* ld = (lv_eventData_t*)lv_event_get_user_data(e);
  ld->isLongPressed = true;
  ld->lv_LongPressHandler(ld->userData);
}

void Choice::set_lv_LongPressHandler(lvHandler_t longPressHandler, void* data)
{
  TRACE("longPressHandler=%p", longPressHandler);

  if (longPressHandler) {
    longPressData.userData = data;
    longPressData.lv_LongPressHandler = longPressHandler;
    lv_obj_add_event_cb(lvobj, localLongPressHandler, LV_EVENT_LONG_PRESSED,
                        &longPressData);
    lv_obj_add_event_cb(lvobj, ClickHandler, LV_EVENT_CLICKED, this);
  }
}
