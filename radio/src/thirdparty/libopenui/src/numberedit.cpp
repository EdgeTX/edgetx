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

#include "numberedit.h"
#include "keyboard_number.h"
#include "audio.h"

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

NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue,
                       WindowFlags windowFlags, LcdFlags textFlags) :
        FormField(parent, rect, windowFlags, textFlags, etx_number_edit_create),
        vmin(vmin),
        vmax(vmax),
        _getValue(std::move(getValue)),
        _setValue(std::move(setValue))
{
  // Allow encoder acceleration
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

  lv_obj_add_event_cb(lvobj, numberedit_cb, LV_EVENT_KEY, this);

  update();
}

void NumberEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    switch (event) {
#if defined(HARDWARE_KEYS)
      case EVT_ROTARY_RIGHT: {
        int value = getValue();
        auto step = getStep();
        step += (rotaryEncoderGetAccel() * getAccelFactor()) / 8;
        do {
          value += step;
        } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
        if (value <= vmax) {
          setValue(value);
        }
        else {
          setValue(vmax);
          onKeyError();
        }
        return;
      }

      case EVT_ROTARY_LEFT: {
        int value = getValue();
        auto step = getStep();
        step += (rotaryEncoderGetAccel() * getAccelFactor()) / 8;
        do {
          value -= step;
        } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
        if (value >= vmin) {
          setValue(value);
        }
        else {
          setValue(vmin);
          onKeyError();
        }
        return;
      }
#endif

      case EVT_VIRTUAL_KEY_PLUS:
        setValue(getValue() + getStep());
        break;

      case EVT_VIRTUAL_KEY_MINUS:
        setValue(getValue() - getStep());
        break;

      case EVT_VIRTUAL_KEY_FORWARD:
        setValue(getValue() + getFastStep() * getStep());
        break;

      case EVT_VIRTUAL_KEY_BACKWARD:
        setValue(getValue() - getFastStep() * getStep());
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
    }
  }

  FormField::onEvent(event);
}

void NumberEdit::onClicked()
{
  lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
  if(indev_type == LV_INDEV_TYPE_POINTER) {
    NumberKeyboard::show(this);
    return;
  }

  FormField::onClicked();
}
