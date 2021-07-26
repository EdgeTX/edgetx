/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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


#include "crc8.h"


#define CRC8_FINISH_VALUE           ( uint8_t ) 0x00    /* CRC8_Calc musst be called with this value to finish the calculation */
#define BYTE_SIZE                   ( uint8_t ) 0x08    /* Bit Size of one Byte */
#define MSB_SET                     ( uint8_t ) 0x80    /* MSB Bit is set in this byte */
#define MSB_FLAG_CLEAR              ( uint8_t ) 0x00    /* MSB Bit flag is not set */
#define MSB_FLAG_SET                ( uint8_t ) 0x01    /* MSB Bit flag is set*/


uint8_t ExpLibCRC8 = CRC8_RESET_VALUE;


void libCRC8Reset(uint8_t * CRC8)
{
  (*CRC8) = CRC8_RESET_VALUE;
}

void libCRC8Calc(uint8_t dataIn, uint8_t * CRC8, uint8_t polynom)
{
  uint8_t bitCnt;
  uint8_t msbFlag;

  for (bitCnt = 0; bitCnt < BYTE_SIZE; bitCnt++) {
    msbFlag = MSB_FLAG_CLEAR;
    if ((*CRC8) & MSB_SET) msbFlag = MSB_FLAG_SET;
      (*CRC8) <<= 1;
    if (dataIn & MSB_SET)
      (*CRC8)++;
    dataIn <<= 1;
    if (msbFlag == MSB_FLAG_SET)
      (*CRC8) ^= polynom;
  }
}

uint8_t getLibCRC8(uint8_t * CRC8, uint8_t polynom)
{
  /* Finisch CRC calculation */
  libCRC8Calc(CRC8_FINISH_VALUE, CRC8, polynom);
  return (*CRC8);
}

/* *pArr needs to point to lenght byte of array */
void libCRC8AddMbusCRC(uint8_t * pArr, uint8_t polynom)
{
  uint8_t length;

  length = *pArr++ - 1;
  *(pArr + length) = libCRC8GetCRCArr(pArr, length, polynom);
  //printf("CRC at: %03d; CRC: %02x\r\n", Length, *( pArr + Length ));
}

uint8_t libCRC8GetCRCArr(uint8_t * pArr, uint8_t length, uint8_t polynom)
{
  uint8_t i;

  libCRC8Reset(&ExpLibCRC8);
  for (i = 0; i < length; i++) {
    libCRC8Calc(*(pArr + i), &ExpLibCRC8, polynom);
  }
  return getLibCRC8(&ExpLibCRC8, polynom);
}

