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

#include "modal_window.h"
#include "keyboard_base.h"
#include "font.h"
#include "layer.h"

ModalWindow::ModalWindow(Window * parent, bool closeWhenClickOutside):
  Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}, 0, 0, etx_modal_create),
  closeWhenClickOutside(closeWhenClickOutside)
{
  Layer::push(this);
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
    Window(parent, rect, OPAQUE, 0, etx_modal_content_create)
{
}

void ModalWindowContent::onClicked() { Keyboard::hide(false); }

void ModalWindowContent::setTitle(const std::string& text)
{
  if (!title) {
    title = etx_modal_title_create(lvobj);
    lv_obj_move_to_index(title, 0);
    lv_obj_update_layout(lvobj);
  }
  lv_label_set_text(title, text.c_str());
}

std::string ModalWindowContent::getTitle() const
{
  if (!title) return std::string();
  return lv_label_get_text(title);
}
