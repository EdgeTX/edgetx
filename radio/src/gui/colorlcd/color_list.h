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
#include <algorithm>
#include <vector>
#include <iostream>
#include "bitmapbuffer.h"
#include "libopenui.h"
#include "theme_manager.h"
#include "listbox.h"

// a list of color theme names with their color to the right
class ColorList : public ListBase
{
 public:
  ColorList(Window *parent, const rect_t &rect, std::vector<ColorEntry> colors,
            WindowFlags windowFlags = 0, LcdFlags lcdFlags = 0);

  inline ColorEntry getSelectedColor()
  {
    auto idx = getSelected();
    if (idx < 0) return ColorEntry{};
    return _colorList[idx];
  }

  std::vector<std::string> getColorListNames(std::vector<ColorEntry> colors);

  inline void setColorList(std::vector<ColorEntry> colorList)
  {
    this->_colorList = colorList;
    ListBase::setNames(getColorListNames(colorList));
    invalidate();
  }

 protected:
  std::vector<ColorEntry> _colorList;
  ThemePersistance *_tp;

  void onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc);
};

