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

#ifndef _DMA_FIFO_H_
#define _DMA_FIFO_H_

#include "definitions.h"

template <int N>
class DMAFifo
{
  public:
    explicit DMAFifo(DMA_Stream_TypeDef * stream):
      stream(stream),
      ridx(0)
    {
    }

    void clear()
    {
      ridx = 0;
    }

    uint32_t size()
    {
      return N;
    }

    uint8_t last(int index)
    {
      return fifo[(2*N - stream->NDTR - index) & (N-1)];
    }

    bool isEmpty()
    {
#if defined(SIMU)
      return true;
#endif
      return (ridx == N - stream->NDTR);
    }

    bool pop(uint8_t & element)
    {
      if (isEmpty()) {
        return false;
      }
      else {
        element = fifo[ridx];
        ridx = (ridx+1) & (N-1);
        return true;
      }
    }

    uint8_t * buffer()
    {
      return fifo;
    }

  protected:
    uint8_t fifo[N];
    DMA_Stream_TypeDef * stream;
    volatile uint32_t ridx;
};

#endif // _DMA_FIFO_H_
