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

#include "theme_manager.h"
#include "etx_lv_theme.h"
#include "view_main.h"

PageHeader::PageHeader(Window* parent, EdgeTxIcon icon) :
    Window(parent, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT})
{
  setWindowFlag(NO_FOCUS | OPAQUE);

  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

  new HeaderIcon(this, icon);

  title = new StaticText(this,
                         {PAGE_TITLE_LEFT, PAGE_TITLE_TOP,
                          LCD_W - PAGE_TITLE_LEFT, EdgeTxStyles::PAGE_LINE_HEIGHT},
                         "", COLOR_THEME_PRIMARY2_INDEX);
}

PageHeader::PageHeader(Window* parent, const char* iconFile) :
    Window(parent, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT})
{
  setWindowFlag(NO_FOCUS | OPAQUE);

  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

  new HeaderIcon(this, iconFile);

  title = new StaticText(this,
                         {PAGE_TITLE_LEFT, PAGE_TITLE_TOP,
                          LCD_W - PAGE_TITLE_LEFT, EdgeTxStyles::PAGE_LINE_HEIGHT},
                         "", COLOR_THEME_PRIMARY2_INDEX);
}

StaticText* PageHeader::setTitle2(std::string txt)
{
  if (title2 == nullptr) {
    title2 = new StaticText(this,
                            {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + EdgeTxStyles::PAGE_LINE_HEIGHT,
                             LCD_W - PAGE_TITLE_LEFT, EdgeTxStyles::PAGE_LINE_HEIGHT},
                            "", COLOR_THEME_PRIMARY2_INDEX);
  }
  title2->setText(std::move(txt));
  return title2;
}

Page::Page(EdgeTxIcon icon, PaddingSize padding, bool pauseRefresh) :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  if (pauseRefresh)
    lv_obj_enable_style_refresh(false);

  header = new PageHeader(this, icon);
  body = new Window(this,
                    {0, EdgeTxStyles::MENU_HEADER_HEIGHT, LCD_W, LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT});
  body->setWindowFlag(NO_FOCUS);

  etx_solid_bg(lvobj);
  lv_obj_set_style_max_height(body->getLvObj(), LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT,
                              LV_PART_MAIN);
  etx_scrollbar(body->getLvObj());

  Layer::back()->hide();
  Layer::push(this);

  body->padAll(padding);

#if defined(HARDWARE_TOUCH)
  addBackButton();
#endif
}

void Page::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  Layer::back()->show();

  Window::deleteLater(detach, trash);
}

void Page::onCancel() { deleteLater(); }

void Page::onClicked() { Keyboard::hide(false); }

void Page::checkEvents()
{
  ViewMain::instance()->runBackground();
  NavWindow::checkEvents();
}

void Page::enableRefresh()
{
  lv_obj_enable_style_refresh(true);
  lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
}

SubPage::SubPage(EdgeTxIcon icon, const char* title, const char* subtitle, bool pauseRefresh) :
  Page(icon, PAD_SMALL, pauseRefresh)
{
  body->padBottom(PAD_LARGE * 2);

  header->setTitle(title);
  header->setTitle2(subtitle);
}

SubPage::SubPage(EdgeTxIcon icon, const char* title, const char* subtitle, SetupLineDef* setupLines, int lineCount) :
  Page(icon, PAD_SMALL, true)
{
  body->padBottom(PAD_LARGE * 2);

  header->setTitle(title);
  header->setTitle2(subtitle);

  SetupLine::showLines(body, y, EDT_X, PAD_SMALL, setupLines, lineCount);

  enableRefresh();
}

Window* SubPage::setupLine(const char* title, std::function<void(Window*, coord_t, coord_t)> createEdit, coord_t lblYOffset)
{
  auto w = new SetupLine(body, y, EDT_X, PAD_SMALL, title, createEdit, lblYOffset);
  y += w->height();
  return w;
}
