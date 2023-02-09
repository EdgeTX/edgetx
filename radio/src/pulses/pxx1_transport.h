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

class StandardPxx1Transport: public Pxx1CrcMixin {
  protected:
    const uint8_t* buffer;
    uint8_t* ptr;
    uint8_t byte = 0;
    uint8_t bits_count = 0;
    uint8_t ones_count = 0;

    StandardPxx1Transport(uint8_t* buffer)
      : buffer(buffer), ptr(buffer)
    {
    }

    void addByte(uint8_t b)
    {
      Pxx1CrcMixin::addToCrc(b);
      addByteWithoutCrc(b);
    };

    void addRawByte(uint8_t b)
    {
      for (uint8_t i = 0; i < 8; i++) {
        // MSB first
        if (b & 0x80) addPart(1);
        else addPart(0);
        b <<= 1;
      }
    }

    void addByteWithoutCrc(uint8_t b)
    {
      for (uint8_t i = 0; i < 8; i++) {
        // MSB first
        addBit(b & 0x80);
        b <<= 1;
      }
    }

    void addBit(uint8_t bit)
    {
      if (bit) {
        addPart(1);
        if (++ones_count == 5) {
          ones_count = 0;
          addPart(0); // Stuff a 0 bit in
        }
      }
      else {
        addPart(0);
        ones_count = 0;
      }
    }

    void addPart(uint8_t bit)
    {
      // MSB first
      byte <<= 1;
      if (bit & 1) {
        byte |= 1;
      }
      if (++bits_count >= 8) {
        *ptr++ = byte;
        bits_count = 0;
      }
    }

    void addTail()
    {
      if (bits_count > 0)
        *ptr++ = byte << (8 - bits_count);
    }

  public:
    uint32_t getSize()
    {
      uint32_t size = (ptr - buffer) * 8;
      if (bits_count > 0)
        size -= (8 - bits_count);

      return size;
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


enum class Pxx1Type : uintptr_t {
  PWM,
  SLOW_SERIAL,
  FAST_SERIAL,
};

template <class PxxTransport>
class Pxx1Pulses: public PxxTransport
{
  public:
    Pxx1Pulses(uint8_t* buffer);
    void setupFrame(uint8_t module, Pxx1Type type);

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
typedef Pxx1Pulses<StandardPxx1Transport> PwmPxx1Pulses;
