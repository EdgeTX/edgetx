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

#include "widgets/simple_btn.h"

static void update_checked_flag(lv_obj_t* obj, WindowFlags flags)
{
  if (!(flags & BUTTON_CHECKED))
    lv_obj_clear_state(obj, LV_STATE_CHECKED);
  else
    lv_obj_add_state(obj, LV_STATE_CHECKED);  
}

Button::Button(Window* parent, const rect_t& rect,
       std::function<uint8_t(void)> pressHandler,
       WindowFlags windowFlag, LcdFlags textFlags,
       LvglCreate objConstruct) :
    FormField(parent, rect, windowFlag, textFlags,
              objConstruct ? objConstruct : simple_btn_create),
    pressHandler(std::move(pressHandler))
{
  lv_obj_add_event_cb(lvobj, Button::long_pressed, LV_EVENT_LONG_PRESSED, nullptr);
  update_checked_flag(lvobj, windowFlags);
}

void Button::check(bool checked)
{
  if (_deleted) return;
  if (checked != bool(windowFlags & BUTTON_CHECKED)) {
    windowFlags ^= BUTTON_CHECKED;
    update_checked_flag(lvobj, windowFlags);
    invalidate();
  }
}

bool Button::checked() const
{
  if (windowFlags & BUTTON_CHECKED_ON_FOCUS)
    return hasFocus();
  else
    return windowFlags & BUTTON_CHECKED;
}

void Button::onPress()
{
  check(pressHandler && pressHandler());
}

void Button::onLongPress()
{
  if (longPressHandler) {
    check(longPressHandler());
    lv_obj_clear_state(lvobj, LV_STATE_PRESSED);
    lv_indev_wait_release(lv_indev_get_act());
  }
}

void Button::onClicked()
{
  onPress();
}

#if defined(HARDWARE_TOUCH)
// this needs to stay so that viewmain doesnt get the touch event
bool Button::onTouchEnd(coord_t x, coord_t y) { return true; }
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler) checkHandler();
}

void Button::long_pressed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto btn = (Button*)lv_obj_get_user_data(obj);
  if (obj) btn->onLongPress();
}


TextButton::TextButton(Window* parent, const rect_t& rect, std::string text,
                       std::function<uint8_t(void)> pressHandler,
                       WindowFlags windowFlags) :
    Button(parent, rect, std::move(pressHandler), windowFlags, 0, lv_btn_create),
    text(std::move(text))
{
  update_checked_flag(lvobj, windowFlags);

  label = lv_label_create(lvobj);
  lv_label_set_text(label, this->text.c_str());
  lv_obj_center(label);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
