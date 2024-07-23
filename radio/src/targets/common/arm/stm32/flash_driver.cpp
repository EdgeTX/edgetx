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

#include "board.h"
#include "hal/watchdog_driver.h"


#if defined(STM32H7)
  #define FLASH_CR FLASH->CR1
  #define FLASH_SR FLASH->SR1
#else
  #define FLASH_CR FLASH->CR
  #define FLASH_SR FLASH->SR
#endif

#if !defined(FLASH_CR_START)
#  define FLASH_CR_START FLASH_CR_STRT
#endif

#if !defined(FLASH_SR_BUSY)
#  define FLASH_SR_BUSY FLASH_SR_BSY
#endif

void waitFlashIdle()
{
  do {
    WDG_RESET();
  } while (FLASH_SR & FLASH_SR_BUSY);
}

//After reset, write is not allowed in the Flash control register (FLASH_CR) to protect the
//Flash memory against possible unwanted operations due, for example, to electric
//disturbances. The following sequence is used to unlock this register:
//1. Write KEY1 = 0x45670123 in the Flash key register (FLASH_KEYR)
//2. Write KEY2 = 0xCDEF89AB in the Flash key register (FLASH_KEYR)
//Any wrong sequence will return a bus error and lock up the FLASH_CR register until the
//next reset.
//The FLASH_CR register can be locked again by software by setting the LOCK bit in the
//FLASH_CR register.
void unlockFlash()
{
#if defined(STM32H7)
  FLASH->KEYR1 = FLASH_KEY1;
  FLASH->KEYR1 = FLASH_KEY2;
#else
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;
#endif
}

void lockFlash()
{
  waitFlashIdle();
  FLASH_CR |= FLASH_CR_LOCK;
}

#define SECTOR_MASK ((uint32_t)0xFFFFFF07)

void eraseSector(uint32_t sector)
{
  waitFlashIdle();

#if defined(FLASH_CR_PSIZE)
  FLASH_CR &= ~FLASH_CR_PSIZE;
  FLASH_CR |= FLASH_CR_PSIZE_1;
#endif
  FLASH_CR &= SECTOR_MASK;
  FLASH_CR |= FLASH_CR_SER | (sector << 3);
  FLASH_CR |= FLASH_CR_START;

  /* Wait for operation to be completed */
  waitFlashIdle();

  /* if the erase operation is completed, disable the SER Bit */
  FLASH_CR &= (~FLASH_CR_SER);
  FLASH_CR &= SECTOR_MASK;
}

void flashWrite(uint32_t * address, const uint32_t * buffer) // page size is 256 bytes
{
#define SECTOR_ADDRESS  (((uint32_t)address) &  0xFFFFF)

// Please note that there is an offset of 4 between
// sector 11 and 12
#define FLASH_BANK     ((((uint32_t)address) & 0x100000) ? 16 : 0)

    // test for possible flash sector boundary
    if ((((uint32_t)address) & 0x1FFFF) == 0) {
        // test first 16KB sectors
        if (SECTOR_ADDRESS == 0x00000) {
            eraseSector(0 + FLASH_BANK);
        }
        // test 128KB sectors
        else if (SECTOR_ADDRESS == 0x20000) {
            eraseSector(5 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0x40000) {
            eraseSector(6 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0x60000) {
            eraseSector(7 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0x80000) {
            eraseSector(8 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0xA0000) {
            eraseSector(9 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0xC0000) {
            eraseSector(10 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0xE0000) {
            eraseSector(11 + FLASH_BANK);
        }
    }
    // test 64KB sector
    else if (SECTOR_ADDRESS == 0x10000) {
        eraseSector(4 + FLASH_BANK);
    }
    else if ((((uint32_t)address) & 0x3FFF) == 0) {
        // test other 16KB sectors
        if (SECTOR_ADDRESS == 0x04000) {
            eraseSector(1 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0x08000) {
            eraseSector(2 + FLASH_BANK);
        }
        else if (SECTOR_ADDRESS == 0x0C000) {
            eraseSector(3 + FLASH_BANK);
        }
    }

#undef SECTOR_ADDRESS
#undef FLASH_BANK

  for (uint32_t i=0; i<FLASH_PAGESIZE/4; i++) {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
     be done by word */

    // Wait for last operation to be completed
    waitFlashIdle();

#if defined(FLASH_CR_PSIZE)
    FLASH_CR &= ~FLASH_CR_PSIZE;
    FLASH_CR |= FLASH_CR_PSIZE_1;
#endif
    FLASH_CR |= FLASH_CR_PG;

    *address = *buffer;

    /* Wait for operation to be completed */
    waitFlashIdle();
    FLASH_CR &= ~FLASH_CR_PG;

    /* Check the written value */
    if (*address != *buffer) {
      /* Flash content doesn't match SRAM content */
      return;
    }
    /* Increment FLASH destination address */
    address += 1;
    buffer += 1;
  }
}

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
