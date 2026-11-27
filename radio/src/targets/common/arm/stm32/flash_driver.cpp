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

#include "flash_driver.h"

#include "stm32_cmsis.h"
#include "stm32_hal.h"

#if defined(STM32H5)
  #include "stm32_hal_ll.h"
#endif

#include <string.h>

static uint32_t stm32_flash_get_size_kb()
{
  uint32_t flash_size_kb = *(const uint32_t*)FLASHSIZE_BASE;
  return flash_size_kb;
}

#if defined(STM32F2) || defined(STM32F4)

static uint32_t stm32_flash_get_sector(uint32_t address)
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

static uint32_t stm32_flash_get_sector_size(uint32_t sector)
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

// twice the datasheet maximum for a 128 KB sector erase at x32 parallelism
// (2 s, DS9405 Table 48); the driver never issues a bank or mass erase
#define FLASH_TIMEOUT_MS 4000

#if defined(FLASH_FLAG_RDERR)
  #define _FLASH_FLAG_RDERR FLASH_FLAG_RDERR
#else
  #define _FLASH_FLAG_RDERR 0U
#endif

// Error flags are sticky until written with 1, so one left over by an earlier
// access since reset (e.g. a stray write to flash, which sets PGSERR) would
// abort the very next erase/program. Clear them before starting an operation.
static void flash_drv_clear_errors()
{
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
                         FLASH_FLAG_PGSERR | _FLASH_FLAG_RDERR);
}

static bool flash_drv_wait_last_op()
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  uint32_t start = DWT->CYCCNT;
  uint32_t timeout_cycles =
      (uint32_t)(FLASH_TIMEOUT_MS * (SystemCoreClock / 1000UL));

  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
    if ((DWT->CYCCNT - start) > timeout_cycles) {
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR |
                             FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
                             FLASH_FLAG_PGSERR);
      return false;
    }
  }

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR)) {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_EOP);
    return false;
  }

  return true;
}

// ES0206 2.2.15: on dual bank devices the data cache may be corrupted by a
// read-while-write, so it must be disabled while flash is erased/programmed
// and reset before being enabled again.
static bool flash_drv_disable_dcache()
{
  bool enabled = (FLASH->ACR & FLASH_ACR_DCEN) != 0;
  if (enabled) FLASH->ACR &= ~FLASH_ACR_DCEN;
  return enabled;
}

static void flash_drv_flush_caches(bool dcache_enabled)
{
  if (FLASH->ACR & FLASH_ACR_ICEN) {
    FLASH->ACR &= ~FLASH_ACR_ICEN;
    FLASH->ACR |= FLASH_ACR_ICRST;
    FLASH->ACR &= ~FLASH_ACR_ICRST;
    FLASH->ACR |= FLASH_ACR_ICEN;
  }
  if (dcache_enabled) {
    FLASH->ACR |= FLASH_ACR_DCRST;
    FLASH->ACR &= ~FLASH_ACR_DCRST;
    FLASH->ACR |= FLASH_ACR_DCEN;
  }
}

#elif defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5)

static uint32_t stm32_flash_get_sector(uint32_t address)
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

static uint32_t stm32_flash_get_sector_size(uint32_t sector)
{
  (void)sector;
  return FLASH_SECTOR_SIZE;
}

#endif

static uint32_t stm32_flash_get_bank(uint32_t address)
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

static inline void stm32_flash_unlock() { HAL_FLASH_Unlock(); }
static inline void stm32_flash_lock() { HAL_FLASH_Lock(); }

static int stm32_flash_erase_sector(uint32_t address)
{
  int ret = 0;

#if defined(STM32F2) || defined(STM32F4)

  uint32_t sector = stm32_flash_get_sector(address);

  __disable_irq();
  __DSB();

  stm32_flash_unlock();
  flash_drv_clear_errors();

  bool dcache = flash_drv_disable_dcache();

  if (sector > 11) sector += 4;

  CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
  FLASH->CR |= FLASH_PSIZE_WORD;
  CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
  FLASH->CR |= FLASH_CR_SER | (sector << FLASH_CR_SNB_Pos);
  FLASH->CR |= FLASH_CR_STRT;

  if (!flash_drv_wait_last_op()) {
    ret = -1;
  }

  CLEAR_BIT(FLASH->CR, FLASH_CR_SER | FLASH_CR_SNB);

  __DSB();
  __enable_irq();

  flash_drv_flush_caches(dcache);

  stm32_flash_lock();

#else

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

  // Interrupts must stay enabled here: the HAL timeout counts HAL_GetTick(),
  // which only advances in the ms timer interrupt. Nothing needs masking, as
  // code and vectors run from TCM/SDRAM, never from the bank being erased.
  stm32_flash_unlock();
  if (HAL_FLASHEx_Erase(&eraseInit, &sector_errors) != HAL_OK) {
    ret = -1;
  }

  stm32_flash_lock();

#endif

  return ret;
}

