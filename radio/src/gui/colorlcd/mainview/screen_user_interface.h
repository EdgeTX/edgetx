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

#include "tabsgroup.h"
#include "widgets_container.h"

class ScreenMenu;
class ThemeView;
class Choice;

class ScreenUserInterfacePage : public PageTab
{
 public:
  ScreenUserInterfacePage(ScreenMenu* menu);

  void build(Window* window) override;

  static LAYOUT_VAL(WWBTN_W, 60, 60, LS(60))

 protected:
  ScreenMenu* menu;
  Choice* widths[MAX_TOPBAR_ZONES];
  ThemeView* themeView = nullptr;
};
