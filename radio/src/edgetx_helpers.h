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

#include <string>
#include <cstring>
#include <cstdint>

template<class T>
inline T sgn(T a)
{
  return a > 0 ? 1 : (a < 0 ? -1 : 0);
}

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

inline int divRoundClosest(int n, int d)
{
  if (d == 0)
    return 0;
  else
    return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

inline unsigned align32(unsigned n)
{
  unsigned rest = (n & 3u);
  return rest ? n + 4u - rest : n;
}

inline std::string TEXT_AT_INDEX(const char *const *val, uint8_t idx)
{
  return std::string(val[idx]);
}

template<class T>
inline void SWAP(T & a, T & b)
{
  T tmp = b;
  b = a;
  a = tmp;
}

inline void memclear(void * p, size_t size)
{
  memset(p, 0, size);
}

inline bool is_memclear(void * p, size_t size)
{
  uint8_t * buf = (uint8_t *)p;
  return buf[0] == 0 && memcmp(buf, buf + 1, size - 1) == 0;
}

// Circular buffer to send data between two threads
template<class T, uint8_t N>
class CircularBuffer {
  protected:
    volatile T buffer[N] = {0};
    uint8_t readPos = 0;
    uint8_t writePos = 0;
    
  public:
    void clear()
    {
      memclear(this, sizeof(*this));
    }

    T read()
    {
      T value = buffer[readPos];
      if (value) {
        buffer[readPos] = 0;
        readPos = (readPos + 1) % N;
      }
      return value;
    }

    // Values must be non-zero
    bool write(T value)
    {
      if (value && !buffer[writePos]) {
        buffer[writePos] = value;
        writePos = (writePos + 1) % N;
        return false;
      }    
      return true;
    }  
};
