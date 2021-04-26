/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "view_main.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
//#include "model_select.h"
//#include "view_channels.h"
//#include "view_statistics.h"
#include "topbar_impl.h"
//#include "menu.h"
#include "view_main_menu.h"

#include "opentx.h"

ViewMain * ViewMain::_instance = nullptr;

ViewMain::ViewMain():
  Window(MainWindow::instance(), MainWindow::instance()->getRect(), NO_SCROLLBAR),
  topbar(dynamic_cast<TopbarImpl*>(TopbarFactory::create(this)))
{
  setPageWidth(getParent()->width());
  focusWindow = this;

  setFocusHandler([&](bool focus) {
      TRACE("[ViewMain] Focus %s",
            focus ? "gained" : "lost");
    });
}

ViewMain::~ViewMain()
{
}

void ViewMain::setTopbarVisible(float visible)
{
  topbar->setVisible(visible);
}

unsigned ViewMain::getMainViewsCount() const
{
  return views;
}

void ViewMain::setMainViewsCount(unsigned views)
{
  if (views > MAX_CUSTOM_SCREENS)
    views = MAX_CUSTOM_SCREENS;

  // update number of views
  this->views = views;

  // adjust current screen if needed
  if (g_model.view >= views) {
    setCurrentMainView(views - 1);
  }
  
  setInnerWidth(getParent()->width() * views);
}

coord_t ViewMain::getMainViewLeftPos(unsigned view) const
{
  return getParent()->width() * view;
}

rect_t ViewMain::getMainZone(rect_t zone, bool hasTopbar) const
{
  auto visibleHeight = topbar->getVisibleHeight(hasTopbar ? 1.0 : 0.0);
  zone.y += visibleHeight;
  zone.h -= visibleHeight;
  
  return zone;
}

unsigned ViewMain::getCurrentMainView() const
{
  return g_model.view;
}

void ViewMain::setCurrentMainView(unsigned view)
{
  if (view < getMainViewsCount()) {
    setScrollPositionX(view * pageWidth);
    TRACE("### switched to view #%i", g_model.view);
  }
}

void ViewMain::nextMainView()
{
  auto view = getCurrentMainView();
  if (++view >= getMainViewsCount())
    view = 0;

  setCurrentMainView(view);
}

void ViewMain::previousMainView()
{
  auto view = getCurrentMainView();
  if (view > 0)
    view--;  
  else
    view = getMainViewsCount() - 1;

  setCurrentMainView(view);
}

Topbar* ViewMain::getTopbar()
{
  return topbar;
}

static bool hasTopbar(unsigned view)
{
  if (view < sizeof(g_model.screenData)) {
    const auto& layoutData = g_model.screenData[view].layoutData;
    return layoutData.options[LAYOUT_OPTION_TOPBAR].value.boolValue;
  }

  return false;
}

void ViewMain::updateTopbarVisibility()
{
  // relative to left visible page
  int leftScroll = getScrollPositionX() % pageWidth;
  if (leftScroll == 0) {
    setTopbarVisible(hasTopbar(g_model.view));
    customScreens[g_model.view]->adjustLayout();
  }
  else {
    int  leftIdx     = getScrollPositionX() / pageWidth;
    bool leftTopbar  = hasTopbar(leftIdx);
    bool rightTopbar = hasTopbar(leftIdx+1);

    if (leftTopbar != rightTopbar) {

      float ratio = (float)leftScroll / (float)pageWidth;

      if (leftTopbar) {
        // scrolling from a screen with Topbar
        ratio = 1.0 - ratio;
      }
      else {
        // scrolling to a screen with Topbar
        // -> ratio is ok
      }

      setTopbarVisible(ratio);
      customScreens[leftIdx]->adjustLayout();
      customScreens[leftIdx+1]->adjustLayout();
    }
  }
}

