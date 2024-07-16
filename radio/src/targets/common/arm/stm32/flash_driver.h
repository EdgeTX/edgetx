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

uint32_t stm32_flash_get_size_kb();
uint32_t stm32_flash_get_sector(uint32_t address);
uint32_t stm32_flash_get_sector_size(uint32_t sector);
uint32_t stm32_flash_get_bank(uint32_t address);

void stm32_flash_unlock();
void stm32_flash_lock();

int stm32_flash_erase_sector(uint32_t address);
int stm32_flash_program(uint32_t address, uint8_t* data, uint32_t len);

// Legacy API

uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

void unlockFlash();
void lockFlash();

void flashWrite(uint32_t* address, const uint32_t* buffer);

