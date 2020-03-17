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

#ifndef _LIBOPEUI_HELPERS_H_
#define _LIBOPEUI_HELPERS_H_

#include <string>
#include <cstring>

#if defined(min)
  #undef min
  #undef max
#endif

template<class T>
inline T min(T a, T b)
{
  return a < b ? a : b;
}

template<class T>
inline T max(T a, T b)
{
  return a > b ? a : b;
}

template<class T>
inline T limit(T vmin, T x, T vmax)
{
  return min(max(vmin, x), vmax);
}

inline int divRoundClosest(const int n, const int d)
{
  if (d == 0)
    return 0;
  else
    return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

// TODO duplicated code
inline std::string TEXT_AT_INDEX(const char * val, uint8_t idx)
{
  const char * start = val + 1 + idx * val[0];
  return std::string(start, min<uint8_t>(val[0], (uint8_t)strlen(start)));
}

#endif
