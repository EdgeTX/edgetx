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



#ifndef __CRC8_H__   
#define __CRC8_H__

#include <stdint.h>

#define POLYNOM_1           (uint8_t) 0xD5        /* Polynom = x^7+x^6+x^4+x^2+X^0  */
#define POLYNOM_2           (uint8_t) 0xBA        /* Polynom = x^7+x^5+x^4+x^3+X^1  */

#define CRC8_RESET_VALUE    (uint8_t) 0x00        /* Reset Value. Must be written into CRC8 Reg before calculating a new CRC */

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


void libCRC8Reset(uint8_t * crc8);
void libCRC8Calc(uint8_t dataIn, uint8_t * crc8, uint8_t polynom);
uint8_t getLibCRC8(uint8_t * crc8, uint8_t polynom);


void libCRC8AddMbusCRC(uint8_t * pArr, uint8_t polynom);
uint8_t libCRC8GetCRCArr(uint8_t * pArr, uint8_t Length, uint8_t polynom);


/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* __CRC8_H__ */

/* *****************************************************************************
 * End of File
 */
