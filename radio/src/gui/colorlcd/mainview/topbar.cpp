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

#include "topbar.h"

#include "edgetx.h"
#include "storage/storage.h"
#include "etx_lv_theme.h"
#include "topbar.h"
#include "view_main.h"
#include "widgets_setup.h"
#include "pagegroup.h"
#include "theme_manager.h"
#include "widget.h"

//-----------------------------------------------------------------------------

void TopBarPersistentData::clearZone(int idx)
{
  zones[idx].clear();
}

void TopBarPersistentData::clear()
{
  for (int i = 0; i < MAX_TOPBAR_ZONES; i += 1)
    clearZone(i);
}

const char* TopBarPersistentData::getWidgetName(int idx)
{
  return zones[idx].widgetName.c_str();
}

void TopBarPersistentData::setWidgetName(int idx, const char* s)
{
  zones[idx].widgetName = s;
}

WidgetPersistentData* TopBarPersistentData::getWidgetData(int idx)
{
  return &zones[idx].widgetData;
}

bool TopBarPersistentData::hasWidget(int idx)
{
  return !zones[idx].widgetName.empty();
}

bool TopBarPersistentData::isWidget(int idx, const char* s)
{
  return zones[idx].widgetName == s;
}

//-----------------------------------------------------------------------------

SetupTopBarWidgetsPage::SetupTopBarWidgetsPage() :
    Window(ViewMain::instance(), rect_t{})
{
  // remember focus
  pushLayer();

  auto viewMain = ViewMain::instance();

  // save current view & switch to 1st one
  viewMain->setCurrentMainView(0);

  // adopt the dimensions of the main view
  setRect(viewMain->getRect());

  auto topbar = viewMain->getTopbar();
  for (unsigned i = 0; i < topbar->getZonesCount(); i++) {
    auto rect = topbar->getZone(i);
    new SetupWidgetsPageSlot(this, rect, topbar, i);
  }

#if defined(HARDWARE_TOUCH)
  addBackButton();
#endif
}

void SetupTopBarWidgetsPage::onClicked()
{
  // block event forwarding (window is transparent)
}

void SetupTopBarWidgetsPage::onCancel() { deleteLater(); }

void SetupTopBarWidgetsPage::deleteLater()
{
  if (_deleted) return;

  // and continue async deletion...
  Window::deleteLater();

  // restore screen setting tab on top
  QuickMenu::openPage(QM_UI_SETUP);

  storageDirty(EE_MODEL);
}

void SetupTopBarWidgetsPage::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_FIRST(KEY_PAGEUP) || event == EVT_KEY_FIRST(KEY_PAGEDN) ||
      event == EVT_KEY_FIRST(KEY_SYS) || event == EVT_KEY_FIRST(KEY_MODEL)) {
    killEvents(event);
  } else if (event == EVT_KEY_FIRST(KEY_TELE)) {
    onCancel();
  } else {
    Window::onEvent(event);
  }
#else
  Window::onEvent(event);
#endif
}

//-----------------------------------------------------------------------------

constexpr uint32_t TOPBAR_REFRESH = 1000 / 10; // 10 Hz

TopBar::TopBar(Window * parent) :
  WidgetsContainer(parent, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT}, MAX_TOPBAR_ZONES)
{
  setWindowFlag(NO_FOCUS);
  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

  headerIcon = new HeaderIcon(parent, ICON_EDGETX, [=]() { ViewMain::instance()->openMenu(); });
}

unsigned int TopBar::getZonesCount() const
{
  unsigned int zc = 0;
  for (int i = 0; i < zoneCount; i += 1)
    if (g_model.topbarWidgetWidth[i] > 0)
      zc += 1;
  return zc;
}

