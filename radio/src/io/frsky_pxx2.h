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

#ifndef _IO_PXX2_H_
#define _IO_PXX2_H_

#include "fifo.h"
#include "pulses/pxx.h"

#include "board.h"

class ModuleFifo : public Fifo<uint8_t, INTMODULE_FIFO_SIZE> {
  public:
    bool getFrame(uint8_t * frame)
    {
      while (true) {
        if (isEmpty()) {
          return false;
        }
        else if (fifo[ridx] != 0x7E) {
          skip();
        }
        else {
          break;
        }
      }

      uint32_t next = nextIndex(ridx);
      uint8_t len = fifo[next];

      if (len > 40) {
        clear();
        return false;
      }

      if (size() < unsigned(len + 4 /* 2 bytes header + 2 bytes CRC */)) {
        // frame not fully received
        return false;
      }

      // we keep the length in frame[0]
      frame[0] = fifo[next];
      next = nextIndex(next);

      uint16_t crc = 0xFFFF;

      for (uint32_t i = 1; i <= len; i++) {
        frame[i] = fifo[next];
        crc -= frame[i];
        next = nextIndex(next);
      }

      uint8_t crcLow = fifo[next];
      next = nextIndex(next);
      uint8_t crcHigh = fifo[next];
      ridx = nextIndex(next);

      return ((crc >> 8) == crcLow) && ((crc & 0xFF) == crcHigh);
    }

};

#endif
