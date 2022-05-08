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

#include "tabsgroup.h"

#include "mainwindow.h"
#include "view_main.h"
#include "static.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

#include "opentx.h" // TODO for constants...

#include <algorithm>


TabsGroupHeader::TabsGroupHeader(TabsGroup* parent, uint8_t icon) :
    FormGroup(parent, {0, 0, LCD_W, MENU_BODY_TOP}, NO_FOCUS | OPAQUE),
#if defined(HARDWARE_TOUCH)
    back(
        this,
        {0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT},
        [=]() -> uint8_t {
          parent->deleteLater();
          return 1;
        },
        NO_FOCUS | FORM_NO_BORDER),
#endif
    icon(icon),
    carousel(this, parent)
{
  lv_obj_set_style_border_width(lvobj, 0, LV_PART_MAIN);
}

void TabsGroupHeader::paint(BitmapBuffer* dc)
{
  OpenTxTheme::instance()->drawPageHeaderBackground(dc, icon, title);
}

TabsCarousel::TabsCarousel(Window* parent, TabsGroup* menu) :
    Window(parent,
           {MENU_HEADER_BUTTONS_LEFT, 0, LCD_W - MENU_HEADER_BUTTONS_LEFT,
            MENU_HEADER_HEIGHT + 10},
           NO_FOCUS | TRANSPARENT),
    menu(menu)
{
}

void TabsCarousel::updateInnerWidth()
{
}

void TabsCarousel::paint(BitmapBuffer * dc)
{
  OpenTxTheme::instance()->drawPageHeader(dc, menu->tabs, currentIndex);
}

#if defined(HARDWARE_TOUCH)
bool TabsCarousel::onTouchStart(coord_t x, coord_t y)
{
  if(sliding)
    sliding = false;

   return Window::onTouchStart(x,y);
}

bool TabsCarousel::onTouchEnd(coord_t x, coord_t y)
{
  if(sliding)
    return true;

  unsigned index = (x - padding_left) / MENU_HEADER_BUTTON_WIDTH;

  if (index >= menu->tabs.size()) {
    return false;
  }

  menu->setCurrentTab(index);
  setCurrentIndex(index);
  return true;
}

bool TabsCarousel::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  sliding = true;

  Window::onTouchSlide(x,y,startX,startY,slideX,slideY);

  return true;
}
#endif

TabsGroup::TabsGroup(uint8_t icon):
  Window(MainWindow::instance(), { 0, 0, LCD_W, LCD_H }, OPAQUE),
  header(this, icon),
  body(this, { 0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT }, NO_FOCUS | FORM_FORWARD_FOCUS)
{
  Layer::push(this);
}

TabsGroup::~TabsGroup()
{
  for (auto tab: tabs) {
    delete tab;
  }
}

void TabsGroup::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;

#if defined(HARDWARE_TOUCH)
  Keyboard::hide();
#endif

  header.deleteLater(true, false);
  body.deleteLater(true, false);

  Window::deleteLater(detach, trash);
  Layer::pop(this);
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

void TabsGroup::setVisibleTab(PageTab* tab)
{
  if (tab != currentTab) {
    clearFocus();
    body.clear();
#if defined(HARDWARE_TOUCH)
    Keyboard::hide();
#endif
    currentTab = tab;
    if (tab->onSetVisible) tab->onSetVisible();

#if defined(DEBUG)
    auto start_ms = RTOS_GET_MS();
    (void)start_ms;
#endif

    auto form = new FormWindow(&body, rect_t{0, 0, body.width(), body.height()},
                               NO_FOCUS | FORM_FORWARD_FOCUS);
    tab->build(form);
    form->setFocus();

    header.setTitle(tab->title.c_str());
    invalidate();
#if defined(DEBUG)
    TRACE("tab time: %d ms", RTOS_GET_MS() - start_ms);
#endif
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

#if defined(KEYS_GPIO_REG_PGUP)
  if (event == EVT_KEY_FIRST(KEY_PGDN)) {
#else
  if (event == EVT_KEY_BREAK(KEY_PGDN)) {
#endif
    killEvents(event);
    uint8_t current = header.carousel.getCurrentIndex() + 1;
    setCurrentTab(current >= tabs.size() ? 0 : current);
  }
#if defined(KEYS_GPIO_REG_PGUP)
  else if (event == EVT_KEY_FIRST(KEY_PGUP)) {
#else
  else if (event == EVT_KEY_LONG(KEY_PGDN)) {
#endif
    killEvents(event);
    uint8_t current = header.carousel.getCurrentIndex();
    setCurrentTab(current == 0 ? tabs.size() - 1 : current - 1);
  }
  else if (event == EVT_KEY_FIRST(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  } else if (parent) {
    parent->onEvent(event);
  }
}
#endif

void TabsGroup::paint(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
}

#if defined(HARDWARE_TOUCH)
bool TabsGroup::onTouchEnd(coord_t x, coord_t y)
{
  Keyboard::hide();
  Window::onTouchEnd(x, y);
  return true;
}
#endif
