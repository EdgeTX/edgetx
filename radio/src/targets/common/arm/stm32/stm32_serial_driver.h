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

#include "stm32_usart_driver.h"
#include "hal/serial_driver.h"

// __DMA
#include "definitions.h"

// Serial buffer to be defined in boards
struct stm32_serial_buffer {
  uint8_t* buffer;
  uint32_t length;
};

// Serial port to be defined in boards
// (the board defines a buffer as well)
//
// etx_serial_driver.init() shall be passed
// this struct cast as (void*)
//
struct stm32_serial_port {
  const stm32_usart_t* usart;
  const stm32_serial_buffer rx_buffer;
  const stm32_serial_buffer tx_buffer;
};

extern const etx_serial_driver_t STM32SerialDriver;

#define DEFINE_STM32_SERIAL_PORT(p,usart,rx_buf_len,tx_buf_len) \
  static uint8_t p ## _RXBuffer[rx_buf_len] __DMA;              \
  static uint8_t p ## _TXBuffer[tx_buf_len] __DMA;              \
  static const stm32_serial_port p ## _STM32Serial = {          \
    &usart,                                                     \
    { p ## _RXBuffer, rx_buf_len },                             \
    { p ## _TXBuffer, tx_buf_len },                             \
  }

#define REF_STM32_SERIAL_PORT(p) ((void*)& p ## _STM32Serial)
