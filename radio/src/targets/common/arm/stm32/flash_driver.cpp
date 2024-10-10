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

#include "stm32_cmsis.h"
#include "stm32_hal.h"

uint32_t stm32_flash_get_size_kb()
{
  uint32_t flash_size_kb = *(const uint32_t*)FLASHSIZE_BASE;
  return flash_size_kb;
}

#if defined(STM32F2) || defined(STM32F4)

uint32_t stm32_flash_get_sector(uint32_t address)
{
  uint32_t sector;
  uint32_t sector_addr = address & 0xFFFFF;

  if (sector_addr < 0x10000) {
    sector = sector_addr / 0x4000;
  } else {
    sector = (sector_addr / 0x20000) + 4;
  }

  // bank 2
  if (address & 0x100000) {
    sector += 12;
  }

  return sector;  
}

static uint32_t _flash_sector_address(uint32_t sector, uint32_t bank)
{
  (void)bank;
  
  uint32_t addr = FLASH_BASE;
  if (sector >= 12) {
    addr += 0x100000;
    sector -= 12;
  }
  
  if (sector <= 4) {
    addr += sector * 0x4000;
  } else {
    addr += (sector - 4) * 0x20000;
  }

  return addr;
}

uint32_t stm32_flash_get_sector_size(uint32_t sector)
{
  // bank 2
  if (sector >= 12) sector -= 12;

  // first 4: 16KB  
  if (sector < 4) return 16 * 1024;
  // sector 4: 64KB
  if (sector == 4) return 64 * 1024;
  // others: 128KB
  return 128 * 1024;
}

#elif defined(STM32H7) || defined(STM32H7RS)

uint32_t stm32_flash_get_sector(uint32_t address)
{
  uint32_t sector_addr = address & 0xFFFFF;
  return sector_addr / FLASH_SECTOR_SIZE;
}

static uint32_t _flash_sector_address(uint32_t sector, uint32_t bank)
{
  uint32_t addr = FLASH_BASE;
#if defined(DUAL_BANK)
  if (bank != FLASH_BANK_1) addr += 0x100000;
#endif
  addr += sector * FLASH_SECTOR_SIZE;
  return addr;
}

uint32_t stm32_flash_get_sector_size(uint32_t sector)
{
  (void)sector;
  return FLASH_SECTOR_SIZE;
}

#endif

uint32_t stm32_flash_get_bank(uint32_t address)
{
#if defined(DUAL_BANK)
  return ((address & 0x100000) != 0x100000) ? FLASH_BANK_1 : FLASH_BANK_2;
#elif defined(FLASH_BANK_1)
  (void)address;
  return FLASH_BANK_1;
#else
  (void)address;
  return 1UL;
#endif
}

void stm32_flash_unlock() { HAL_FLASH_Unlock(); }
void stm32_flash_lock() { HAL_FLASH_Lock(); }

int stm32_flash_erase_sector(uint32_t address)
{
  FLASH_EraseInitTypeDef eraseInit;
  eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
  eraseInit.Sector = stm32_flash_get_sector(address);
  eraseInit.NbSectors = 1UL;

#if defined(FLASH_BANK_1)
  eraseInit.Banks = stm32_flash_get_bank(address);
#endif

#if defined(FLASH_VOLTAGE_RANGE_3)
  eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
#endif

  uint32_t sector_errors = 0;
  if (HAL_FLASHEx_Erase(&eraseInit, &sector_errors) != HAL_OK) {
    return -1;
  }

  return 0;
}

#if defined(STM32H7)
  #define FLASH_PROG_WORDS FLASH_NB_32BITWORD_IN_FLASHWORD
  #define _FLASH_PROGRAM(address, p_data) \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uintptr_t)p_data)
#elif defined(STM32H7RS)
  #define FLASH_PROG_WORDS 4UL
  #define _FLASH_PROGRAM(address, p_data) \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, address, (uintptr_t)p_data)
#else
  #define FLASH_PROG_WORDS 1UL
  #define _FLASH_PROGRAM(address, p_data) \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *p_data)
#endif

int stm32_flash_program(uint32_t address, uint8_t* data, uint32_t len)
{
  uint32_t* p_data = (uint32_t*)data;
  uint32_t end_addr = address + len;

  while (address < end_addr) {
    if (_FLASH_PROGRAM(address, p_data) != HAL_OK) {
      return -1;
    }

    address += sizeof(uint32_t) * FLASH_PROG_WORDS;
    p_data += FLASH_PROG_WORDS;
  }

  return 0;
}

// Legacy API

#define FLASH_PAGESIZE 256

void unlockFlash() { stm32_flash_unlock(); }
void lockFlash() { stm32_flash_lock(); }

void flashWrite(uint32_t* address, const uint32_t* buffer)
{
  // check first if the address is on a sector boundary
  uint32_t sector = stm32_flash_get_sector((uintptr_t)address);
  uint32_t bank = stm32_flash_get_bank((uintptr_t)address);

  if ((uintptr_t)address == _flash_sector_address(sector, bank)) {
    if (stm32_flash_erase_sector((uintptr_t)address) < 0) return;
  }

  stm32_flash_program((uintptr_t)address, (uint8_t*)buffer, FLASH_PAGESIZE);
}

// TODO: move this somewhere else, as it depends on firmware layout
uint32_t isFirmwareStart(const uint8_t * buffer)
{
  const uint32_t * block = (const uint32_t *)buffer;

#if defined(STM32F4)
  // Stack pointer in CCM or RAM
  if ((block[0] & 0xFFFC0000) != 0x10000000 && (block[0] & 0xFFFC0000) != 0x20000000) {
    return 0;
  }
  // First ISR pointer in FLASH
  if ((block[1] & 0xFFC00000) != 0x08000000) {//for nv14 firmware may start up to 0x81F ....
    return 0;
  }
  // Second ISR pointer in FLASH
  if ((block[2] & 0xFFC00000) != 0x08000000) {
    return 0;
  }
#else
  // Stack pointer in RAM
  if ((block[0] & 0xFFFC0000) != 0x20000000) {
    return 0;
  }
  // First ISR pointer in FLASH
  if ((block[1] & 0xFFF00000) != 0x08000000) {
    return 0;
  }
  // Second ISR pointer in FLASH
  if ((block[2] & 0xFFF00000) != 0x08000000) {
    return 0;
  }
#endif
  return 1;
}

uint32_t isBootloaderStart(const uint8_t * buffer)
{
  const uint32_t * block = (const uint32_t *)buffer;

  for (int i = 0; i < 256; i++) {
    if (block[i] == 0x544F4F42/*BOOT*/) {
      return 1;
    }
  }
  return 0;
}
