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

#include "topbar_impl.h"
#include "edgetx.h"
#include "theme_manager.h"
#include "view_main.h"

constexpr uint32_t TOPBAR_REFRESH = 1000 / 10; // 10 Hz

class TopBarEdgeTx : public HeaderIcon
{
 public:
  TopBarEdgeTx(Window* parent) : HeaderIcon(parent, ICON_EDGETX)
  {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
  }

  void onClicked() override
  {
    ViewMain::instance()->openMenu();
  }
};

TopBar::TopBar(Window * parent) :
  TopBarBase(parent, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT}, &g_model.topbarData)
{
  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

  headerIcon = new TopBarEdgeTx(parent);
}

unsigned int TopBar::getZonesCount() const
{
  unsigned int zoneCount = 0;
  for (int i = 0; i < MAX_TOPBAR_ZONES; i += 1)
    if (g_model.topbarWidgetWidth[i] > 0)
      zoneCount += 1;
  return zoneCount;
}

rect_t TopBar::getZone(unsigned int index) const
{
  coord_t x = MENU_HEADER_BUTTONS_LEFT + 1;

  for (unsigned int i = 0; i < index; i += 1)
    x += (g_model.topbarWidgetWidth[i] * (TOPBAR_ZONE_WIDTH + PAD_TINY));

  coord_t size = ((g_model.topbarWidgetWidth[index] - 1) * (TOPBAR_ZONE_WIDTH + PAD_TINY) + TOPBAR_ZONE_WIDTH);

  if ((x + size) > LCD_W) size = LCD_W - x;

  return {x, TOPBAR_ZONE_VMARGIN, size, TOPBAR_ZONE_HEIGHT};
}

void TopBar::setVisible(float visible) // 0.0 -> 1.0
{
  if (visible == 0.0) {
    setTop(-(int)EdgeTxStyles::MENU_HEADER_HEIGHT);
  }
  else if (visible == 1.0) {
    setTop(0);
  }
  else if (visible > 0.0 && visible < 1.0){
    float top = - (float)EdgeTxStyles::MENU_HEADER_HEIGHT * (1.0 - visible);
    setTop((coord_t)top);
  }
}

void TopBar::setEdgeTxButtonVisible(float visible) // 0.0 -> 1.0
{
  if (visible == 0.0) {
    headerIcon->setTop(-(int)EdgeTxStyles::MENU_HEADER_HEIGHT);
  }
  else if (visible == 1.0) {
    headerIcon->setTop(0);
  }
  else if (visible > 0.0 && visible < 1.0){
    float top = - (float)EdgeTxStyles::MENU_HEADER_HEIGHT * (1.0 - visible);
    headerIcon->setTop((coord_t)top);
  }
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
  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= TOPBAR_REFRESH) {
    lastRefresh = now;
    TopBarBase::checkEvents();
  }
}

void TopBar::removeWidget(unsigned int index)
{
  bool mark = false;

  // If user manually removes 'system' widgets, mark name so widget does not get reloaded on restart
  if ((index == MAX_TOPBAR_ZONES - 1) && (strncmp(persistentData->zones[index].widgetName, "Date Time", WIDGET_NAME_LEN) == 0))
    mark = true;
  if ((index == MAX_TOPBAR_ZONES - 2) && (strncmp(persistentData->zones[index].widgetName, "Radio Info", WIDGET_NAME_LEN) == 0))
    mark = true;
#if defined(INTERNAL_GPS)
  if ((index == MAX_TOPBAR_ZONES - 3) && (strncmp(persistentData->zones[index].widgetName, "Internal GPS", WIDGET_NAME_LEN) == 0))
    mark = true;
#endif
  
  WidgetsContainerImpl::removeWidget(index);

  // If user manually removes 'system' widgets, mark name so widget does not get reloaded on restart
  if (mark)
    strcpy(persistentData->zones[index].widgetName, "--");
}
