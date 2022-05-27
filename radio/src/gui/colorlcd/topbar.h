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

constexpr coord_t TOPBAR_ZONE_WIDTH  = 70;
constexpr coord_t TOPBAR_ZONE_MARGIN = 3;
constexpr coord_t TOPBAR_HEIGHT = MENU_HEADER_HEIGHT - 2 * TOPBAR_ZONE_MARGIN;

class ScreenMenu;
class TopbarImpl;

class Topbar
{
 public:
  virtual void load() = 0;
};

class TopbarFactory
{
 public:
  static Topbar* create(Window* parent);
};

class SetupTopBarWidgetsPage : public FormWindow
{
 public:
  explicit SetupTopBarWidgetsPage(ScreenMenu* menu);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SetupTopBarWidgetsPage"; }
#endif

  void onClicked() override;
  void onCancel() override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  ScreenMenu* menu;
};
