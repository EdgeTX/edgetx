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

#include "FatFs/diskio.h"
#include "fw_version.h"
#include "board.h"
#include "debug.h"

#include <string.h>

#if !defined(BOOT)
  #include "timers_driver.h"
  #define WATCHDOG_SUSPEND(x) watchdogSuspend(x)
#else
  #define WATCHDOG_SUSPEND(...)
#endif

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

#include "usbd_msc_mem.h"
#include "usb_conf.h"

enum MassstorageLuns {
  STORAGE_SDCARD_LUN,
  STORAGE_LUN_NBR
};

/* USB Mass storage Standard Inquiry Data */
const unsigned char STORAGE_Inquirydata[] = { //18
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (USBD_STD_INQUIRY_LENGTH - 5),
  0x00,
  0x00,	
  0x00,
  USB_MANUFACTURER,                        /* Manufacturer : 8 bytes */
  USB_PRODUCT,                             /* Product      : 16 Bytes */
  'R', 'a', 'd', 'i', 'o', ' ', ' ', ' ',
  '1', '.', '0', '0',                      /* Version      : 4 Bytes */
};

#define RESERVED_SECTORS (1 /*Boot*/ + 2 /*Fat table */ + 1 /*Root dir*/ + 8 /* one cluster for firmware.txt */)

int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun, 
                           uint32_t *block_num, 
                           uint32_t *block_size);

int8_t STORAGE_IsReady (uint8_t lun);

int8_t STORAGE_IsWriteProtected (uint8_t lun);

int8_t STORAGE_Read (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_Write (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_GetMaxLun (void);

const USBD_STORAGE_cb_TypeDef USBD_MICRO_SDIO_fops =    // modified my OpenTX
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

const USBD_STORAGE_cb_TypeDef  * const USBD_STORAGE_fops = &USBD_MICRO_SDIO_fops;    // modified my OpenTX

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

int8_t STORAGE_Init (uint8_t lun)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

/* TODO if no SD ... if( SD_Init() != 0)
  {
    return (-1); 
  } 
*/
  return (0);
}

/**
  * @brief  return medium capacity and block size
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
  if (!SD_CARD_PRESENT())
    return -1;
  
  *block_size = BLOCK_SIZE;

  static DWORD sector_count = 0;
  if (sector_count == 0) {
    if (disk_ioctl(0, GET_SECTOR_COUNT, &sector_count) != RES_OK) {
      sector_count = 0;
      return -1;
    }
  }

  *block_num  = sector_count;

  return 0;
}

uint8_t lunReady[STORAGE_LUN_NBR];

void usbInitLUNs()
{
  lunReady[STORAGE_SDCARD_LUN] = 1;
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{ 
  return (lunReady[STORAGE_SDCARD_LUN] != 0 && SD_CARD_PRESENT()) ? 0 : -1;
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
  return  0;
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
  
  // read without cache
  return (__disk_read(0, buf, blk_addr, blk_len) == RES_OK) ? 0 : -1;
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */

int8_t STORAGE_Write (uint8_t lun, 
                  uint8_t *buf, 
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
  WATCHDOG_SUSPEND(100/*1s*/);
  
  // write without cache
  return (__disk_write(0, buf, blk_addr, blk_len) == RES_OK) ? 0 : -1;
}

/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */

int8_t STORAGE_GetMaxLun (void)
{
  return STORAGE_LUN_NBR - 1;
}
