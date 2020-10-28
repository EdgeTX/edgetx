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

#include <cinttypes>

typedef int coord_t;

struct point_t
{
  coord_t x;
  coord_t y;

  bool operator != (const point_t & b) const
  {
    return x != b.x || y != b.y;
  }
};

struct rect_t
{
  coord_t x, y, w, h;

  constexpr coord_t left() const
  {
    return x;
  }

  constexpr coord_t right() const
  {
    return x + w;
  }

  constexpr coord_t top() const
  {
    return y;
  }

  constexpr coord_t bottom() const
  {
    return y + h;
  }

  bool contains(coord_t x, coord_t y) const
  {
    return (x >= this->x && x < this->x + this->w && y >= this->y && y < this->y + this->h);
  }

  bool contains(const rect_t & other) const
  {
    return left() <= other.left() && right() >= other.right() && top() <= other.top() && bottom() >= other.bottom();
  }
};

typedef uint32_t LcdFlags;
typedef uint16_t event_t;

