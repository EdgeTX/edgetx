/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
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
