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

#include "window.h"

class Widget;
class WidgetFactory;

class WidgetsContainer: public Window
{
 public:
  WidgetsContainer(Window* parent, const rect_t& rect, uint8_t zoneCount);

  virtual unsigned int getZonesCount() const = 0;
  virtual rect_t getZone(unsigned int index) const = 0;
  virtual Widget * createWidget(unsigned int index, const WidgetFactory * factory) = 0;

  Widget* getWidget(unsigned int index);
  virtual void removeWidget(unsigned int index);
  void removeAllWidgets();
  void updateZones();
  void showWidgets(bool visible = true);
  void hideWidgets() { showWidgets(false); }
  void refreshWidgets(bool inForeground);

  virtual bool isLayout() { return false; }
  virtual bool isAppMode() const { return false; }
  bool isWidgetsContainer() override { return true; }

  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  uint8_t zoneCount = 0;
  Widget** widgets = nullptr;
};
