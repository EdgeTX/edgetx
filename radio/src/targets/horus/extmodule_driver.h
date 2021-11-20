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

#pragma once

#include <stdint.h>

void extmoduleStop();
void extmodulePpmStart();

#if defined(PXX1)
void extmodulePxx1PulsesStart();
void extmoduleSendNextFramePxx1(const uint32_t* pulses, uint16_t length);
#endif

// Soft serial on PPM pin
void extmoduleSerialStart();

#if defined(AFHDS3) && !(defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO))
void extmoduleSendNextFrameAFHDS3(const uint16_t* dataPtr, uint16_t dataSize);
#endif

void extmoduleSendNextFrameSoftSerial100kbit(const uint32_t* pulses, uint16_t length);

// Bitbang serial
void extmoduleSendInvertedByte(uint8_t byte);
