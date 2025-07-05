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

#include <cinttypes>

typedef int coord_t;

#if defined(COLORLCD)
typedef uint16_t pixel_t;
#else
typedef uint8_t pixel_t;
#endif

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

typedef uint32_t tmr10ms_t;
typedef int32_t rotenc_t;
typedef int32_t getvalue_t;
typedef int32_t mixsrc_t;
typedef int32_t swsrc_t;
typedef int16_t safetych_t;
typedef int16_t gvar_t;
typedef uint32_t bitfield_channels_t;
typedef uint16_t FlightModesType;
typedef uint16_t source_t;

// 2 bits per switch, max 32 switches
typedef uint64_t swconfig_t;
typedef uint64_t swarnstate_t;

#if defined(COLORLCD)
  // pot config: 4 bits per pot
  typedef uint64_t potconfig_t;

  // pot warning enabled: 1 bit per pot
  typedef uint16_t potwarnen_t;
#else
  // pot config: 4 bits per pot
  typedef uint32_t potconfig_t;

  // pot warning enabled: 1 bit per pot
  typedef uint8_t potwarnen_t;
#endif
