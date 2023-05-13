/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

class Slider : public FormField
{
 public:
  Slider(Window* parent, const rect_t& rect, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Slider"; }
#endif

  void setValue(int value)
  {
    if (_setValue != nullptr) _setValue(limit(vmin, value, vmax));
  }

 protected:
  int vmin;
  int vmax;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;

  int value(coord_t x) const;
};