rect_t TopBar::getZone(unsigned int index) const
{
  coord_t x = MENU_HEADER_BUTTONS_LEFT + 1;

  for (unsigned int i = 0; i < index; i += 1)
    x += (g_model.topbarWidgetWidth[i] * (TOPBAR_ZONE_WIDTH + PAD_TINY));

  coord_t size = ((g_model.topbarWidgetWidth[index] - 1) * (TOPBAR_ZONE_WIDTH + PAD_TINY) + TOPBAR_ZONE_WIDTH);

  if ((x + size) > LCD_W) size = LCD_W - x;

  return {x, PAD_THREE, size, TOPBAR_ZONE_HEIGHT};
}

void TopBar::setVisible(float visible) // 0.0 -> 1.0
{
  coord_t y = 0;
  if (visible == 0.0) {
    y = -EdgeTxStyles::MENU_HEADER_HEIGHT;
  } else if (visible > 0.0 && visible < 1.0){
    y = -(float)EdgeTxStyles::MENU_HEADER_HEIGHT * (1.0 - visible);
  }
  if (y != top()) setTop(y);
}

void TopBar::setEdgeTxButtonVisible(float visible) // 0.0 -> 1.0
{
  coord_t y = 0;
  if (visible == 0.0) {
    y = -EdgeTxStyles::MENU_HEADER_HEIGHT;
  } else if (visible > 0.0 && visible < 1.0){
    y = -(float)EdgeTxStyles::MENU_HEADER_HEIGHT * (1.0 - visible);
  }
  if (y != headerIcon->top()) headerIcon->setTop(y);
}

coord_t TopBar::getVisibleHeight(float visible) const // 0.0 -> 1.0
{
  if (visible == 0.0) {
    return 0;
  }
  else if (visible == 1.0) {
    return EdgeTxStyles::MENU_HEADER_HEIGHT;
  }

  float h = (float)EdgeTxStyles::MENU_HEADER_HEIGHT * visible;
  return (coord_t)h;
}

void TopBar::checkEvents()
{
  uint32_t now = lv_tick_get();
  if (now - lastRefresh >= TOPBAR_REFRESH) {
    lastRefresh = now;
    WidgetsContainer::checkEvents();
  }
}

void TopBar::removeWidget(unsigned int index)
{
  if (index >= zoneCount) return;

  bool mark = false;

  // If user manually removes 'system' widgets, mark name so widget does not get reloaded on restart
  if ((index == (unsigned int)(zoneCount - 1)) && g_model.getTopbarData()->isWidget(index, "Date Time"))
    mark = true;
  if ((index == (unsigned int)(zoneCount - 2)) && g_model.getTopbarData()->isWidget(index, "Radio Info"))
    mark = true;
#if defined(INTERNAL_GPS)
  if ((index == (unsigned int)(zoneCount - 3)) && g_model.getTopbarData()->isWidget(index, "Internal GPS"))
    mark = true;
#endif

  // If user manually removes 'system' widgets, mark name so widget does not get reloaded on restart
  if (mark)
    g_model.getTopbarData()->setWidgetName(index, "---");

  g_model.getTopbarData()->clearZone(index);

  WidgetsContainer::removeWidget(index);
}

void TopBar::load()
{
  unsigned int count = getZonesCount();
  for (unsigned int i = 0; i < count; i++) {
    // remove old widget
    if (widgets[i]) {
      widgets[i]->deleteLater();
      widgets[i] = nullptr;
    }
  }

  for (unsigned int i = 0; i < count; i++) {
    // and load new one if required
    if (g_model.getTopbarData()->hasWidget(i)) {
      widgets[i] = WidgetFactory::newWidget(g_model.getTopbarData()->getWidgetName(i), this, getZone(i), -1, i);
    }
  }
}

Widget* TopBar::createWidget(unsigned int index,
                      const WidgetFactory* factory)
{
  if (index >= zoneCount) return nullptr;

  // remove old one if existing
  removeWidget(index);

  Widget* widget = nullptr;
  if (factory) {
    g_model.getTopbarData()->setWidgetName(index, factory->getName());
    widget = factory->create(this, getZone(index), -1, index);
  }
  widgets[index] = widget;

  return widget;
}

void TopBar::create()
{
  g_model.getTopbarData()->clear();
}

//-----------------------------------------------------------------------------
