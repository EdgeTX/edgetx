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

#include "page.h"
#include "mainwindow.h"
#include "keyboard_base.h"
#include "opentx.h"

PageHeader::PageHeader(Page * parent, uint8_t icon):
  FormGroup(parent, { 0, 0, LCD_W, MENU_HEADER_HEIGHT }, OPAQUE),
  icon(icon)
{
#if defined(HARDWARE_TOUCH)
  new Button(this, { 0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT },
             [=]() -> uint8_t {
               parent->deleteLater();
               return 0;
             }, NO_FOCUS | FORM_NO_BORDER);
#endif
  title = new StaticText(this, rect_t{}, "", 0, COLOR_THEME_PRIMARY2);
  title->setTop(PAGE_TITLE_TOP);
  title->setLeft(PAGE_TITLE_LEFT);
}

void PageHeader::paint(BitmapBuffer * dc)
{
  OpenTxTheme::instance()->drawPageHeaderBackground(dc, getIcon(), "");
  dc->drawSolidFilledRect(MENU_HEADER_HEIGHT, 0, LCD_W - MENU_HEADER_HEIGHT,
                          MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
}

static constexpr rect_t _get_body_rect()
{
  return { 0, MENU_HEADER_HEIGHT, LCD_W, LCD_H - MENU_HEADER_HEIGHT };
}

Page::Page(unsigned icon):
  Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}, OPAQUE),
  header(this, icon),
  body(this, _get_body_rect(), FORM_FORWARD_FOCUS)
{
  Layer::push(this);
}

void Page::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);

  header.deleteLater(true, false);
  body.deleteLater(true, false);
  Window::deleteLater(detach, trash);
}

void Page::paint(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
}

void Page::onCancel()
{
  deleteLater();
}

void Page::onClicked()
{
  Keyboard::hide();
}
