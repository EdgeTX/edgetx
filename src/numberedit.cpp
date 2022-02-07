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


static LvglWidgetFactory numberEditFactory = { lv_textarea_create, nullptr };
static lv_style_t style_main;
static lv_style_t style_edit;

static void text_area_cb(lv_event_t* e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_DELETE) return;

  NumberEdit* numEdit = (NumberEdit*)lv_event_get_user_data(e);
  if (!numEdit || numEdit->deleted()) return;

  uint32_t key = lv_event_get_key(e);
  lv_obj_t* obj = numEdit->getLvObj();

  if (obj != nullptr && lv_obj_is_editable(obj)) {
    lv_group_t* grp = (lv_group_t*)lv_obj_get_group(obj);
    if (grp && lv_group_get_focused(grp) == obj)
      numEdit->setEditMode(lv_group_get_editing(grp));

    switch (key) {
      case LV_KEY_LEFT:
        //    numEdit->onEvent(EVT_ROTARY_LEFT);
        break;
      case LV_KEY_RIGHT:
        //    numEdit->onEvent(EVT_ROTARY_RIGHT);
        break;
    }
  }

  // TODO: this looks highly forbidden!!!
  e->code = LV_EVENT_REFRESH;
}

NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue),
                   std::move(setValue), windowFlags, textFlags,
                   &numberEditFactory)
{
  // properties
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(lvobj, false);
  lv_textarea_set_one_line(lvobj, true);

  auto value = _getValue();
  this->setValue(value);

  // LV_PART_MAIN
  lv_style_init(&style_main);
  lv_style_set_border_width(&style_main, 1);
  lv_style_set_border_color(&style_main, makeLvColor(COLOR_THEME_SECONDARY2));
  lv_style_set_bg_color(&style_main, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
  //lv_style_set_radius(&style_main, 0);
  lv_style_set_text_font(&style_main, &lv_font_roboto_13);
  lv_style_set_text_color(&style_main, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_obj_add_style(lvobj, &style_main, LV_PART_MAIN);

  // LV_STATE_FOCUSED
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_FOCUS),
                            LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2),
                              LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_set_style_outline_width(lvobj, 0, LV_PART_MAIN|LV_STATE_EDITED);
  lv_obj_set_style_outline_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN|LV_STATE_EDITED);
  lv_obj_set_style_outline_width(lvobj, 0, LV_PART_MAIN|LV_STATE_FOCUS_KEY);
  lv_obj_set_style_outline_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN|LV_STATE_FOCUS_KEY);

  // Hide cursor
  lv_obj_set_style_opa(lvobj, 0, LV_PART_CURSOR);

  // Show Cursor in "Edit" mode
  lv_style_init(&style_edit);
  lv_style_set_opa(&style_edit, LV_OPA_COVER);
  lv_style_set_bg_opa(&style_edit, LV_OPA_COVER);
  lv_style_set_pad_left(&style_edit, (lv_coord_t)-(FIELD_PADDING_LEFT+2));
  lv_style_set_pad_top(&style_edit, (lv_coord_t)-(FIELD_PADDING_TOP+2));
  lv_obj_add_style(lvobj, &style_edit, LV_PART_CURSOR | LV_STATE_EDITED);

  // Text padding
  auto label = lv_textarea_get_label(lvobj);
  lv_obj_set_style_pad_left(label, FIELD_PADDING_LEFT, LV_PART_MAIN);
  lv_obj_set_style_pad_top(label, FIELD_PADDING_TOP, LV_PART_MAIN);

  lv_obj_add_event_cb(lvobj, text_area_cb, LV_EVENT_ALL, this);
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

    case EVT_KEY_FIRST(KEY_EXIT):
#if defined(SOFTWARE_KEYBOARD)
      Keyboard::hide();
      return;
#endif
      break;


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

void NumberEdit::onFocusLost()
{
#if defined(SOFTWARE_KEYBOARD)
  Keyboard::hide();
#endif

  FormField::onFocusLost();
}


#if defined(HARDWARE_TOUCH)
bool NumberEdit::onTouchEnd(coord_t, coord_t)
{
  if (!enabled) {
    return true;
  }

  if (hasFocus()) {
    setEditMode(true);
  }
  else {
    setFocus(SET_FOCUS_DEFAULT);
  }

#if defined(SOFTWARE_KEYBOARD)
  NumberKeyboard::show(this);
#endif

  return true;
}
#endif

