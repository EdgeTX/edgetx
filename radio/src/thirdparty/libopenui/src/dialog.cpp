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

static rect_t _get_form_rect(const rect_t& rect)
{
  return {0, POPUP_HEADER_HEIGHT, rect.w,
          coord_t(rect.h - POPUP_HEADER_HEIGHT)};
}

DialogWindowContent::DialogWindowContent(Dialog* parent, const rect_t& rect) :
    ModalWindowContent(parent, rect),
    form(this, _get_form_rect(rect), FORM_NO_BORDER)
{
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
std::string DialogWindowContent::getName() const override
{
  return "DialogWindowContent";
}
#endif

Dialog::Dialog(Window* parent, std::string title, const rect_t& rect) :
    ModalWindow(parent), content(new DialogWindowContent(this, rect))
{
  bringToTop();
  if (!title.empty()) content->setTitle(std::move(title));
}

void Dialog::onCancel()
{
  deleteLater();
}
