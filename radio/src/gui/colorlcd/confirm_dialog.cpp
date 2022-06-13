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

ConfirmDialog::ConfirmDialog(Window * parent, const char * title, const char * message, std::function<void(void)> confirmHandler) :
  Dialog(parent, title, {LCD_W / 10 , LCD_H / 4, LCD_W - 2 * LCD_W / 10, LCD_H - 2 * LCD_H / 4}),
  confirmHandler(std::move(confirmHandler))
{
  auto form = &content->form;
  FormGridLayout grid(content->form.width());
  form->clear();

  new StaticText(form, grid.getCenteredSlot(), message);
  grid.setLabelWidth(15);
  grid.setMarginRight(15);
  grid.nextLine();
  grid.nextLine();

  auto noButton = new TextButton(form, grid.getFieldSlot(2, 0), STR_NO, [=]() -> int8_t {
      this->deleteLater();
      return 0;
  });

  new TextButton(form, grid.getFieldSlot(2, 1), STR_YES, [=]() -> int8_t {
      this->deleteLater();
      this->confirmHandler();
      return 0;
  });

  // noButton->setFocus(SET_FOCUS_DEFAULT);
}