#if defined(STM32H7)
  #define FLASH_PROG_WORDS FLASH_NB_32BITWORD_IN_FLASHWORD
  #define _FLASH_PROGRAM(address, p_data) \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uintptr_t)p_data)
#elif defined(STM32H7RS) || defined(STM32H5)
  #define FLASH_PROG_WORDS 4UL
  #define _FLASH_PROGRAM(address, p_data) \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, address, (uintptr_t)p_data)
#endif

static int stm32_flash_program(uint32_t address, void* data, uint32_t len)
{
  int ret = 0;

#if defined(STM32F2) || defined(STM32F4)

  uint32_t* p_data = (uint32_t*)data;
  uint32_t end_addr = address + len;

  __disable_irq();
  __DSB();
  stm32_flash_unlock();
  flash_drv_clear_errors();

  bool dcache = flash_drv_disable_dcache();

  while (address < end_addr) {
    CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;

    *(__IO uint32_t*)address = *p_data;

    // PG must be cleared even on failure, or the next erase sees PG+SER
    bool ok = flash_drv_wait_last_op();
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

    if (!ok) {
      ret = -1;
      break;
    }

    address += sizeof(uint32_t);
    p_data++;
  }

  __DSB();
  __enable_irq();

  flash_drv_flush_caches(dcache);

  stm32_flash_lock();

#else

  uint32_t* p_data = (uint32_t*)data;
  uint32_t end_addr = address + len;

  // interrupts stay enabled, see stm32_flash_erase_sector()
  stm32_flash_unlock();
  while (address < end_addr) {
    if (_FLASH_PROGRAM(address, p_data) != HAL_OK) {
      ret = -1;
      break;
    }

    address += sizeof(uint32_t) * FLASH_PROG_WORDS;
    p_data += FLASH_PROG_WORDS;
  }

  stm32_flash_lock();

#endif

  return ret;
}

static int stm32_flash_read(uint32_t address, void* data, uint32_t len)
{
  memcpy(data, (void*)address, len);
  return 0;
}

const etx_flash_driver_t stm32_flash_driver = {
  .get_size_kb = stm32_flash_get_size_kb,
  .get_sector = stm32_flash_get_sector,
  .get_sector_size = stm32_flash_get_sector_size,
  .erase_sector = stm32_flash_erase_sector,
  .program = stm32_flash_program,
  .read = stm32_flash_read,
};

#if defined(BOOT) && defined(BOOTLOADER_DFU)
//
// USB DFU interface: the firmware area of the internal flash. The
// bootloader runs from flash, so its own sectors are not exposed.
//
#include "board.h"
#include "usbd_dfu.h"
#include "hal/watchdog_driver.h"

#if !defined(STM32F429xx)
  #error "BOOTLOADER_DFU: sector layout only defined for STM32F429xI"
#endif

static_assert(APP_START_ADDRESS == 0x08020000, "firmware must start at sector 5");

// 2 MB dual bank: bank 1 sectors 5..11, then all of bank 2
#define DFU_MEDIA_DESC \
  "@Internal Flash   /0x08020000/07*128Kg,04*016Kg,01*064Kg,07*128Kg"

static bool dfu_in_app_area(uint32_t address, uint32_t len)
{
  return address >= APP_START_ADDRESS && len <= FLASH_END + 1 - address;
}

static uint16_t stm32_flash_dfu_init() { return USBD_OK; }
static uint16_t stm32_flash_dfu_deinit() { return USBD_OK; }

static uint16_t stm32_flash_dfu_erase(uint32_t address)
{
  if (!dfu_in_app_area(address, 1)) return USBD_FAIL;
  // back to back erases from the USB interrupt starve the main loop, which
  // feeds the watchdog otherwise (one 128 KB erase: 2 s max < its 4 s)
  WDG_RESET();
  return stm32_flash_erase_sector(address) < 0 ? USBD_FAIL : USBD_OK;
}

static uint16_t stm32_flash_dfu_write(uint8_t* src, uint8_t* dest,
                                      uint32_t len)
{
  uint32_t address = (uintptr_t)dest;
  if (!dfu_in_app_area(address, len)) return USBD_FAIL;

  // the driver programs whole words: pad a partial last word with the
  // erased value (only the last block can be shorter than the buffer)
  uint32_t padded = (len + 3) & ~3UL;
  if (padded > USBD_DFU_XFER_SIZE) return USBD_FAIL;
  memset(src + len, 0xFF, padded - len);

  WDG_RESET();
  return stm32_flash_program(address, src, padded) < 0 ? USBD_FAIL : USBD_OK;
}

static uint8_t* stm32_flash_dfu_read(uint8_t* src, uint8_t* dest, uint32_t len)
{
  if (!dfu_in_app_area((uintptr_t)src, len)) return nullptr;
  memcpy(dest, src, len);
  return dest;
}

