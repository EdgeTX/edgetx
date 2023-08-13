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

#include "hal/serial_driver.h"

// BT driver
#define BLUETOOTH_BOOTLOADER_BAUDRATE   230400
#define BLUETOOTH_DEFAULT_BAUDRATE      115200
#if defined(PCBX9E)
#define BLUETOOTH_FACTORY_BAUDRATE      9600
#else
#define BLUETOOTH_FACTORY_BAUDRATE      57600
#endif
#define BT_TX_FIFO_SIZE    64
#define BT_RX_FIFO_SIZE    256

void bluetoothInit(uint32_t baudrate, bool enable);
void bluetoothWrite(const void* buffer, uint32_t length);
int bluetoothRead(uint8_t* data);
uint8_t bluetoothIsWriting();
void bluetoothDisable();

#if defined(PCBX9LITES) || defined(PCBX7ACCESS)
  #define IS_BLUETOOTH_CHIP_PRESENT()     (true)
#elif defined(PCBX9LITE)
  #define IS_BLUETOOTH_CHIP_PRESENT()     (false)
#elif defined(BLUETOOTH_PROBE) && !defined(SIMU)
  extern volatile uint8_t btChipPresent;
  #define IS_BLUETOOTH_CHIP_PRESENT()     (btChipPresent)
#else
  #define IS_BLUETOOTH_CHIP_PRESENT()     (true)
#endif
