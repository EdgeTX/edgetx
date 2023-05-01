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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SDIO_SD_H_
#define _SDIO_SD_H_

#include <stdint.h>

#define BLOCK_SIZE 512

/* Includes ------------------------------------------------------------------*/
typedef enum
{
  SD_ERROR,
  SD_OK = 0
} SD_Error;

/**
  * @brief  SDIO Transfer state
  */
typedef enum
{
  SD_TRANSFER_OK  = 0,
  SD_TRANSFER_BUSY = 1,
  SD_TRANSFER_ERROR
} SDTransferState;

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((uint8_t)0x01)
#define SD_NOT_PRESENT                             ((uint8_t)0x00)

SD_Error SD_Init(void);
SDTransferState SD_GetStatus(void);
uint8_t SD_Detect(void);
SD_Error SD_PowerOFF(void);
SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize);
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_WriteBlock(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize);
SD_Error SD_WriteMultiBlocks(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SDTransferState SD_GetTransferState(void);
SD_Error SD_WaitReadOperation(uint32_t timeout);
SD_Error SD_WaitWriteOperation(uint32_t timeout);

size_t SD_GetSectorCount();
size_t SD_GetSectorSize();
size_t SD_GetBlockSize();

#endif // _SDIO_SD_H_

