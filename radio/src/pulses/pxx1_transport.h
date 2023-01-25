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

#include "pxx.h"
#include "crc.h"

class Pxx1CrcMixin {
  protected:
    void initCrc()
    {
      crc = 0;
    }

    void addToCrc(uint8_t byte)
    {
      crc = (crc << 8) ^ (crc16tab[CRC_1189][((crc >> 8) ^ byte) & 0xFF]);
    }

    uint16_t crc;
};

// Simulate legacy PWM over a serial transport @ 125kb/s
class SerialPxxBitTransport {
  protected:
    const uint8_t* buffer;
    uint8_t* ptr;
    uint8_t byte = 0;
    uint8_t bits_count = 0;

    SerialPxxBitTransport(uint8_t* buffer)
      : buffer(buffer), ptr(buffer)
    {
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

  public:
    uint32_t getSize()
    {
      return ptr - buffer;
    }
};

template <class BitTransport>
class StandardPxx1Transport: public BitTransport, public Pxx1CrcMixin {
  protected:
    uint8_t ones_count = 0;

    StandardPxx1Transport(uint8_t* buffer)
      : BitTransport(buffer)
    {
    }

    void addByte(uint8_t byte)
    {
      Pxx1CrcMixin::addToCrc(byte);
      addByteWithoutCrc(byte);
    };

    void addRawByte(uint8_t byte)
    {
      for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80)
          BitTransport::addPart(1);
        else
          BitTransport::addPart(0);
        byte <<= 1;
      }
    }

    void addByteWithoutCrc(uint8_t byte)
    {
      for (uint8_t i = 0; i < 8; i++) {
        addBit(byte & 0x80);
        byte <<= 1;
      }
    }

    void addBit(uint8_t bit)
    {
      if (bit) {
        BitTransport::addPart(1);
        if (++ones_count == 5) {
          ones_count = 0;
          BitTransport::addPart(0); // Stuff a 0 bit in
        }
      }
      else {
        BitTransport::addPart(0);
        ones_count = 0;
      }
    }
};

class UartPxx1Transport: public Pxx1CrcMixin {
  protected:
    const uint8_t* buffer;
    uint8_t* ptr;

    UartPxx1Transport(uint8_t* buffer)
      : buffer(buffer), ptr(buffer)
    {
    }

    void addByte(uint8_t byte)
    {
      Pxx1CrcMixin::addToCrc(byte);
      addWithByteStuffing(byte);
    }

    void addRawByte(uint8_t byte)
    {
      *ptr++ = byte;
    }

    void addByteWithoutCrc(uint8_t byte)
    {
      addWithByteStuffing(byte);
    }

    void addWithByteStuffing(uint8_t byte)
    {
      if (0x7E == byte) {
        addRawByte(0x7D);
        addRawByte(0x5E);
      }
      else if (0x7D == byte) {
        addRawByte(0x7D);
        addRawByte(0x5D);
      }
      else {
        addRawByte(byte);
      }
    }

    void addTail()
    {
      // nothing
    }

  public:
    uint32_t getSize()
    {
      return ptr - buffer;
    }
};

template <class PxxTransport>
class Pxx1Pulses: public PxxTransport
{
  public:
    Pxx1Pulses(uint8_t* buffer);
    void setupFrame(uint8_t port);

  protected:
    void addHead()
    {
      // send 7E, do not CRC
      PxxTransport::addRawByte(0x7E);
    }

    void addCrc()
    {
      PxxTransport::addByteWithoutCrc(Pxx1CrcMixin::crc >> 8);
      PxxTransport::addByteWithoutCrc(Pxx1CrcMixin::crc);
    }

    void addFlag1(uint8_t port, uint8_t sendFailsafe);
    void addExtraFlags(uint8_t port);
    void addChannels(uint8_t port, uint8_t sendFailsafe, uint8_t sendUpperChannels);
    void add8ChannelsFrame(uint8_t port, uint8_t sendUpperChannels, uint8_t sendFailsafe);
};

typedef Pxx1Pulses<UartPxx1Transport> UartPxx1Pulses;
typedef Pxx1Pulses<StandardPxx1Transport<SerialPxxBitTransport>> SerialPxx1Pulses;
