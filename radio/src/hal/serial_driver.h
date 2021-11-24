/*
 * Copyright (C) EdgeTx
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

enum SerialParity {
    ETX_Parity_None,
    ETX_Parity_Even,
    ETX_Parity_Odd,
};

enum SerialStopBits {
    ETX_StopBits_One,
    ETX_StopBits_OneAndHalf,
    ETX_StopBits_Two,
};

enum SerialWordLength {
  ETX_WordLength_8,
  ETX_WordLength_9,
};

struct etx_serial_init {
  uint32_t baudrate;    // = 0;
  uint8_t parity;       // = ETX_Parity_None;
  uint8_t stop_bits;    // = ETX_StopBits_One;
  uint8_t word_length;  // = ETX_WordLength_8;
  bool rx_enable;       // = false;

  void (*on_receive)(uint8_t data);  // = nullptr;
  void (*on_error)();                // = nullptr;
};

struct etx_serial_callbacks_t {
  bool (*on_send)(uint8_t& data);
  void (*on_receive)(uint8_t data);
  void (*on_error)();
};

struct etx_serial_driver_t {

  // Init module communication
  void (*init)(const etx_serial_init* params);

  // De-Init module communication
  void (*deinit)();

  // Send a single byte
  void (*sendByte)(uint8_t byte);

  // Send a buffer
  void (*sendBuffer)(const uint8_t* data, uint8_t size);

  // Wait for last send operation to complete
  void (*waitForTxCompleted)();

  // Callbacks
  void (*setReceiveCb)(void (*on_receive)(uint8_t data));
  void (*setOnErrorCb)(void (*on_error)());
};
