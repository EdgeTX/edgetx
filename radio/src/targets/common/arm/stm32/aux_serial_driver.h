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

#include <stdint.h>

#include "hal/serial_driver.h"

#if defined(AUX_SERIAL)
extern uint8_t auxSerialMode;
void auxSerialSetup(unsigned int baudrate, bool rx_enable,
                    uint8_t word_length = ETX_WordLength_8,
                    uint8_t parity = ETX_Parity_None,
                    uint8_t stop_bits = ETX_StopBits_One);
void auxSerialInit(unsigned int mode, unsigned int protocol);
void auxSerialPutc(uint8_t c);
void auxSerialSbusInit();
void auxSerialStop();
uint8_t auxSerialTracesEnabled();
#endif

#if defined(AUX2_SERIAL)
extern uint8_t aux2SerialMode;
void aux2SerialSetup(unsigned int baudrate, bool rx_enable,
                    uint8_t word_length = ETX_WordLength_8,
                    uint8_t parity = ETX_Parity_None,
                    uint8_t stop_bits = ETX_StopBits_One);
void aux2SerialInit(unsigned int mode, unsigned int protocol);
void aux2SerialPutc(uint8_t c);
void aux2SerialSbusInit();
void aux2SerialStop();
uint8_t aux2SerialTracesEnabled();
#endif

