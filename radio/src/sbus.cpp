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

#include "opentx.h"
#include "sbus.h"
#include "timers_driver.h"

#define SBUS_FRAME_GAP_DELAY   1000 // 500uS

#define SBUS_START_BYTE        0x0F
#define SBUS_END_BYTE          0x00
#define SBUS_FLAGS_IDX         23
#define SBUS_FRAMELOST_BIT     2
#define SBUS_FAILSAFE_BIT      3

#define SBUS_CH_BITS           11
#define SBUS_CH_MASK           ((1<<SBUS_CH_BITS) - 1)

#define SBUS_CH_CENTER         0x3E0

// Range for pulses (ppm input) is [-512:+512]
void processSbusFrame(uint8_t * sbus, int16_t * pulses, uint32_t size)
{
  if (size != SBUS_FRAME_SIZE || sbus[0] != SBUS_START_BYTE || sbus[SBUS_FRAME_SIZE-1] != SBUS_END_BYTE) {
    return; // not a valid SBUS frame
  }
  if ((sbus[SBUS_FLAGS_IDX] & (1<<SBUS_FAILSAFE_BIT)) || (sbus[SBUS_FLAGS_IDX] & (1<<SBUS_FRAMELOST_BIT))) {
    return; // SBUS invalid frame or failsafe mode
  }

  sbus++; // skip start byte

  uint32_t inputbitsavailable = 0;
  uint32_t inputbits = 0;
  for (uint32_t i=0; i<MAX_TRAINER_CHANNELS; i++) {
    while (inputbitsavailable < SBUS_CH_BITS) {
      inputbits |= *sbus++ << inputbitsavailable;
      inputbitsavailable += 8;
    }
    *pulses++ = ((int32_t) (inputbits & SBUS_CH_MASK) - SBUS_CH_CENTER) * 5 / 8;
    inputbitsavailable -= SBUS_CH_BITS;
    inputbits >>= SBUS_CH_BITS;
  }

  ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;
}

void processSbusInput()
{
#if !defined(SIMU)
  uint8_t rxchar;
  uint32_t active = 0;
  static uint8_t SbusIndex = 0;
  static uint16_t SbusTimer;
  static uint8_t SbusFrame[SBUS_FRAME_SIZE];

  while (sbusGetByte(&rxchar)) {
    active = 1;
    if (SbusIndex > SBUS_FRAME_SIZE-1) {
      SbusIndex = SBUS_FRAME_SIZE-1;
    }
    SbusFrame[SbusIndex++] = rxchar;
  }
  if (active) {
    SbusTimer = getTmr2MHz();
    return;
  }
  else {
    if (SbusIndex) {
      if ((uint16_t) (getTmr2MHz() - SbusTimer) > SBUS_FRAME_GAP_DELAY) {
        processSbusFrame(SbusFrame, ppmInput, SbusIndex);
        SbusIndex = 0;
      }
    }
  }
#endif
}
