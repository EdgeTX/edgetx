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

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>

#define MAX_AUX_SERIAL 2

void    initSerialPorts();
uint8_t serialGetMode(int port_nr);
uint8_t serialTracesEnabled(int port_nr);
void    serialInit(int port_nr, int mode);
void    serialStop(int port_nr);
void    serialPutc(int port_nr, uint8_t c);

#if defined(AUX_SERIAL)
#define auxSerialSbusInit()      serialInit(0, UART_MODE_SBUS_TRAINER)
#define auxSerialGetMode()       serialGetMode(0)
#define auxSerialTracesEnabled() serialTracesEnabled(0)
#define auxSerialStop()          serialStop(0)
#define auxSerialPutc(c)         serialPutc(0, (c))
#endif

#if defined(AUX2_SERIAL)
#define aux2SerialSbusInit()      serialInit(1, UART_MODE_SBUS_TRAINER)
#define aux2SerialGetMode()       serialGetMode(1)
#define aux2SerialTracesEnabled() serialTracesEnabled(1)
#define aux2SerialStop()          serialStop(1)
#define aux2SerialPutc(c)         serialPutc(1, (c))
#endif

//
// Functions used by debug.h
//
#ifdef __cplusplus
extern "C" {
#endif

void dbgSerialPutc(char c);
void dbgSerialPrintf(const char *format, ...);
void dbgSerialCrlf();

#ifdef __cplusplus
}
#endif

#define dbgSerialPrint(...) do { dbgSerialPrintf(__VA_ARGS__); dbgSerialCrlf(); } while(0)

// Debug serial callback
void dbgSerialSetSendCb(void* ctx, void (*cb)(void*, uint8_t));

#endif // _SERIAL_H_