void ViewMain::setScrollPositionX(coord_t value)
{
  Window::setScrollPositionX(value);
  topbar->setLeft(getScrollPositionX());

  // update page index
  g_model.view = getPageIndex();
  setFocus();

  // update Topbar
  updateTopbarVisibility();
}

void ViewMain::setScrollPositionY(coord_t value)
{
  // this one is not used yet, but could
  // usefull to place the screens upwards
  Window::setScrollPositionY(value);
  topbar->setTop(getScrollPositionY());
}

#if defined(HARDWARE_TOUCH)

//#define DEBUG_SLIDE

bool ViewMain::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  // prevent screen sliding over the next one (one screen at a time)
  if (slidingWindow == this) {
    if (prevSlideState != touchState.event) {
      if (touchState.event == TE_SLIDE_END) {
        startSlidePage = getCurrentMainView();
#if defined(DEBUG_SLIDE)
        TRACE("### startSlidePage = %d ###", startSlidePage);
#endif
      }
      prevSlideState = touchState.event;
    }
    else if (prevSlideState == TE_SLIDE_END){

      // let's check what would be the next position
      auto nextPos = (getScrollPositionX() - slideX);

      // if we would get over more than one page counting from slide-start
      if (abs(nextPos - getMainViewLeftPos(startSlidePage)) > pageWidth) {

#if defined(DEBUG_SLIDE)
        TRACE("### kill the move ###");
#endif
        // kill the movement and let "snap-to-view" finish the job
        prevSlideState   = TE_NONE;
        touchState.event = TE_NONE;
        touchState.lastDeltaX = 0;
        touchState.lastDeltaY = 0;
        return true;
      }
    }
  }

#if defined(DEBUG_SLIDE)
  TRACE("%sWindow[scrollX=%d, scrollY=%d]->onTouchSlide[x=%d, y=%d, startX=%d, startY=%d, slideX=%d, slideY=%d]",
        touchState.event == TE_SLIDE_END && slidingWindow ? "###" : "",
        getScrollPositionX(), getScrollPositionY(),
        x, y, startX, startY, slideX, slideY);
#endif

  return Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
}

bool ViewMain::onTouchEnd(coord_t x, coord_t y)
{
  if (Window::onTouchEnd(x,y)) return true;
  if (!hasFocus()) {
    setFocus();
  }
  else {
    openMenu();
  }
  return true;
}
#endif

#if defined(HARDWARE_KEYS)
void ViewMain::onEvent(event_t event)
{
  switch (event) {
    case EVT_KEY_BREAK(KEY_MODEL):
      killEvents(event);
      new ModelMenu();
      break;

    case EVT_KEY_BREAK(KEY_RADIO):
      killEvents(event);
      new RadioMenu();
      break;

    case EVT_KEY_LONG(KEY_TELEM):
      killEvents(event);
      new ScreenMenu();
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      killEvents(event);
      openMenu();
      break;

    case EVT_KEY_BREAK(KEY_PGDN):
      killEvents(event);
      nextMainView();
      break;

//TODO: these need to go away!
// -> board code should map the keys as required
#if defined(KEYS_GPIO_REG_UP)
    case EVT_KEY_BREAK(KEY_PGUP):
#else
    case EVT_KEY_LONG(KEY_PGDN):
#endif
      killEvents(event);
      previousMainView();
      break;

    case EVT_ROTARY_LEFT:
      // decrement
    case EVT_ROTARY_RIGHT:
      // increment
      if (customScreens[g_model.view]) {
        customScreens[g_model.view]->setFocus();
      }
      break;

      break;
  }
}
#endif

void ViewMain::openMenu()
{
  new ViewMainMenu(this);
}

void ViewMain::paint(BitmapBuffer * dc)
{
  TRACE_WINDOWS("### ViewMain::paint(offset_x=%d;offset_y=%d) ###",
        dc->getOffsetX(), dc->getOffsetY());

  OpenTxTheme::instance()->drawBackground(dc);

  if (g_model.view >= getMainViewsCount()) {
    g_model.view = 0;
  }
}
