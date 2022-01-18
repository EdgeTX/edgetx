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

#ifndef _SBUS_H_
#define _SBUS_H_

#define SBUS_BAUDRATE         100000
#define SBUS_FRAME_SIZE       25

// Setup SBUS AUX serial input
void sbusSetAuxGetByte(void* ctx, int (*fct)(void*, uint8_t*));

// SBUS AUX serial getter:
//  if set, it will fetch data from the handler set
//  with sbusSetAuxGetByte()
int sbusAuxGetByte(uint8_t* byte);

// Setup general SBUS input source
void sbusSetGetByte(int (*fct)(uint8_t*));

void processSbusInput();
void sbusTrainerPauseCheck();

#endif // _SBUS_H_
