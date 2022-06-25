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

#ifndef _FIFO_H_
#define _FIFO_H_

#include <inttypes.h>

template <class T, int N>
class Fifo
{
  static_assert((N > 1) & !(N & (N - 1)), "Fifo size must be a power of two!");

  public:
    Fifo():
      widx(0),
      ridx(0)
    {
    }

    void clear()
    {
      widx = ridx = 0;
    }

    void push(T element)
    {
      uint32_t next = nextIndex(widx);
      if (next != ridx) {
        fifo[widx] = element;
        widx = next;
      }
    }

    void skip()
    {
      ridx = nextIndex(ridx);
    }

    bool pop(T & element)
    {
      if (isEmpty()) {
        return false;
      }
      else {
        element = fifo[ridx];
        ridx = nextIndex(ridx);
        return true;
      }
    }

    bool isEmpty() const
    {
      return (ridx == widx);
    }

    bool isFull() const
    {
      uint32_t next = nextIndex(widx);
      return (next == ridx);
    }

    uint32_t size() const
    {
      return (N + widx - ridx) & (N - 1);
    }

    bool hasSpace(uint32_t n) const
    {
      return (N > (size() + n));
    }

    bool probe(T & element) const
    {
      if (isEmpty()) {
        return false;
      }
      else {
        element = fifo[ridx];
        return true;
      }
    }

    T * buffer()
    {
      return fifo;
    }

  protected:
    T fifo[N];
    volatile uint32_t widx;
    volatile uint32_t ridx;

    static inline uint32_t nextIndex(uint32_t idx)
    {
      return (idx + 1) & (N - 1);
    }
};

#endif // _FIFO_H_
