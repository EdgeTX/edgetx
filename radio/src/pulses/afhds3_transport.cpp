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

#include "afhds3_transport.h"

// timer is 2 MHz
#if defined(AFHDS3_SLOW)
  // 1000000/57600 = 17,36 us
  #define BITLEN_AFHDS (35)
#else
  // 1000000/115200 = 8,68 us
  #define BITLEN_AFHDS (17)
#endif

namespace afhds3
{

static void _serial_reset(void* buffer)
{
  auto data = (SerialData*)buffer;
  data->ptr = data->pulses;
}

static void _serial_sendByte(void* buffer, uint8_t b)
{
  auto data = (SerialData*)buffer;
  *(data->ptr++) = b;
}

static void _serial_flush(void*) {}

uint32_t _serial_getSize(void* buffer)
{
  auto data = (SerialData*)buffer;
  return data->ptr - data->pulses;
}

static void _pulses_reset(void* buffer)
{
  auto data = (PulsesData*)buffer;
  data->ptr = data->pulses;
}

static inline void _pulses_send_level(PulsesData* data, uint16_t v)
{
  uint32_t size = data->ptr - data->pulses;
  if (size >= AFHDS_MAX_PULSES_TRANSITIONS) return;
  *(data->ptr)++ = v;
}

static void _pulses_sendByte(void* buffer, uint8_t b)
{
  auto data = (PulsesData*)buffer;
  uint32_t size = data->ptr - data->pulses;
  if (size >= AFHDS_MAX_PULSES_TRANSITIONS) return;

  // use 8n1
  // parity: If the parity is enabled, then the MSB bit of the data to be
  // transmitted is changed by the parity bit start is always 0
  bool level = 0;
  uint16_t length = BITLEN_AFHDS;     // start bit
  for (uint8_t i = 0; i <= 8; i++) {  // 8 data bits + Stop=1
    bool next_level = b & 1;
    if (level == next_level) {
      length += BITLEN_AFHDS;
    } else {
      _pulses_send_level(data, length);
      length = BITLEN_AFHDS;
      level = next_level;
    }
    b = (b >> 1) | 0x80;  // shift left to get next bit, fill msb with stop
                          // bit - needed just once
  }
  _pulses_send_level(data, length);  // last bit (stop)
}

static void _pulses_flush(void* buffer)
{
  // TODO: use less, as the new driver can handle it
  auto data = (PulsesData*)buffer;
  *data->ptr = 60000;
}

uint32_t _pulses_getSize(void* buffer)
{
  auto data = (PulsesData*)buffer;
  return data->ptr - data->pulses;
}
  
void Transport::init(Type t)
{
  if (t == Serial) {
    reset = _serial_reset;
    sendByte = _serial_sendByte;
    flush = _serial_flush;
    getSize = _serial_getSize;
  } else if (t == Pulses) {
    reset = _pulses_reset;
    sendByte = _pulses_sendByte;
    flush = _pulses_flush;
    getSize = _pulses_getSize;
  } else {
    reset = nullptr;
    sendByte = nullptr;
    flush = nullptr;
    getSize = nullptr;
  }
} 

};
