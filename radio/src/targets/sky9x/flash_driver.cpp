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

#include "edgetx.h"

uint32_t (*IAP_Function)(uint32_t, uint32_t);

void flashWrite(uint32_t * address, const uint32_t * buffer)    // size is 256 bytes
{
  uint32_t FlashSectorNum;
  uint32_t flash_cmd = 0;

  // Always initialise this here, setting a default doesn't seem to work
  IAP_Function = (uint32_t (*)(uint32_t, uint32_t)) *(( uint32_t *)0x00800008);
  FlashSectorNum = (uint32_t) address;
  FlashSectorNum >>= 8;// page size is 256 bytes
  FlashSectorNum &= 2047;// max page number

  /* Send data to the sector here */
  for (int i=0; i<FLASH_PAGESIZE/4; i++) {
    *address++ = *buffer++;
  }

  /* build the command to send to EEFC */
  flash_cmd = (0x5A << 24) | (FlashSectorNum << 8) | 0x03; //AT91C_MC_FCMD_EWP ;

  __disable_irq();
  /* Call the IAP function with appropriate command */
  i = IAP_Function(0, flash_cmd);
  __enable_irq();
}

uint32_t readLockBits()
{
  // Always initialise this here, setting a default doesn't seem to work
  IAP_Function = (uint32_t (*)(uint32_t, uint32_t)) *(( uint32_t *)0x00800008);

  uint32_t flash_cmd = (0x5A << 24) | 0x0A;//AT91C_MC_FCMD_GLB ;
  __disable_irq();
  (void) IAP_Function( 0, flash_cmd );
  __enable_irq();
  return EFC->EEFC_FRR;
}

void clearLockBits()
{
  uint32_t i;
  uint32_t flash_cmd = 0;

  // Always initialise this here, setting a default doesn't seem to work
  IAP_Function = (uint32_t (*)(uint32_t, uint32_t)) *(( uint32_t *)0x00800008);
  for ( i = 0; i < 16; i += 1 )
  {
    flash_cmd = (0x5A << 24) | ((128*i) << 8) | 0x09; //AT91C_MC_FCMD_CLB ;
    __disable_irq();
    /* Call the IAP function with appropriate command */
    (void) IAP_Function( 0, flash_cmd );
    __enable_irq();
  }
}

uint32_t isFirmwareStart(const uint8_t * buffer)
{
  const uint32_t * block = (const uint32_t *)buffer;

  if ((block[0] & 0xFFFE3000) != 0x20000000 ) {
    return 0;
  }
  if ((block[1] & 0xFFF80000) != 0x00400000) {
    return 0;
  }
  if ((block[2] & 0xFFF80000) != 0x00400000) {
    return 0;
  }
  return 1;
}
