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

#include "dialog.h"
#include "mainwindow.h"
#include "theme.h"
#include "progress.h"

DialogWindowContent::DialogWindowContent(Dialog* parent, const rect_t& rect) :
    ModalWindowContent(parent, rect),
    form(this, rect_t{})
{
  form.setFlexLayout();
  lv_obj_set_style_max_height(form.getLvObj(), LCD_H * 0.8, 0);
  lv_obj_set_style_pad_all(form.getLvObj(), lv_dpx(8), 0);
  lv_obj_set_scrollbar_mode(form.getLvObj(), LV_SCROLLBAR_MODE_AUTO);
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

//-----------------------------------------------------------------------------

ProgressDialog::ProgressDialog(Window *parent, std::string title,
                               std::function<void()> onClose) :
    Dialog(parent, title, rect_t{}),
    progress(new Progress(&content->form, rect_t{})),
    onClose(std::move(onClose))
{
  progress->setHeight(LV_DPI_DEF / 4);

  content->setWidth(LCD_W * 0.8);
  content->updateSize();

  auto content_w = lv_obj_get_content_width(content->form.getLvObj());
  progress->setWidth(content_w);

  // disable canceling dialog
  setCloseWhenClickOutside(false);
}

void ProgressDialog::updateProgress(int percentage)
{
  progress->setValue(percentage);
  lv_refr_now(nullptr);
}

void ProgressDialog::closeDialog()
{
  deleteLater();
  onClose();
}
