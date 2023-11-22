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
#include "theme.h"

PageHeader::PageHeader(Page * parent, uint8_t icon):
  FormWindow(parent, { 0, 0, LCD_W, MENU_HEADER_HEIGHT }, OPAQUE),
  icon(icon)
{
#if defined(HARDWARE_TOUCH)
  new Button(this, { 0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT },
             [=]() -> uint8_t {
               parent->onCancel();
               return 0;
             }, NO_FOCUS);
#endif
  title = new StaticText(this,
                         {PAGE_TITLE_LEFT, PAGE_TITLE_TOP,
                          LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                         "", 0, COLOR_THEME_PRIMARY2);
}

StaticText* PageHeader::setTitle2(std::string txt)
{
  if (title2 == nullptr) {
    title2 = new StaticText(this, 
                            {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                             LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                             "", 0, COLOR_THEME_PRIMARY2);
  }
  title2->setText(std::move(txt));
  return title2;
}

void PageHeader::paint(BitmapBuffer * dc)
{
  EdgeTxTheme::instance()->drawPageHeaderBackground(dc, getIcon(), nullptr);
  dc->drawSolidFilledRect(MENU_HEADER_HEIGHT, 0, LCD_W - MENU_HEADER_HEIGHT,
                          MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
}

static constexpr rect_t _get_body_rect()
{
  return { 0, MENU_HEADER_HEIGHT, LCD_W, LCD_H - MENU_HEADER_HEIGHT };
}

Page::Page(unsigned icon):
  NavWindow(Layer::back(), {0, 0, LCD_W, LCD_H}, OPAQUE),
  header(this, icon),
  body(this, _get_body_rect())
{
  Layer::push(this);

  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY3), 0);

  body.padAll(0);
  lv_obj_set_scrollbar_mode(body.getLvObj(), LV_SCROLLBAR_MODE_AUTO);
}

void Page::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);

  header.deleteLater(true, false);
  body.deleteLater(true, false);
  Window::deleteLater(detach, trash);
}

void Page::onCancel()
{
  deleteLater();
}

void Page::onClicked() { Keyboard::hide(false); }
