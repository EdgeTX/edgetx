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

ConfirmDialog::ConfirmDialog(Window* parent, const char* title,
                             const char* message,
                             std::function<void(void)> confirmHandler,
                             std::function<void(void)> cancelHandler) :
    Dialog(parent, title, rect_t{}),
    confirmHandler(std::move(confirmHandler)),
    cancelHandler(std::move(cancelHandler))
{
  auto form = &content->form;
  auto msg = new StaticText(form, rect_t{}, message);
  msg->padAll(lv_dpx(16));

  lv_obj_set_style_bg_color(content->getLvObj(),
                            makeLvColor(COLOR_THEME_SECONDARY3), 0);

  auto box = new FormWindow(form, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW);
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY, 0);

  auto btn = new TextButton(box, rect_t{}, STR_NO, [=]() -> int8_t {
    onCancel();
    return 0;
  });
#if LCD_W > LCD_H
  btn->setWidth(LV_DPI_DEF);
#else
  btn->setWidth(LV_DPI_DEF * 3 / 4);
#endif

  btn = new TextButton(box, rect_t{}, STR_YES, [=]() -> int8_t {
    this->deleteLater();
    this->confirmHandler();
    return 0;
  });
#if LCD_W > LCD_H
  btn->setWidth(LV_DPI_DEF);
#else
  btn->setWidth(LV_DPI_DEF * 3 / 4);
#endif

  content->setWidth(LCD_W * 0.8);
  content->updateSize();
}

void ConfirmDialog::onCancel()
{
  deleteLater();
  if (cancelHandler) cancelHandler();
}
