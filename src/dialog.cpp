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
    form(this,
        { 0, POPUP_HEADER_HEIGHT, rect.w,
          coord_t(rect.h - POPUP_HEADER_HEIGHT) },
        FORM_NO_BORDER)
{
  form.setFocus(SET_FOCUS_DEFAULT);
}

void DialogWindowContent::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  form.deleteLater(true, false);
  ModalWindowContent::deleteLater(detach, trash);
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
