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

#pragma once

#include "layout.h"

class HeaderIcon;

//-----------------------------------------------------------------------------

#if LCD_W == 800  // TODO: handle this better
static constexpr int TOPBAR_ZONE_WIDTH = 102;
#else
static LAYOUT_VAL_SCALED(TOPBAR_ZONE_WIDTH, 70)
#endif
static constexpr int MAX_TOPBAR_ZONES = (LCD_W - MENU_HEADER_BUTTONS_LEFT - 1 + TOPBAR_ZONE_WIDTH / 2) / TOPBAR_ZONE_WIDTH;

struct TopBarPersistentData {
  ZonePersistentData zones[MAX_TOPBAR_ZONES] FUNC(widget_is_active);
#if !defined(YAML_GENERATOR)
  void clearZone(int idx);
  void clear();
  const char* getWidgetName(int idx);
  void setWidgetName(int idx, const char* s);
  WidgetPersistentData* getWidgetData(int idx);
  bool hasWidget(int idx);
  bool isWidget(int idx, const char* s);
#endif
};

//-----------------------------------------------------------------------------

class SetupTopBarWidgetsPage : public Window
{
 public:
  explicit SetupTopBarWidgetsPage();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SetupTopBarWidgetsPage"; }
#endif

  void onClicked() override;
  void onCancel() override;
  void onEvent(event_t event)  override;
  void deleteLater(bool detach = true, bool trash = true) override;
};

//-----------------------------------------------------------------------------

class TopBar: public WidgetsContainer
{
 public:
  explicit TopBar(Window * parent);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "TopBar";
  }
#endif

  unsigned int getZonesCount() const override;

  rect_t getZone(unsigned int index) const override;

  void setVisible(float visible);
  void setEdgeTxButtonVisible(float visible);
  coord_t getVisibleHeight(float visible) const; // 0.0 -> 1.0

  void checkEvents() override;

  bool isTopBar() override { return true; }

  void removeWidget(unsigned int index) override;

  Widget* createWidget(unsigned int index, const WidgetFactory* factory) override;

  void create();

  void load();

  static LAYOUT_VAL_SCALED(HDR_DATE_XO, 48)

  static constexpr coord_t TOPBAR_ZONE_HEIGHT = EdgeTxStyles::MENU_HEADER_HEIGHT - 2 * PAD_THREE;

 protected:
  uint32_t lastRefresh = 0;
  HeaderIcon* headerIcon = nullptr;
};

//-----------------------------------------------------------------------------
