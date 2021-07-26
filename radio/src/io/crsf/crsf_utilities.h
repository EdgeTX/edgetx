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

#ifndef __UTILITIES_H__  /* Guard against multiple inclusion */
#define __UTILITIES_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <limits.h>
#include <stdbool.h>
#include "crc8.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#define BITMASK_8(b)                    (1 << ((b) % CHAR_BIT))
#define BITSLOT(b)                      ((b) / CHAR_BIT)
#define BITSET(a, b)                    ((a)[BITSLOT(b)] |= BITMASK_8(b))
#define BITCLEAR(a, b)                  ((a)[BITSLOT(b)] &= ~BITMASK_8(b))
#define BITTEST(a, b)                   ((a)[BITSLOT(b)] & BITMASK_8(b))
#define BITNSLOTS(nb)                   ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define LIBUTIL_ARRAY_SIZE(array)       (sizeof(array) / sizeof(array[0]))

#define HW_ID_MASK                        (uint32_t) 0xFFFFFF00

/* ******************** Public Functions and Procedures ******************** */


/* *********************** Write Byte Array Functions ********************** */
void libUtilClearBuffer(uint8_t * arrayPointer, uint32_t * increment, uint32_t arraySize);
void libUtilWrite8(uint8_t * arrayPointer, uint32_t * increment, uint8_t inputData);
void libUtilWrite16(uint8_t * arrayPointer, uint32_t * increment, uint16_t inputData);
void libUtilWrite24(uint8_t * arrayPointer, uint32_t * increment, uint32_t inputData);
void libUtilWrite32(uint8_t * arrayPointer, uint32_t * increment, uint32_t inputData);
uint32_t libUtilWriteString(uint8_t * arrayPointer, uint32_t * increment, char * inputStr, bool nullendbyte);
void libUtilWriteBytes(uint8_t * out_array, uint32_t * increment, uint8_t * in_array, uint32_t needBytes);
void libUtilWriteEnd8(uint8_t * arrayPointer, uint32_t frameLengthAt, uint8_t cmdSize, uint8_t polynom);

/* *********************** Shift Byte Array Functions ********************** */
void libUtilRightShift(uint8_t * arrayPointer, uint16_t start, uint16_t end, uint16_t right_shift);

/* *********************** Read Byte Array Functions *********************** */
uint8_t libUtilRead8(uint8_t * arrayPointer, uint32_t * increment);
uint16_t libUtilRead16(uint8_t * arrayPointer, uint32_t * increment);
uint32_t libUtilRead24(uint8_t * arrayPointer, uint32_t * increment);
uint32_t libUtilRead32(uint8_t * arrayPointer, uint32_t * increment);
int8_t libUtilReadInt8(uint8_t * arrayPointer, uint32_t * increment);
int16_t libUtilReadInt16(uint8_t * arrayPointer, uint32_t * increment);
int32_t libUtilReadInt24(uint8_t * arrayPointer, uint32_t * increment);
int32_t libUtilReadInt32(uint8_t * arrayPointer, uint32_t * increment);
float libUtilReadFloat(uint8_t * arrayPointer, uint32_t * increment, uint8_t decimalPoint);
uint32_t libUtilReadString(uint8_t * arrayPointer, uint32_t * increment, char * outputStr, bool skipNullChar);

/* ************************************************************************* */
void libUtilHexToString(uint8_t * hex, uint8_t sizeOfHex, char * separator, bool capital, char * string);
uint8_t libUtilReverseUint8(uint8_t data);
uint16_t libUtilReverseUint16(uint16_t data);
uint32_t libUtilReverseUint32(uint32_t data);
/* ************************************************************************* */

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* __UTILITIES_H__ */

/* ****************************************************************************
 End of File
 */
