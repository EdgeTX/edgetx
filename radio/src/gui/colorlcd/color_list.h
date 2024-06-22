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

#include <vector>

#include "listbox.h"
#include "theme_manager.h"

// a list of color theme names with their color to the right
class ColorList : public ListBox
{
 public:
  ColorList(Window* parent, const rect_t& rect, std::vector<ColorEntry> colors);

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
    ListBox::setNames(getColorListNames(colorList));
  }

 protected:
  std::vector<ColorEntry> _colorList;

  void onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc);
};

class ColorSwatch : public Window
{
 public:
  ColorSwatch(Window* window, const rect_t& rect, uint32_t color);

  void setColor(uint32_t colorEntry);
  void setColor(uint8_t r, uint8_t g, uint8_t b);

 protected:
};
