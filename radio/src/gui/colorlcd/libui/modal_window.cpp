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

#include "layer.h"
#include "etx_lv_theme.h"
#include "mainwindow.h"

// Modal overlay style (for dimming background)
void modal_window_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->bg_opacity_50, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_BLACK_INDEX, LV_PART_MAIN);
}

static const lv_obj_class_t modal_window_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_window_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

static lv_obj_t* modal_create(lv_obj_t* parent)
{
  return etx_create(&modal_window_class, parent);
}

ModalWindow::ModalWindow(bool closeWhenClickOutside) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}, modal_create),
    closeWhenClickOutside(closeWhenClickOutside)
{
  setWindowFlag(OPAQUE);
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
    // Call onCancel of first child (if one exists)
    if (children.size() > 0)
      children.front()->onCancel();
    else
      deleteLater();
  }
}
