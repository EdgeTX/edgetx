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

#include "tabsgroup.h"
#include "mainwindow.h"
#include "view_main.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

#include "opentx.h" // TODO for constants...

#include <algorithm>

TabsGroupHeader::TabsGroupHeader(TabsGroup * parent, uint8_t icon):
  FormGroup(parent, { 0, 0, LCD_W, MENU_BODY_TOP }, OPAQUE),
#if defined(HARDWARE_TOUCH)
  back(this, { 0, 0, MENU_HEADER_BUTTON_WIDTH, MENU_HEADER_BUTTON_WIDTH }, ICON_BACK,
       [=]() -> uint8_t {
         parent->deleteLater();
         ViewMain::instance()->setFocus((SET_FOCUS_DEFAULT));
         return 1;
       }, NO_FOCUS),
#endif
  icon(icon),
  carousel(this, parent)
{
}

void TabsGroupHeader::paint(BitmapBuffer * dc)
{
  OpenTxTheme::instance()->drawMenuBackground(dc, icon, title);
}

TabsCarousel::TabsCarousel(Window * parent, TabsGroup * menu):
  Window(parent, { MENU_HEADER_BUTTONS_LEFT, 0, LCD_W - MENU_HEADER_BUTTONS_LEFT, MENU_HEADER_HEIGHT + 10 }, TRANSPARENT),
  menu(menu)
{
}

void TabsCarousel::updateInnerWidth()
{
  setInnerWidth(padding_left + MENU_HEADER_BUTTON_WIDTH * menu->tabs.size());
}

void TabsCarousel::paint(BitmapBuffer * dc)
{
  OpenTxTheme::instance()->drawMenuHeader(dc, menu->tabs, currentIndex);
}

#if defined(HARDWARE_TOUCH)
bool TabsCarousel::onTouchEnd(coord_t x, coord_t y)
{
  unsigned index = (x - padding_left) / MENU_HEADER_BUTTON_WIDTH;
  if (index >= menu->tabs.size()) {
    return false;
  }
  menu->setCurrentTab(index);
  setCurrentIndex(index);
  return true;
}
#endif

TabsGroup::TabsGroup(uint8_t icon):
  Window(MainWindow::instance(), { 0, 0, LCD_W, LCD_H }, OPAQUE),
  header(this, icon),
  body(this, { 0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT }, FORM_FORWARD_FOCUS)
{
}

TabsGroup::~TabsGroup()
{
  for (auto tab: tabs) {
    delete tab;
  }
}

void TabsGroup::addTab(PageTab * page)
{
  tabs.push_back(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
  header.carousel.updateInnerWidth();
  invalidate();
}

int TabsGroup::removeTab(PageTab * page)
{
  auto tabIter = std::find(tabs.begin(), tabs.end(), page);
  if (tabIter != tabs.end()) {
    auto idx = tabIter - tabs.begin();
    removeTab(idx);
    return idx;
  }

  return -1;
}

void TabsGroup::removeTab(unsigned index)
{
  if (currentTab == tabs[index]) {
    setCurrentTab(max<unsigned>(0, index - 1));
  }
  tabs.erase(tabs.begin() + index);
  header.carousel.updateInnerWidth();
  invalidate();
}

void TabsGroup::removeAllTabs()
{
  for (auto * tab: tabs) {
    delete tab;
  }
  tabs.clear();
  currentTab = nullptr;
  header.carousel.updateInnerWidth();
}

void TabsGroup::setVisibleTab(PageTab * tab)
{
  if (tab != currentTab) {
    clearFocus();
    body.clear();
#if defined(HARDWARE_TOUCH)
    Keyboard::hide();
#endif
    currentTab = tab;
    tab->build(&body);
    if (!focusWindow)
      setFocus(SET_FOCUS_DEFAULT);
    header.setTitle(tab->title.c_str());
    invalidate();
  }
}

// TODO: add a mechanism to trigger a rebuild of a PageTab
// -> rebuild():
//    - as in setVisibleTab() (clear & build)
//    - but without changing tab

void TabsGroup::checkEvents()
{
  Window::checkEvents();
  if (currentTab) {
    currentTab->checkEvents();
  }
}

#if defined(HARDWARE_KEYS)
void TabsGroup::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_PGDN)) {
    uint8_t current = header.carousel.getCurrentIndex() + 1;
    setCurrentTab(current >= tabs.size() ? 0 : current);
  }
#if defined(KEYS_GPIO_REG_UP)
  else if (event == EVT_KEY_BREAK(KEY_PGUP)) {
#else
  else if (event == EVT_KEY_LONG(KEY_PGDN)) {
#endif
    killEvents(event);
    uint8_t current = header.carousel.getCurrentIndex();
    setCurrentTab(current == 0 ? tabs.size() - 1 : current - 1);
  }
  else if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    ViewMain::instance()->setFocus(SET_FOCUS_DEFAULT);
    deleteLater();
  }
  else if (parent) {
    parent->onEvent(event);
  }
}
#endif

void TabsGroup::paint(BitmapBuffer * dc)
{
  dc->clear(DEFAULT_BGCOLOR);
}

#if defined(HARDWARE_TOUCH)
bool TabsGroup::onTouchEnd(coord_t x, coord_t y)
{
  Keyboard::hide();
  Window::onTouchEnd(x, y);
  return true;
}
#endif
