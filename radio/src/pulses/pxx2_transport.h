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

#include <stdint.h>

class Pxx2CrcMixin {
  protected:
    uint16_t crc = 0xFFFF;

    void addToCrc(uint8_t byte)
    {
      crc -= byte;
    }

};

class Pxx2Transport: public Pxx2CrcMixin {
  protected:
    uint8_t* buffer;
    uint8_t* ptr;

    Pxx2Transport(uint8_t* buffer)
      : buffer(buffer), ptr(buffer)
    {}
  
    void addWord(uint32_t word)
    {
      addByte(word);
      addByte(word >> 8);
      addByte(word >> 16);
      addByte(word >> 24);
    }

    void addByte(uint8_t byte)
    {
      Pxx2CrcMixin::addToCrc(byte);
      addByteWithoutCrc(byte);
    };

    void addByteWithoutCrc(uint8_t byte)
    {
      *ptr++ = byte;
    }

  public:
    uint32_t getSize()
    {
      return ptr - buffer;
    }
};

class Pxx2Pulses: public Pxx2Transport {
  friend class Pxx2OtaUpdate;

  public:
    Pxx2Pulses(uint8_t* buffer)
      : Pxx2Transport(buffer)
    {
      addHead();
    }

    bool setupFrame(uint8_t module, int16_t* channels, uint8_t nChannels);
    void setupAuthenticationFrame(uint8_t module, uint8_t mode, const uint8_t * outputMessage);

  protected:
    void setupHardwareInfoFrame(uint8_t module, int16_t* channels, uint8_t nChannels);

    void setupRegisterFrame(uint8_t module);

    void setupBindFrame(uint8_t module);

    void setupAccstBindFrame(uint8_t module);

    void setupAccessBindFrame(uint8_t module);

    void setupResetFrame(uint8_t module);

    void setupShareMode(uint8_t module);

    void setupModuleSettingsFrame(uint8_t module, int16_t* channels, uint8_t nChannels);

    void setupReceiverSettingsFrame(uint8_t module, int16_t* channels, uint8_t nChannels);

    void setupChannelsFrame(uint8_t module, int16_t* channels, uint8_t nChannels);

    void setupTelemetryFrame(uint8_t module);

    void setupSpectrumAnalyser(uint8_t module);

    void setupPowerMeter(uint8_t module);

    void sendOtaUpdate(uint8_t module, const char * rxName, uint32_t address, const char * data);

    void addHead()
    {
      // send 7E, do not CRC
      Pxx2Transport::addByteWithoutCrc(0x7E);

      // reserve 1 byte for LEN
      Pxx2Transport::addByteWithoutCrc(0x00);
    }

    void addFrameType(uint8_t type_c, uint8_t type_id)
    {
      // TYPE_C + TYPE_ID
      Pxx2Transport::addByte(type_c);
      Pxx2Transport::addByte(type_id);
    }

    uint8_t addFlag0(uint8_t module);

    void addFlag1(uint8_t module);

    void addPulsesValues(uint16_t low, uint16_t high);

    void addChannels(uint8_t module, int16_t* channels, uint8_t nChannels);

    void addFailsafe(uint8_t module);

    void addCrc()
    {
      Pxx2Transport::addByteWithoutCrc(Pxx2CrcMixin::crc >> 8);
      Pxx2Transport::addByteWithoutCrc(Pxx2CrcMixin::crc);
    }

    void endFrame()
    {
      if (getSize() <= 2) return;

      // update the frame LEN = frame length minus the 2 first bytes
      buffer[1] = getSize() - 2;

      // now add the CRC
      addCrc();
    }
};
