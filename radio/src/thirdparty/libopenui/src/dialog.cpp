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

#include "dialog.h"
#include "mainwindow.h"
#include "theme.h"

DialogWindowContent::DialogWindowContent(Dialog* parent, const rect_t& rect) :
    ModalWindowContent(parent, rect),
    form(this, rect_t{})
{
  form.setFlexLayout();

  auto form_obj = form.getLvObj();
  lv_obj_set_style_pad_all(form_obj, lv_dpx(8), 0);

  lv_coord_t max_height = LCD_H * 0.8;
  lv_obj_set_style_max_height(form_obj, max_height, 0);
}

void DialogWindowContent::setTitle(const std::string& text)
{
  ModalWindowContent::setTitle(text);
  if (title) {
    lv_coord_t title_h = lv_obj_get_height(title);
    lv_obj_set_y(form.getLvObj(), title_h);
  }
}

void DialogWindowContent::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  form.deleteLater(true, false);
  ModalWindowContent::deleteLater(detach, trash);
}

void DialogWindowContent::updateSize()
{
  lv_obj_center(lvobj);
  ModalWindowContent::updateSize();
}

#if defined(DEBUG_WINDOWS)
std::string DialogWindowContent::getName() const
{
  return "DialogWindowContent";
}
#endif

Dialog::Dialog(Window* parent, std::string title, const rect_t& rect) :
    ModalWindow(parent), content(new DialogWindowContent(this, rect))
{
  lv_obj_set_style_bg_color(content->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
  lv_obj_set_style_bg_opa(content->getLvObj(), LV_OPA_100, LV_PART_MAIN);
  bringToTop();
  if (!title.empty()) content->setTitle(std::move(title));
}

void Dialog::onCancel()
{
  deleteLater();
}

void Dialog::onEvent(event_t event)
{
  // block key events 
  (void)event;
}
