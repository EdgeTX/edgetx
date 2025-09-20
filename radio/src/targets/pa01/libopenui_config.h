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

<<<<<<<< HEAD:radio/src/targets/pa01/libopenui_config.h
constexpr coord_t INPUT_EDIT_CURVE_WIDTH = 132;
constexpr coord_t INPUT_EDIT_CURVE_HEIGHT = INPUT_EDIT_CURVE_WIDTH;
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 8) + 8;

========
#include "pagegroup.h"

class ModelMenu : public PageGroup
{
 public:
  ModelMenu();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ModelMenu"; }
#endif
};
>>>>>>>> 63f64f877 (New quick menu for 3.0.):radio/src/gui/colorlcd/setup_menus/menu_model.h
