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

#include "libopenui_config.h"
#include "button.h"
#include "font.h"
#include "theme.h"

static void lvglEvent(lv_event_t* e)
{
  Button* btn = (Button*)lv_event_get_user_data(e);
  if(btn->deleted())
    return;

  btn->onEvent(e);
}

Button::Button(Window* parent, const rect_t& rect,
       std::function<uint8_t(void)> pressHandler,
       WindowFlags windowFlag, LcdFlags textFlags,
       LvglCreate objConstruct) :
    FormField(parent, rect, windowFlag, textFlags, objConstruct),
    pressHandler(std::move(pressHandler))
{
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_add_event_cb(lvobj, lvglEvent, LV_EVENT_CLICKED, this);
}

void Button::onPress()
{
  bool check = (pressHandler && pressHandler());
  windowFlags = check ? windowFlags | BUTTON_CHECKED : windowFlags & ~BUTTON_CHECKED;
  if(lvobj == nullptr)
    return;

  if (! (windowFlags & BUTTON_CHECKED))
    lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
  else
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);
}

#if defined(HARDWARE_KEYS)
void Button::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("Button").c_str(), event);

  if (enabled && event == EVT_KEY_FIRST(KEY_ENTER)) {
    killEvents(event);
    onKeyPress();
    onPress();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

void Button::onEvent(lv_event_t* event)
{
  lv_event_code_t code = lv_event_get_code(event);

  TRACE_WINDOWS("%s received lvgl event code 0x%X", getWindowDebugString("Button").c_str(), code);

  if (enabled && code == LV_EVENT_CLICKED) {
    if (!(windowFlags & NO_FOCUS)) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onKeyPress();
    onPress();
  }
}

#if defined(HARDWARE_TOUCH)
// this needs to stay so that viewmain doesnt get the touch event
bool Button::onTouchEnd(coord_t x, coord_t y)  
{
  return true;
}
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler)
    checkHandler();
}

TextButton::TextButton(Window* parent, const rect_t& rect, std::string text,
                       std::function<uint8_t(void)> pressHandler,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    Button(parent, rect, std::move(pressHandler), windowFlags, textFlags,
           lv_btn_create),
    text(std::move(text))
{
  setTextFlags(textFlags | COLOR_THEME_PRIMARY1);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CHECKABLE);
  if (windowFlags & BUTTON_CHECKED)
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);

  label = lv_label_create(lvobj);
  lv_label_set_text(label, this->text.c_str());
  lv_obj_center(label);
  lv_obj_set_style_text_color(label, makeLvColor(this->getTextFlags()),
                              LV_PART_MAIN);
  lv_obj_set_style_text_color(label, makeLvColor(COLOR_THEME_PRIMARY2),
                              LV_PART_MAIN | LV_STATE_FOCUSED);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
