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

#include "datastructs.h"
#include "screen_user_interface.h"
#include "radio_theme.h"

class ScreenSetupPage : public PageGroupItem
{
 public:
  ScreenSetupPage(unsigned customScreenIndex, PageDef& pageDef);

  void build(Window* form) override;

  void update(uint8_t index) override;

  bool isVisible() const override { return customScreens[customScreenIndex] != nullptr; }

  static constexpr int FIRST_SCREEN_OFFSET = 2;

 protected:
  unsigned customScreenIndex;
  Window* layoutOptions = nullptr;

  void clearLayoutOptions();
  void buildLayoutOptions();
};
