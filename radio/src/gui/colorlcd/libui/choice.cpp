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
#include "etx_lv_theme.h"

// Choice
static void choice_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_MAIN, PAD_TINY);
  lv_obj_set_style_pad_hor(obj, PAD_MEDIUM, LV_PART_MAIN);
}

static const lv_obj_class_t choice_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = choice_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = EdgeTxStyles::UI_ELEMENT_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_obj_t),
};

static lv_obj_t* choice_create(lv_obj_t* parent)
{
  return etx_create(&choice_class, parent);
}

ChoiceBase::ChoiceBase(Window* parent, const rect_t& rect,
                       int vmin, int vmax, const char* title,
                       std::function<int()> _getValue,
                       std::function<void(int)> _setValue,
                       ChoiceType type) :
    FormField(parent, rect, choice_create),
    vmin(vmin), vmax(vmax), menuTitle(title), type(type),
    _getValue(std::move(_getValue)),
    _setValue(std::move(_setValue))
{
  padLeft(PAD_TINY);
  padRight(PAD_SMALL);

  // Add image
  lv_obj_t* img = lv_img_create(lvobj);
  lv_img_set_src(
      img, type == CHOICE_TYPE_DROPOWN ? LV_SYMBOL_DOWN : LV_SYMBOL_DIRECTORY);
  lv_obj_set_pos(img, 0, PAD_TINY);

  // Add label
  label = lv_label_create(lvobj);
  lv_obj_set_pos(label, type == CHOICE_TYPE_DROPOWN ? ICON_W - 2 : ICON_W, PAD_TINY);
  etx_font(label, FONT_XS_INDEX, LV_STATE_USER_1);
}

std::string Choice::getLabelText()
{
  std::string text;

  if (_getValue) {
    int val = _getValue();
    if (textHandler) {
      text = textHandler(val);
    } else {
      val -= vmin;
      if (val >= 0 && val < (int)values.size()) {
        text = values[val];
      } else {
        text = std::to_string(val + vmin);
      }
    }
  }

  return text;
}

void ChoiceBase::update()
{
  if (!deleted()) {
    if (width() > 0) {
      int w = width() - (type == CHOICE_TYPE_DROPOWN ? ICON_W - 2 : ICON_W) - PAD_TINY * 3;
      if (getTextWidth(getLabelText().c_str(), 0, FONT(STD)) > w)
        lv_obj_add_state(label, LV_STATE_USER_1);
      else
        lv_obj_clear_state(label, LV_STATE_USER_1);
    }
    lv_label_set_text(label, getLabelText().c_str());
  }
}

Choice::Choice(Window* parent, const rect_t& rect, int vmin, int vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue, const char* title, ChoiceType type) :
    ChoiceBase(parent, rect, vmin, vmax, title, getValue, setValue, type)
{
  update();
}

Choice::Choice(Window* parent, const rect_t& rect, const char* const values[],
               int vmin, int vmax, std::function<int()> getValue,
               std::function<void(int)> setValue, const char* title) :
    ChoiceBase(parent, rect, vmin, vmax, title, getValue, setValue, CHOICE_TYPE_DROPOWN)
{
  setValues(values);
  update();
}

Choice::Choice(Window* parent, const rect_t& rect,
               std::vector<std::string> values, int vmin, int vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue, const char* title) :
    ChoiceBase(parent, rect, vmin, vmax, title, getValue, setValue, CHOICE_TYPE_DROPOWN),
    values(std::move(values))
{
  update();
}

void Choice::addValue(const char* value)
{
  values.emplace_back(value);
  vmax += 1;
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
    update();
  }
}

void Choice::onClicked()
{
  if (!deleted())
    openMenu();
}

void Choice::fillMenu(Menu* menu, const FilterFct& filter)
{
  if (menu->count() > 0)
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

  auto menu = new Menu();
  if (menuTitle) menu->setTitle(menuTitle);

  fillMenu(menu);

  menu->setCloseHandler([=]() { setEditMode(false); });
}
