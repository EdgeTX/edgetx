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
#include "keyboard_base.h"
#include "font.h"
#include "layer.h"

ModalWindow::ModalWindow(Window * parent, bool closeWhenClickOutside):
  Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}),
  closeWhenClickOutside(closeWhenClickOutside)
{
  Layer::push(this);
  lv_obj_set_style_bg_color(lvobj, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_50, LV_PART_MAIN);
}

void ModalWindow::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void ModalWindow::onClicked()
{
  if (closeWhenClickOutside) {
    deleteLater();
  }
}

ModalWindowContent::ModalWindowContent(ModalWindow* parent,
                                       const rect_t& rect) :
    Window(parent, rect, OPAQUE)
{
}

void ModalWindowContent::onClicked()
{
  Keyboard::hide();
}

void ModalWindowContent::setTitle(const std::string& text)
{
  if (!title) {
    title = lv_label_create(lvobj);
    lv_obj_move_to_index(title, 0);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_pad_all(title, PAGE_PADDING, LV_PART_MAIN);

    lv_color_t bg_color = makeLvColor(COLOR_THEME_SECONDARY1);
    lv_obj_set_style_bg_color(title, bg_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(title, LV_OPA_100, LV_PART_MAIN);
    
    lv_color_t txt_color = makeLvColor(COLOR_THEME_PRIMARY2);
    lv_obj_set_style_text_color(title, txt_color, LV_PART_MAIN);
    
    lv_obj_update_layout(lvobj);
  }
  lv_label_set_text(title, text.c_str());
}

std::string ModalWindowContent::getTitle() const
{
  if (!title) return std::string();
  return lv_label_get_text(title);
}
