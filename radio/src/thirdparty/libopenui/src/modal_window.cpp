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


#include "modal_window.h"
#include "font.h"
#include "layer.h"

ModalWindow::ModalWindow(Window * parent, bool closeWhenClickOutside):
  Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}),
  closeWhenClickOutside(closeWhenClickOutside)
{
  lv_obj_set_style_bg_color(lvobj, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_50, LV_PART_MAIN);
  Layer::push(this);
}

void ModalWindow::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;
    
  Layer::pop(this);

  Window::deleteLater(detach, trash);
}

#if defined(HARDWARE_TOUCH)
bool ModalWindow::onTouchEnd(coord_t x, coord_t y)
{
  if (closeWhenClickOutside) {
    onKeyPress();
    deleteLater();
  }
  return true;
}
#endif

ModalWindowContent::ModalWindowContent(ModalWindow* parent,
                                       const rect_t& rect) :
    Window(parent, rect, OPAQUE)
{
}

void ModalWindowContent::setTitle(const std::string& text)
{
  if (!title) {
    title = lv_label_create(lvobj);
    lv_obj_move_to_index(title, 0);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_pad_all(title, PAGE_PADDING, LV_PART_MAIN);
    // TODO: styling
    // lv_obj_set_style_pad_left(title, FIELD_PADDING_LEFT, 0);
  }
  lv_label_set_text(title, text.c_str());
}

std::string ModalWindowContent::getTitle() const
{
  if (!title) return std::string();
  return lv_label_get_text(title);
}
