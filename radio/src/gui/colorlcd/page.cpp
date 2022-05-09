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
#if defined(HARDWARE_TOUCH)
  , back(this, { 0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT },
       [=]() -> uint8_t {
         parent->deleteLater();
         return 0;
       }, NO_FOCUS | FORM_NO_BORDER)
#endif
{
}

void PageHeader::paint(BitmapBuffer * dc)
{
  OpenTxTheme::instance()->drawPageHeaderBackground(dc, getIcon(), "");
  dc->drawSolidFilledRect(MENU_HEADER_HEIGHT, 0, LCD_W - MENU_HEADER_HEIGHT,
                          MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
}

Page::Page(unsigned icon):
  Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}, OPAQUE),
  header(this, icon),
  body(this, { 0, MENU_HEADER_HEIGHT, LCD_W, LCD_H - MENU_HEADER_HEIGHT }, FORM_FORWARD_FOCUS)
{
  Layer::push(this);
  clearFocus();
}

void Page::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);

  header.deleteLater(true, false);
  body.deleteLater(true, false);

#if defined(HARDWARE_TOUCH)
  Keyboard::hide();
#endif

  Window::deleteLater(detach, trash);
}

void Page::setFocus(uint8_t flag, Window * from)
{
  body.setFocus(flag, from);
}

void Page::paint(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
}

#if defined(HARDWARE_KEYS)
void Page::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Page::onTouchEnd(coord_t x, coord_t y)
{
#if defined (SOFTWARE_KEYBOARD)
  Keyboard::hide();
#endif

  Window::onTouchEnd(x, y);
  return true;
}
#endif
