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

#include "form.h"
#include "bitmaps.h"

constexpr coord_t FAB_BUTTON_WIDTH = 80;
constexpr coord_t FAB_BUTTON_HEIGHT = 114;

class SelectFabCarousel : public Window
{
 public:
  explicit SelectFabCarousel(Window* parent);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SelectFabCarousel"; }
#endif

  // Add a new button to the carousel
  void addButton(EdgeTxIcon icon, const char* title,
                 std::function<uint8_t(void)> pressHandler);
};
