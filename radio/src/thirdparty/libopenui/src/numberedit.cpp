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

#include "numberedit.h"

#if defined(SOFTWARE_KEYBOARD)
#include "keyboard_number.h"
#endif


static void numberedit_cb(lv_event_t* e)
{
  NumberEdit* numEdit = (NumberEdit*)lv_event_get_user_data(e);
  if (!numEdit || numEdit->deleted()) return;

  uint32_t key = lv_event_get_key(e);
  switch (key) {
  case LV_KEY_LEFT:
    numEdit->onEvent(EVT_ROTARY_LEFT);
    break;
  case LV_KEY_RIGHT:
    numEdit->onEvent(EVT_ROTARY_RIGHT);
    break;
  }
}

// create a new class for number edit so we can set the sytles differently if needed.
const lv_obj_class_t lv_numberedit_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = lv_textarea_class.constructor_cb,
    .destructor_cb = lv_textarea_class.destructor_cb,
    .user_data = nullptr,
    .event_cb = lv_textarea_class.event_cb,
    .width_def = LV_DPI_DEF / 2,
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_textarea_t)
};

lv_obj_t *lv_numberedit_create(lv_obj_t *parent) 
{
    lv_obj_t * obj = lv_obj_class_create_obj(&lv_numberedit_class, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue),
                   std::move(setValue), windowFlags, textFlags,
                   lv_numberedit_create)
{
  // properties
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(lvobj, false);
  lv_textarea_set_one_line(lvobj, true);

  auto value = _getValue();
  this->setValue(value);
  lv_obj_add_event_cb(lvobj, numberedit_cb, LV_EVENT_KEY, this);
}

void NumberEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    switch (event) {
#if defined(HARDWARE_KEYS)
      case EVT_ROTARY_RIGHT: {
        int value = getValue();
        do {
          value += ROTARY_ENCODER_SPEED() * step;
        } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
        if (value <= vmax) {
          setValue(value);
          onKeyPress();
        }
        else {
          setValue(vmax);
          onKeyError();
        }
        return;
      }

      case EVT_ROTARY_LEFT: {
        int value = getValue();
        do {
          value -= ROTARY_ENCODER_SPEED() * step;
        } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
        if (value >= vmin) {
          setValue(value);
          onKeyPress();
        }
        else {
          setValue(vmin);
          onKeyError();
        }
        return;
      }
#endif

#if defined(HARDWARE_TOUCH)
      case EVT_VIRTUAL_KEY_PLUS:
        setValue(getValue() + getStep());
        break;

      case EVT_VIRTUAL_KEY_MINUS:
        setValue(getValue() - getStep());
        break;

      case EVT_VIRTUAL_KEY_FORWARD:
        setValue(getValue() + 10 * getStep());
        break;

      case EVT_VIRTUAL_KEY_BACKWARD:
        setValue(getValue() - 10 * getStep());
        break;

      case EVT_VIRTUAL_KEY_DEFAULT:
        setValue(getDefault());
        break;

      case EVT_VIRTUAL_KEY_MAX:
        setValue(getMax());
        break;

      case EVT_VIRTUAL_KEY_MIN:
        setValue(getMin());
        break;

      case EVT_VIRTUAL_KEY_SIGN:
        setValue(-getValue());
        break;
#endif
    }
  }

  FormField::onEvent(event);
}

void NumberEdit::onClicked()
{
  lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
  if(indev_type == LV_INDEV_TYPE_POINTER) {
    NumberKeyboard::show(this);
  } else {
    FormField::onClicked();
  }
}