static uint16_t stm32_flash_dfu_get_status(uint32_t address, uint8_t cmd,
                                           uint8_t* buffer)
{
  (void)address;
  // poll timeout in ms (24 bits); typical sector erase is up to 1 s for
  // 128 KB (DS9405), and the erase runs with interrupts disabled
  uint32_t timeout = (cmd == DFU_MEDIA_ERASE) ? 1000 : 20;
  buffer[1] = timeout & 0xFF;
  buffer[2] = (timeout >> 8) & 0xFF;
  buffer[3] = (timeout >> 16) & 0xFF;
  return USBD_OK;
}

// called from the USB interrupt: the jump is done by bootloaderDFU()
extern volatile bool bootloaderDFULeaveRequested;
static uint16_t stm32_flash_dfu_leave(uint32_t address)
{
  (void)address;
  bootloaderDFULeaveRequested = true;
  return USBD_OK;
}

static const USBD_DFU_MediaTypeDef _stm32_flash_dfu_media = {
  .pStrDesc = (uint8_t*)DFU_MEDIA_DESC,
  .Init = stm32_flash_dfu_init,
  .DeInit = stm32_flash_dfu_deinit,
  .Erase = stm32_flash_dfu_erase,
  .Write = stm32_flash_dfu_write,
  .Read = stm32_flash_dfu_read,
  .GetStatus = stm32_flash_dfu_get_status,
  .LeaveDFU = stm32_flash_dfu_leave,
};

const void* stm32_flash_dfu_media = &_stm32_flash_dfu_media;

#endif // BOOT && BOOTLOADER_DFU

// Legacy API

#define FLASH_PAGESIZE 256

void unlockFlash() { stm32_flash_unlock(); }
void lockFlash() { stm32_flash_lock(); }

bool flashWrite(uint32_t* address, const uint32_t* buffer)
{
  // check first if the address is on a sector boundary
  uint32_t sector = stm32_flash_get_sector((uintptr_t)address);
  uint32_t bank = stm32_flash_get_bank((uintptr_t)address);

  if ((uintptr_t)address == _flash_sector_address(sector, bank)) {
    if (stm32_flash_erase_sector((uintptr_t)address) < 0) return false;
  }

  if (stm32_flash_program((uintptr_t)address, (uint8_t*)buffer,
                          FLASH_PAGESIZE) < 0)
    return false;

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  // drop any lines cached before the erase/program so the read back
  // below comes from flash
  SCB_InvalidateDCache_by_Addr(address, FLASH_PAGESIZE);
#elif defined(STM32H5)
  // H5 has no core D-cache, but its ICACHE caches data reads from flash
  // too: lines read before the erase (e.g. the old firmware's vectors and
  // version string) would make the read back below see stale data
  LL_ICACHE_Invalidate();
  while (!LL_ICACHE_IsActiveFlag_BSYEND()) {}
  LL_ICACHE_ClearFlag_BSYEND();
#endif

  // verify the page was actually written, independently of what the
  // flash controller reported
  return memcmp(address, buffer, FLASH_PAGESIZE) == 0;
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
#elif defined(STM32H7)
  // Stack pointer in DTCM RAM
  if ((block[0] & 0xFFFF0000) != 0x20020000) {
    return 0;
  }
#if defined(FIRMWARE_QSPI)
  // First ISR pointer in FLASH (QSPI)
  if ((block[1] & 0xF0000000) != 0x90000000) {
    return 0;
  }
  // Second ISR pointer in FLASH (SDRAM)
  if ((block[2] & 0xF0000000) != 0xC0000000) {
    return 0;
  }
#elif defined(STM32H743xx)
  // Internal Flash execution (STM32H743)
  // block[] = {stack, reset_handler, length, version_ptr}
  // 1. Reset Handler in internal flash (Bank 1 or Bank 2: 0x08000000 - 0x081FFFFF)
  if ((block[1] & 0xFFE00000) != 0x08000000) {
    return 0;
  }
  // 2. Length must be non-zero and fit inside available firmware flash (1920 KB)
  if (block[2] == 0 || block[2] > (1920 * 1024)) {
    return 0;
  }
  // 3. Version string pointer in internal flash
  if ((block[3] & 0xFFE00000) != 0x08000000) {
    return 0;
  }
#else
  #error "Unsupported H7 storage layout"
#endif
#elif defined(STM32H5)
  // Stack pointer in RAM (640K, top at 0x200A0000)
  if ((block[0] & 0xFFF00000) != 0x20000000) {
    return 0;
  }
  // First ISR pointer in FLASH (up to 2M)
  if ((block[1] & 0xFFE00000) != 0x08000000) {
    return 0;
  }
  // Second ISR pointer in FLASH
  if ((block[2] & 0xFFE00000) != 0x08000000) {
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
