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

/* ******************************** Includes ******************************* */
#include "crsf_utilities.h"

/* *********************** Write Byte Array Functions ********************** */
void libUtilClearBuffer(uint8_t * arrayPointer, uint32_t * increment, uint32_t arraySize)
{
  memset(arrayPointer, 0, arraySize);
  *increment = 0;
}

void libUtilWrite8(uint8_t * arrayPointer, uint32_t * increment, uint8_t inputData)
{
  *(arrayPointer + (*increment)++) = inputData;
}

void libUtilWrite16(uint8_t * arrayPointer, uint32_t * increment, uint16_t inputData)
{
  libUtilWrite8(arrayPointer, increment, (inputData >> 8) & 0xFF);
  libUtilWrite8(arrayPointer, increment, inputData & 0xFF);
}

void libUtilWrite24(uint8_t * arrayPointer, uint32_t * increment, uint32_t inputData)
{
  libUtilWrite8(arrayPointer, increment, (inputData >> 16) & 0xFF);
  libUtilWrite8(arrayPointer, increment, (inputData >> 8) & 0xFF);
  libUtilWrite8(arrayPointer, increment, inputData & 0xFF);
}

void libUtilWrite32(uint8_t * arrayPointer, uint32_t * increment, uint32_t inputData)
{
  libUtilWrite16(arrayPointer, increment, (inputData >> 16) & 0xFFFF);
  libUtilWrite16(arrayPointer, increment, inputData & 0xFFFF);
}


uint32_t libUtilWriteString(uint8_t * arrayPointer, uint32_t * increment, char * inputStr, bool nullendbyte)
{
  uint32_t i = 0;
  while (inputStr[i] != '\0') {
    libUtilWrite8(arrayPointer, increment, inputStr[i++]);
  }
  if (nullendbyte) {
    libUtilWrite8(arrayPointer, increment, 0);
  }
  return i;
}

void libUtilWriteBytes(uint8_t * out_array, uint32_t * increment, uint8_t * inArray, uint32_t needBytes)
{
  uint32_t i = 0;
  for (i = 0; i < needBytes; i++) {
    libUtilWrite8(out_array, increment, inArray[i]);
  }
}

void libUtilWriteEnd8(uint8_t * arrayPointer, uint32_t frameLengthAt, uint8_t cmdSize, uint8_t polynom)
{
  libUtilWrite8(arrayPointer, &frameLengthAt, cmdSize);
  libCRC8AddMbusCRC(arrayPointer, polynom);
}

/* *********************** Shift Byte Array Functions ********************** */
void libUtilRightShift(uint8_t * arrayPointer, uint16_t start, uint16_t end, uint16_t right_shift)
{
  uint16_t i;
  for (i = end; i <= start; i--) {
    *(arrayPointer + (i +  right_shift)) = *(arrayPointer + i);
  }
}

/* *********************** Read Byte Array Functions *********************** */
uint8_t libUtilRead8(uint8_t * arrayPointer, uint32_t * increment)
{
  return arrayPointer[(*increment)++];
}

uint16_t libUtilRead16(uint8_t * arrayPointer, uint32_t * increment)
{
  return ((((uint16_t)libUtilRead8(arrayPointer, increment)) << 8) | libUtilRead8(arrayPointer, increment));
}

uint32_t libUtilRead24(uint8_t * arrayPointer, uint32_t * increment)
{
  return ((((uint32_t)libUtilRead8(arrayPointer, increment)) << 16) | (((uint32_t)libUtilRead8(arrayPointer, increment)) << 8 ) | libUtilRead8(arrayPointer, increment));
}

uint32_t libUtilRead32(uint8_t * arrayPointer, uint32_t * increment)
{
  return ((((uint32_t)libUtilRead16( arrayPointer, increment)) << 16) | libUtilRead16(arrayPointer, increment));
}

int8_t libUtilReadInt8(uint8_t * arrayPointer, uint32_t * increment)
{
  return (int8_t)libUtilRead8(arrayPointer, increment);
}

int16_t libUtilReadInt16(uint8_t * arrayPointer, uint32_t * increment)
{
  return (int16_t)libUtilRead16(arrayPointer, increment);
}

int32_t libUtilReadInt24(uint8_t * arrayPointer, uint32_t * increment)
{
  return (int32_t)libUtilRead32( arrayPointer, increment);
}

int32_t libUtilReadInt32(uint8_t * arrayPointer, uint32_t * increment)
{
  return (int32_t)libUtilRead32( arrayPointer, increment);
}

float libUtilReadFloat(uint8_t * arrayPointer, uint32_t * increment, uint8_t decimalPoint)
{
  return (((float)libUtilReadInt32(arrayPointer, increment)) / pow(10, decimalPoint));
}

uint32_t libUtilReadString(uint8_t * arrayPointer, uint32_t * increment, char * outputStr, bool skipNullChar)
{
  uint32_t i = 0;
  while (arrayPointer[(*increment)] != '\0') {
    outputStr[i++] = libUtilRead8(arrayPointer, increment);
  }
  if (skipNullChar) {
    (*increment)++;
  }
  return i;
}

void libUtilHexToString(uint8_t * hex, uint8_t sizeof_hex, char * separator, bool capital, char * string)
{
  uint8_t i = 0;
  for (i = 0; i < sizeof_hex; i++) {
    if (capital) {
      sprintf(string, "%s%s%02X", string, separator, hex[i]);
    }
    else {
      sprintf(string, "%s%s%02x", string, separator, hex[i]);
    }
  }
}

uint8_t libUtilReverseUint8(uint8_t data)
{
  uint8_t rval = 0;
  uint8_t i = 0;
  for (i = 0; i < 8; i++) {
    if ((data & (1 << i)) != 0) {
      rval |= (0x80 >> i);
    }
  }
  return rval;
}

uint16_t libUtilReverseUint16(uint16_t data)
{
  return (((uint16_t)libUtilReverseUint8(data & 0xFF) << 8) | libUtilReverseUint8((data>>8) & 0xFF));
}

uint32_t libUtil_ReverseUint32(uint32_t data)
{
  return (((uint32_t)libUtilReverseUint16(data & 0xFFFF) << 16) | libUtilReverseUint16((data>>16) & 0xFFFF));
}
/* ****************************************************************************
 End of File
 */
