/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "opentx.h"
#include "confirm_dialog.h"
#include "static.h"
#include "gridlayout.h"

ConfirmDialog::ConfirmDialog(Window* parent, const char* title,
                             const char* message,
                             std::function<void(void)> confirmHandler) :
    Dialog(parent, title, rect_t{}),
    confirmHandler(std::move(confirmHandler))
{
  auto form = &content->form;
  auto msg = new StaticText(form, rect_t{}, message);
  msg->padAll(lv_dpx(16));

  auto box = new FormGroup(form, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW);

  auto box_obj = box->getLvObj();
  lv_obj_set_style_flex_main_place(box_obj, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
  
  auto btn = new TextButton(box, rect_t{}, STR_NO, [=]() -> int8_t {
    this->deleteLater();
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), LV_DPI_DEF);

  btn = new TextButton(box, rect_t{}, STR_YES, [=]() -> int8_t {
    this->deleteLater();
    this->confirmHandler();
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), LV_DPI_DEF);

  content->setWidth(LCD_W * 0.8);
  content->updateSize();
}
