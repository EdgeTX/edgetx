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
#include "static.h"
#include "theme.h"
#include "view_main.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

#include <algorithm>

#include "opentx.h"  // TODO for constants...

TabCarouselButton::TabCarouselButton(Window* parent, const rect_t& rect,
                                     std::vector<PageTab*>& tabs, uint8_t index,
                                     std::function<uint8_t(void)> pressHandler,
                                     WindowFlags flags) :
    Button(parent, rect, std::move(pressHandler), flags, 0, window_create),
    tabs(tabs),
    index(index)
{
}

void TabCarouselButton::paint(BitmapBuffer* dc)
{
  EdgeTxTheme::instance()->drawMenuIcon(dc, tabs[index]->getIcon(), checked());
}

void TabCarouselButton::check(bool checked)
{
  Button::check(checked);
  if (checked) {
    lv_obj_move_foreground(lvobj);
  }
}

TabsGroupHeader::TabsGroupHeader(TabsGroup* parent, uint8_t icon) :
    FormWindow(parent, {0, 0, LCD_W, MENU_BODY_TOP}, NO_FOCUS | OPAQUE),
#if defined(HARDWARE_TOUCH)
    back(
        this,
        {0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT},
        [=]() -> uint8_t {
          parent->deleteLater();
          return 1;
        },
        NO_FOCUS, 0, window_create),
#endif
    icon(icon),
    carousel(this, parent)
{
  lv_obj_set_style_border_width(lvobj, 0, LV_PART_MAIN);
}

void TabsGroupHeader::paint(BitmapBuffer* dc)
{
  EdgeTxTheme::instance()->drawPageHeaderBackground(dc, icon, title.c_str());
}

TabsCarousel::TabsCarousel(Window* parent, TabsGroup* menu) :
    Window(parent,
           {MENU_HEADER_BUTTONS_LEFT, 0,
            DATETIME_SEPARATOR_X - MENU_HEADER_BUTTONS_LEFT,
            MENU_HEADER_HEIGHT + 10},
           NO_FOCUS | TRANSPARENT),
    menu(menu)
{
}

void TabsCarousel::update()
{
  while (buttons.size() < menu->tabs.size()) {
    int index = buttons.size();
    rect_t btnCoords = {(int)(index * (MENU_HEADER_BUTTON_WIDTH + 1)), 0,
                        (int)(MENU_HEADER_BUTTON_WIDTH + 3),
                        int(MENU_TITLE_TOP + 5)};
    buttons.emplace_back(new TabCarouselButton(
        this, btnCoords, menu->tabs, index,
        [&, index]() {
          menu->setCurrentTab(index);
          setCurrentIndex(index);

          for (auto& b : buttons) b->check(false);

          buttons[index]->check(true);
          return true;
        },
        TRANSPARENT | NO_FOCUS));
    if (index == 0) buttons[index]->check(true);
  }
  while (buttons.size() > menu->tabs.size()) {
    buttons.back()->deleteLater();
    buttons.pop_back();
  }
}

void TabsCarousel::setCurrentIndex(uint8_t index)
{
  if (buttons.size() <= index) return;
  buttons[currentIndex]->check(false);
  currentIndex = index;
  buttons[currentIndex]->check(true);
}

void TabsCarousel::paint(BitmapBuffer* dc) {}

static constexpr rect_t _get_body_rect()
{
  return {0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT};
}

TabsGroup* TabsGroup::activeTabsGroup = nullptr;

TabsGroup::TabsGroup(uint8_t icon) :
    NavWindow(Layer::back(), {0, 0, LCD_W, LCD_H}, OPAQUE),
    header(this, icon),
    body(this, _get_body_rect(), NO_FOCUS)
{
  Layer::push(this);

  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY3), 0);
  lv_obj_set_scrollbar_mode(body.getLvObj(), LV_SCROLLBAR_MODE_AUTO);

  activeTabsGroup = this;
}

TabsGroup::~TabsGroup()
{
  if (activeTabsGroup == this) activeTabsGroup = nullptr;

  for (auto tab : tabs) {
    delete tab;
  }
}

void TabsGroup::refreshTheme()
{
  if (activeTabsGroup)
    lv_obj_set_style_bg_color(activeTabsGroup->getLvObj(),
                              makeLvColor(COLOR_THEME_SECONDARY3), 0);
}

void TabsGroup::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  Layer::pop(this);

  header.deleteLater(true, false);
  body.deleteLater(true, false);

  Window::deleteLater(detach, trash);
}

void TabsGroup::addTab(PageTab* page)
{
  tabs.push_back(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
  header.carousel.update();
  invalidate();
}

int TabsGroup::removeTab(PageTab* page)
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
  header.carousel.update();
  invalidate();
}

void TabsGroup::removeAllTabs()
{
  for (auto* tab : tabs) {
    delete tab;
  }
  tabs.clear();
  currentTab = nullptr;
  header.carousel.update();
}

void TabsGroup::setVisibleTab(PageTab* tab)
{
  if (tab != currentTab) {
    body.clear();
    currentTab = tab;
    if (tab->onSetVisible) tab->onSetVisible();

#if defined(DEBUG)
    auto start_ms = RTOS_GET_MS();
    (void)start_ms;
#endif

    rect_t r = rect_t{0, 0, body.width(), body.height()};
    auto form = new FormWindow(&body, r, NO_FOCUS);

    form->padAll(lv_dpx(8));
    tab->build(form);

    header.setTitle(tab->title.c_str());
    header.invalidate();

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

  static uint32_t lastRefresh = 0;
  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= 5000) {
    lastRefresh = now;
    header.invalidate();
  }
}

#if defined(HARDWARE_KEYS)
void TabsGroup::onPressPGUP()
{
  uint8_t current = header.carousel.getCurrentIndex();
  setCurrentTab(current == 0 ? tabs.size() - 1 : current - 1);
}
void TabsGroup::onPressPGDN()
{
  uint8_t current = header.carousel.getCurrentIndex() + 1;
  setCurrentTab(current >= tabs.size() ? 0 : current);
}
#endif

void TabsGroup::onClicked() { Keyboard::hide(false); }

void TabsGroup::onCancel() { deleteLater(); }

#if defined(HARDWARE_TOUCH)
bool TabsGroup::onTouchEnd(coord_t x, coord_t y)
{
  // Keyboard::hide();
  Window::onTouchEnd(x, y);
  return true;
}
#endif
