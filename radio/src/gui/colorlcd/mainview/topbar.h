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

class ScreenMenu;
class TopBar;

class TopbarFactory
{
 public:
  static TopBar* create(Window* parent);
};

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
