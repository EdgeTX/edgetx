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
  return ::min(::max(vmin, x), vmax);
}

inline int divRoundClosest(int n, int d)
{
  if (d == 0)
    return 0;
  else
    return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

inline int multDivRoundClosest(int v, int n, int d)
{
  if (n == d)
    return v;
  else
    return divRoundClosest(v * n, d);
}

inline int mod(int k, int n)
{
  return ((k %= n) < 0) ? k + n : k;
}

inline unsigned align32(unsigned n)
{
  unsigned rest = (n & 3u);
  return rest ? n + 4u - rest : n;
}

template<class T>
inline T sgn(T a)
{
  return a > 0 ? 1 : (a < 0 ? -1 : 0);
}

// TODO duplicated code
inline std::string TEXT_AT_INDEX(const char *const *val, uint8_t idx)
{
  return std::string(val[idx]);
}

inline const char * findNextLine(const char * stack)
{
  while (true) {
    const char * pos = strchr(stack, '\n');
    if (!pos)
      return nullptr;
    if (pos == stack || *((uint8_t *)(pos - 1)) < 0xFE)
      return pos;
    stack = pos + 1;
  }
}
