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

struct etx_serial_init {
  uint32_t baudrate;
  uint16_t parity;
  uint16_t stop_bits;
  uint16_t word_length;
  bool     rx_enable;
  void     (*on_receive)(uint8_t data);
  void     (*on_error)();

  etx_serial_init();
};

void intmoduleSerialStart(const etx_serial_init* params);
void intmoduleSerialStop();

void intmoduleSendByte(uint8_t byte);
void intmoduleSendBuffer(const uint8_t* data, uint8_t size);
void intmoduleWaitForTxCompleted();
