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

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

#include "usbd_msc_mem.h"
#include "usb_conf.h"

enum MassstorageLuns {
  STORAGE_SDCARD_LUN,
  STORAGE_EEPROM_LUN,
  STORAGE_SPI_FLASH_LUN,
  STORAGE_LUN_NBR
};

/* USB Mass storage Standard Inquiry Data */
const unsigned char STORAGE_Inquirydata[] = { //36
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
  /* LUN 1 */
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
  '1', '.', '0' ,'0',                      /* Version      : 4 Bytes */
  /* LUN 2 */
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
  '1', '.', '0' ,'0',                      /* Version      : 4 Bytes */
};

#define RESERVED_SECTORS (1 /*Boot*/ + 2 /*Fat table */ + 1 /*Root dir*/ + 8 /* one cluster for firmware.txt */)

int32_t fat12Write(const uint8_t * buffer, uint16_t sector, uint16_t count);
int32_t fat12Read(uint8_t * buffer, uint16_t sector, uint16_t count );

int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun, 
                           uint32_t *block_num, 
                           uint32_t *block_size);

int8_t  STORAGE_IsReady (uint8_t lun);

int8_t  STORAGE_IsWriteProtected (uint8_t lun);

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


uint16_t flashSpiGetSectorSize();
uint16_t flashSpiGetPageSize();
uint16_t flashSpiGetSectorCount();
int flashSpiErase(size_t address);
size_t flashSpiRead(size_t address, uint8_t* data, size_t size);
size_t flashSpiWrite(size_t address, const uint8_t* data, size_t size);


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
  if (lun == STORAGE_EEPROM_LUN) {
    *block_size = BLOCK_SIZE;
#if defined(EEPROM)
    *block_num  = RESERVED_SECTORS + EEPROM_SIZE/BLOCK_SIZE + FLASHSIZE/BLOCK_SIZE;
#else
    *block_num  = RESERVED_SECTORS + FLASHSIZE/BLOCK_SIZE;
#endif
    return 0;
  }  else if (lun == STORAGE_SPI_FLASH_LUN) {
#if !defined(BOOT)
    *block_num = flashSpiGetSectorCount();
    *block_size = flashSpiGetSectorSize();
//    *block_num = flashSpiGetSectorCount()*(flashSpiGetSectorSize()/512);
//    *block_size = 512;
#else
    *block_num = 0;
    *block_size = 0;
#endif
    return 0;
  }

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
  lunReady[STORAGE_EEPROM_LUN] = 1;
  lunReady[STORAGE_SPI_FLASH_LUN] = 1;
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{ 
#if defined(EEPROM)
  if (lun == STORAGE_EEPROM_LUN) {
    return (lunReady[STORAGE_EEPROM_LUN] != 0) ? 0 : -1;
  }
#endif
  if (lun == STORAGE_SPI_FLASH_LUN) {
    return (lunReady[STORAGE_SPI_FLASH_LUN] != 0) ? 0 : -1;
  }


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
  
  if (lun == STORAGE_EEPROM_LUN) {
    return (fat12Read(buf, blk_addr, blk_len) == 0) ? 0 : -1;
  } else if  (lun == STORAGE_SPI_FLASH_LUN) {
#if !defined(BOOT)
    uint16_t len = blk_len*flashSpiGetSectorSize();
    uint32_t ret = flashSpiRead(blk_addr*flashSpiGetSectorSize(), buf, len);
    return (ret==len)?0:-1;
#else
    return 0;
#endif
  }

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
  
  if (lun == STORAGE_EEPROM_LUN)	{
    return (fat12Write(buf, blk_addr, blk_len) == 0) ? 0 : -1;
  } else if  (lun == STORAGE_SPI_FLASH_LUN) {
#if !defined(BOOT)
    uint16_t sectSize = flashSpiGetSectorSize();
    uint16_t len = blk_len*sectSize;
    flashSpiErase(blk_addr*sectSize);
    uint32_t ret = 0;
    uint16_t writeSize = flashSpiGetPageSize();
    for(int i=0; i<len; i+=writeSize)
    {
      ret |= flashSpiWrite((blk_addr*sectSize)+i, buf+i, writeSize) != writeSize;
      if(ret)
        break;
    }
    return ret;
//    flashSpiWrite(blk_addr*512, buf, 256);
//    flashSpiWrite((blk_addr*512)+256, buf+256, 256);
//    return 0;
#else
    return 0;
#endif
  }


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


/* Firmware.txt */
const char firmware_txt[] =
#if defined(BOOT)
  "EdgeTX Bootloader"
#else
  "EdgeTX Firmware"
#endif
  " for " FLAVOUR "\r\n\r\n"
#if defined(BOOT)
  "BOOTVER    "
#else
  "FWVERSION  "
#endif
  "edgetx-" FLAVOUR "-" VERSION " (" GIT_STR ")\r\n"
  "DATE       " DATE "\r\n"
  "TIME       " TIME "\r\n"
#if !defined(BOOT)
  "BOOTVER   "
#else
  "FWVERSION "
#endif
  ;

//------------------------------------------------------------------------------
/**
 * FAT12 boot sector partition.
 */
#if defined(EEPROM)
#define TOTALSECTORS  (RESERVED_SECTORS + (EEPROM_SIZE/BLOCK_SIZE) +  (FLASHSIZE/BLOCK_SIZE))
#else
#define TOTALSECTORS  (RESERVED_SECTORS + (FLASHSIZE/BLOCK_SIZE))
#endif
const char g_FATboot[BLOCK_SIZE] =
{
  0xeb, 0x3c, 0x90, // Jump instruction.
  'O', 'p', 'e', 'n', 'T', 'x', 0x00, 0x00, // OEM Name
  0x00, 0x02, // Bytes per sector
  0x08, // Sectors per FS cluster.
  0x01, 0x00, // Reserved sector count

  0x01, // Number of FATs
  0x10, 0x00, // Number of root directory entries
  TOTALSECTORS & 0x00ff,  (TOTALSECTORS & 0xff00) >> 8, // Total sectors
  0xf8, // Media descriptor
  0x02, 0x00, // Sectors per FAT table
  0x20, 0x00, // Sectors per track
  0x40, 0x00, // Number of heads
  0x00, 0x00, 0x00, 0x00, // Number of hidden sectors

  0x00, 0x00, 0x00, 0x00, // Large number of sectors.
  0x00, // Physical drive number
  0x00, // Reserved
  0x29, // Extended boot signature
  'O', 'T', 'x', 0xD1, // Disk ID (serial number)
  'V', 'I', 'R', 'T', 'F', 'S', ' ', ' ', ' ', ' ', ' ', // Volume label
  'F', 'A', 'T', '1', '2', ' ', ' ', ' ', // FAT file system type
  0x00, 0x00, // OS boot code

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};


//	File Attributes
//	0 	0x01 	Read Only.
//	1 	0x02 	Hidden.
//	2 	0x04 	System.
//	3 	0x08 	Volume Label.
//	4 	0x10 	Subdirectory.
//	5 	0x20 	Archive.
//	6 	0x40 	Device.
//	7 	0x80 	Reserved.

typedef struct
{
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t create_time_ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t ea_index;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t start_cluster;
    uint32_t file_size;
} FATDirEntry_t;

// First 16 FAT root directory entries (1 sector)
const FATDirEntry_t g_DIRroot[] =
{
    {
        { USB_PRODUCT },
        { ' ', ' ', ' '},
        0x08,		// Volume
        0x00,
        0x00,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x00000000
    },
    {
      { 'F', 'I', 'R', 'M', 'W', 'A', 'R', 'E'},
      { 'T', 'X', 'T'},
      0x21,          // Readonly+Archive
      0x00,
      0x3E,
      0xA301,
      0x3D55,
      0x3D55,
      0x0000,
      0xA302,
      0x3D55,
      0x0002,
      sizeof(firmware_txt) + strlen(getFirmwareVersion())
    },
    {
      { 'F', 'I', 'R', 'M', 'W', 'A', 'R', 'E'},
      { 'B', 'I', 'N'},
#if defined(BOOT)
      0x20,          // Archive
#else
      0x21,          // Readonly+Archive
#endif
      0x00,
      0x3E,
      0xA301,
      0x3D55,
      0x3D55,
      0x0000,
      0xA302,
      0x3D55,
      0x0003,
      FLASHSIZE
  },
#if defined(EEPROM)
    {
        { 'E', 'E', 'P', 'R', 'O', 'M', ' ', ' '},
        { 'B', 'I', 'N'},
        0x20,		// Archive
        0x00,
        0x3E,
        0xA301,
        0x3D55,
        0x3D55,
        0x0000,
        0xA302,
        0x3D55,
        0x0003 + (FLASHSIZE/BLOCK_SIZE)/8,
        EEPROM_SIZE
    },
#endif
  // Emty entries are 0x00, omitted here. Up to 16 entries can be defined here
};

static void writeByte(uint8_t *buffer, uint16_t sector, int byte, uint8_t value)
{
  if (byte >= sector* BLOCK_SIZE && byte < (sector+1)* BLOCK_SIZE)
    buffer[byte - sector*BLOCK_SIZE] = value;
}

static void pushCluster(uint8_t *buffer, uint16_t sector, uint16_t & cluster, int & rest, uint16_t value)
{
  // boot sector is in front of FAT
  sector= sector-1;

  // First byte of the cluster
  int startbyte = cluster *3/2;
  if (cluster % 2 == 0) {
    // First 12 bit half
    rest = value >> 8;
    writeByte(buffer, sector, startbyte, value & 0xff);
  } else {
    // second 12 bit half, write rest and next byte
    writeByte(buffer, sector, startbyte, value << 4 | rest );
    writeByte(buffer, sector, startbyte+1, (value >> 4) & 0xff);
  }
  cluster++;
}

// count is number of 512 byte sectors
int32_t fat12Read(uint8_t * buffer, uint16_t sector, uint16_t count)
{
  while(count) {
    memset(buffer, 0x00, BLOCK_SIZE);
    if (sector == 0) {
      memcpy(buffer, g_FATboot, sizeof(g_FATboot) ) ;
    }
    else if (sector == 1 || sector == 2) {
      // FAT table. Generate on the fly to save the 1024 byte flash space
      uint16_t cluster=0;
      int rest;
      pushCluster (buffer, sector, cluster, rest, (uint16_t) 0xFF8);
      pushCluster (buffer, sector, cluster, rest, (uint16_t) 0xFFF);

      // Entry for firmware.txt, exactly one cluster
      pushCluster (buffer, sector, cluster, rest, (uint16_t) 0xFFF);

      // Entry for firmware.bin
      for (int i=0;i<FLASHSIZE/BLOCK_SIZE/8 -1;i++)
        pushCluster (buffer, sector, cluster, rest, cluster+1);
      pushCluster (buffer, sector, cluster, rest, (uint16_t) 0xFFF);

#if defined(EEPROM)
      // Entry for eeprom.bin
      for (int i=0;i<EEPROM_SIZE/BLOCK_SIZE/8 -1;i++)
        pushCluster (buffer, sector, cluster, rest, cluster+1);
      pushCluster (buffer, sector, cluster, rest, (uint16_t) 0xFFF);
#endif

      // Ensure last cluster is written if it is the first half
      pushCluster (buffer, sector, cluster, rest, (uint16_t)  0x000);

      // Rest is 0x0 as per memset
    }
    else if (sector == 3) {
      memcpy(buffer, g_DIRroot, sizeof(g_DIRroot) ) ;
    }
    else if (sector == 4) {
      memcpy(buffer, firmware_txt, sizeof(firmware_txt));
      strcpy((char *)(buffer + sizeof(firmware_txt)), getFirmwareVersion());
    }
    else if (sector < RESERVED_SECTORS) {
      // allocated to firmware.txt
    }
    else if (sector < RESERVED_SECTORS + (FLASHSIZE/BLOCK_SIZE )) {
      uint32_t address;
      address = sector - RESERVED_SECTORS;
      address *= BLOCK_SIZE;
      address += FIRMWARE_ADDRESS;
      memcpy(buffer, (uint8_t *)address, BLOCK_SIZE);
    }
#if defined(EEPROM)
    else if (sector < RESERVED_SECTORS + (EEPROM_SIZE/BLOCK_SIZE) + (FLASHSIZE/BLOCK_SIZE)) {
      eepromReadBlock(buffer, (sector - RESERVED_SECTORS - (FLASHSIZE/BLOCK_SIZE))*BLOCK_SIZE, BLOCK_SIZE);
    }
#endif
    buffer += BLOCK_SIZE ;
    sector++ ;
    count-- ;
  }
  return 0;
}



int32_t fat12Write(const uint8_t * buffer, uint16_t sector, uint16_t count)
{
  enum FatWriteOperation {
    FATWRITE_NONE,
    FATWRITE_EEPROM,
    FATWRITE_FIRMWARE
  };

  // Silence compiler warning that this is not used on X10
  __attribute__((unused)) static uint8_t operation = FATWRITE_NONE;

  TRACE("FAT12 Write(sector=%d, count=%d)", sector, count);

  if (sector < RESERVED_SECTORS) {
    // reserved, read-only
  }
  else if (sector < RESERVED_SECTORS + (FLASHSIZE/BLOCK_SIZE)) {
#if !defined(BOOT) // Don't allow overwrite of running firmware
    return -1;
#else
    // firmware
    uint32_t address;
    address = sector - RESERVED_SECTORS;
    address *= BLOCK_SIZE;
    address += FIRMWARE_ADDRESS;
    while (count) {
      for (uint32_t i=0; i<BLOCK_SIZE/FLASH_PAGESIZE; i++) {
        if (address >= FIRMWARE_ADDRESS+BOOTLOADER_SIZE/*protect bootloader*/ && address <= FIRMWARE_ADDRESS+FLASHSIZE-FLASH_PAGESIZE) {
          if (address == FIRMWARE_ADDRESS+BOOTLOADER_SIZE && isFirmwareStart(buffer)) {
            TRACE("FIRMWARE start found in sector %d", sector);
            operation = FATWRITE_FIRMWARE;
          }
          if (operation == FATWRITE_FIRMWARE) {
            flashWrite((uint32_t *)address, (uint32_t *)buffer);
          }
        }
        address += FLASH_PAGESIZE;
        buffer += FLASH_PAGESIZE;
      }
      sector++;
      count--;
      if (sector-RESERVED_SECTORS >= (FLASHSIZE/BLOCK_SIZE)) {
        TRACE("FIRMWARE end written at sector %d", sector-1);
        operation = FATWRITE_NONE;
      }
    }
#endif
  }
#if defined(EEPROM)
  else if (sector < RESERVED_SECTORS + (EEPROM_SIZE/BLOCK_SIZE) + (FLASHSIZE/BLOCK_SIZE)) {
    // eeprom
    while (count) {
      if (operation == FATWRITE_NONE && isEepromStart(buffer)) {
        TRACE("EEPROM start found in sector %d", sector);
        operation = FATWRITE_EEPROM;
      }
      if (operation == FATWRITE_EEPROM) {
        eepromWriteBlock((uint8_t *)buffer, (sector-RESERVED_SECTORS-(FLASHSIZE/BLOCK_SIZE))*BLOCK_SIZE, BLOCK_SIZE);
      }
      buffer += BLOCK_SIZE;
      sector++;
      count--;
      if (sector-RESERVED_SECTORS >= (EEPROM_SIZE/BLOCK_SIZE)+(FLASHSIZE/BLOCK_SIZE)) {
        TRACE("EEPROM end written at sector %d", sector-1);
        operation = FATWRITE_NONE;
      }
    }
  }
#endif
  return 0 ;
}
