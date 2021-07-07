/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _PULSES_PXX_H_
#define _PULSES_PXX_H_

#include "pulses_common.h"

#define PXX_SEND_BIND                      0x01
#define PXX_SEND_FAILSAFE                  (1 << 4)
#define PXX_SEND_RANGECHECK                (1 << 5)

#define PXX2_LOWSPEED_BAUDRATE             230400
#define PXX2_HIGHSPEED_BAUDRATE            450000
#define PXX2_NO_HEARTBEAT_PERIOD           4000/*us*/
#define PXX2_MAX_HEARTBEAT_PERIOD          (9000 + 1000)/*us longest period (isrm/accst) + 1ms heartbeat backup*/
#define PXX2_TOOLS_PERIOD                  1000/*us*/
#define PXX2_FRAME_MAXLENGTH               64

#define PXX_PULSES_PERIOD                  9000/*us*/
#define EXTMODULE_PXX1_SERIAL_PERIOD       4000/*us*/
#define EXTMODULE_PXX1_SERIAL_BAUDRATE     420000

#if defined(PXX_FREQUENCY_HIGH)
  #define INTMODULE_PXX1_SERIAL_BAUDRATE   450000
  #define INTMODULE_PXX1_SERIAL_PERIOD     4000/*us*/
#else
  #define INTMODULE_PXX1_SERIAL_BAUDRATE   115200
  #define INTMODULE_PXX1_SERIAL_PERIOD     9000/*us*/
#endif

// Used by the Sky9x family boards
class SerialPxxBitTransport: public DataBuffer<uint8_t, 64> {
  protected:
    uint8_t byte;
    uint8_t bits_count;

    void initFrame(uint32_t period)
    {
      initBuffer();
      byte = 0;
      bits_count = 0;
    }

    void addSerialBit(uint8_t bit)
    {
      byte >>= 1;
      if (bit & 1) {
        byte |= 0x80;
      }
      if (++bits_count >= 8) {
        *ptr++ = byte;
        bits_count = 0;
      }
    }

    // 8uS/bit 01 = 0, 001 = 1
    void addPart(uint8_t value)
    {
      addSerialBit(0);
      if (value) {
        addSerialBit(0);
      }
      addSerialBit(1);
    }

    void addTail()
    {
      while (bits_count != 0) {
        addSerialBit(1);
      }
    }
};

class PwmPxxBitTransport: public PulsesBuffer<pulse_duration_t, 200> {
  protected:
    uint16_t rest;

    void initFrame(uint32_t period)
    {
      initBuffer();
      rest = period * 2000; // 0.5uS (2Mhz)
    }

    void addPart(uint8_t value)
    {
      pulse_duration_t duration = value ? 47 : 31;
      *ptr++ = duration;
      rest -= duration + 1;
    }

    void addTail()
    {
      // rest min value is 18000 - 200 * 48 = 8400 (4.2ms)
      *(ptr - 1) = 60000;
    }
};

#endif
