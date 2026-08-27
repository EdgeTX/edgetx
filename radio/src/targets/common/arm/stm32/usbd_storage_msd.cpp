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

/* Includes ------------------------------------------------------------------*/

#include "usbd_def.h"
#include "usbd_msc.h"

#include "hal/fatfs_diskio.h"
#include "hal/storage.h"

#include "stm32_hal.h"
#include "stm32_hal_ll.h"

#include "fw_version.h"
#include "hal.h"
#include "debug.h"

#include "usb_descriptor.h"
#include "usbd_storage_msd.h"

#include <string.h>

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS

#if !defined(BOOT)
  #include "timers_driver.h"
  #define WATCHDOG_SUSPEND(x) watchdogSuspend(x)
#else
  #define WATCHDOG_SUSPEND(...)
#endif

#include "usb_conf.h"

#if defined(FIRMWARE_FORMAT_UF2) && defined(BOOT)
#define USE_UF2_DRIVE 1
#else
#define USE_UF2_DRIVE 0
#endif

#if USE_UF2_DRIVE
#include "drivers/uf2_ghostfat.h"
#endif

enum MassstorageLuns {
  STORAGE_SDCARD_LUN,

#if USE_UF2_DRIVE
  STORAGE_UF2_LUN,
#endif

  STORAGE_LUN_NBR
};

/** USB Mass storage Standard Inquiry Data. */
const uint8_t STORAGE_Inquirydata[] = {/* 36 */
  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  USB_MANUFACTURER,                        /* Manufacturer : 8 bytes */
  USB_PRODUCT,                             /* Product      : 16 Bytes */
  'R', 'a', 'd', 'i', 'o', ' ', ' ', ' ',
  '1', '.', '0', '0',                      /* Version      : 4 Bytes */

#if USE_UF2_DRIVE
  /* LUN 1 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  USB_MANUFACTURER,                        /* Manufacturer : 8 bytes */
  USB_PRODUCT,                             /* Product      : 16 Bytes */
  'R', 'a', 'd', 'i', 'o', ' ', ' ', ' ',
  '1', '.', '0' ,'0',                      /* Version      : 4 Bytes */
#endif
};

static int8_t STORAGE_Init(uint8_t lun);
static int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady(uint8_t lun);
static int8_t STORAGE_IsWriteProtected(uint8_t lun);
static int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

USBD_StorageTypeDef USBD_Storage_Interface_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata
};

int8_t STORAGE_Init(uint8_t lun)
{
#if USE_UF2_DRIVE
  if (lun == STORAGE_UF2_LUN) {
    uf2_fat_reset_state();
    return USBD_OK;
  }
#endif

  disk_initialize(0);
  return USBD_OK;
}

/**
  * @brief  return medium capacity and block size
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  if (lun >= STORAGE_LUN_NBR) return USBD_FAIL;
  
#if USE_UF2_DRIVE
  if (lun == STORAGE_UF2_LUN) {
    *block_size = BLOCK_SIZE;
    *block_num = UF2_NUM_BLOCKS;
    return USBD_OK;
  }
#endif

  if (!SD_CARD_PRESENT())
    return USBD_FAIL;

  *block_size = BLOCK_SIZE;

  static DWORD sector_count = 0;
  if (sector_count == 0) {
    auto drv = storageGetDefaultDriver();
    if (drv->ioctl(0, GET_SECTOR_COUNT, &sector_count) != RES_OK) {
      sector_count = 0;
      return USBD_FAIL;
    }
  }

  *block_num  = sector_count;
  return USBD_OK;
}

uint8_t lunReady[STORAGE_LUN_NBR];

void usbInitLUNs()
{
  for (int i = 0; i < STORAGE_LUN_NBR; i++) {
    lunReady[i] = 1;
  }
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
  switch (lun) {
    case STORAGE_SDCARD_LUN:
      return (lunReady[STORAGE_SDCARD_LUN] != 0 && storageIsPresent())
                 ? USBD_OK
                 : USBD_FAIL;

#if USE_UF2_DRIVE
    case STORAGE_UF2_LUN:
      return USBD_OK;
#endif

    default:
      return USBD_FAIL;
  }
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  return (USBD_OK);
}

/**
  * @brief  Read data from the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to save data
  * @param  blk_addr :  address of 1st block to be read
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */

int8_t STORAGE_Read (uint8_t lun,
                   uint8_t *buf,
                   uint32_t blk_addr,
                   uint16_t blk_len)
{
  WATCHDOG_SUSPEND(100/*1s*/);

#if USE_UF2_DRIVE
  if (lun == STORAGE_UF2_LUN) {
    uf2_fat_read_block(blk_addr, buf);
    return 0;
  }
#endif

  auto drv = storageGetDefaultDriver();
  return (drv->read(0, buf, blk_addr, blk_len) == RES_OK) ? USBD_OK : USBD_FAIL;
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  WATCHDOG_SUSPEND(500/*5s*/);

#if USE_UF2_DRIVE
  if (lun == STORAGE_UF2_LUN) {
    int wr_ret;
    while ((blk_len--) && (wr_ret = uf2_fat_write_block(blk_addr, buf)) > 0) {
      blk_addr += 512;
      buf += 512;
    }
    // return wr_ret > 0 ? USBD_OK : USBD_FAIL;
    return USBD_OK;
  }
#endif

  auto drv = storageGetDefaultDriver();
  return (drv->write(0, buf, blk_addr, blk_len) == RES_OK) ? USBD_OK : USBD_FAIL;
}

/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */

int8_t STORAGE_GetMaxLun(void)
{
  return (STORAGE_LUN_NBR - 1);
}
