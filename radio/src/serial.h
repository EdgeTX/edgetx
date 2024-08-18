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
#include "hal/serial_port.h"

#define SERIAL_CONF_BITS_PER_PORT    8    /* configuration uses 1 byte per serial port */
#define SERIAL_CONF_MODE_MASK        0x0F /* lower 4 bits for mode */
#define SERIAL_CONF_POWER_BIT        7    /* MSBit of the configuration byte */

const etx_serial_port_t* serialGetPort(uint8_t port_nr);

int  serialGetMode(uint8_t port_nr);
void serialSetMode(uint8_t port_nr, int mode);

bool serialGetPower(uint8_t port_nr);
void serialSetPower(uint8_t port_nr, bool enabled);

uint32_t serialGetBaudrate(uint8_t port_nr);
void serialSetBaudrate(uint8_t port_nr, uint32_t baudrate);

int serialGetModePort(int mode);

void initSerialPorts();
void serialInit(uint8_t port_nr, int mode);
void serialStop(uint8_t port_nr);

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

// Query debug callback
void (*dbgSerialGetSendCb())(void*, uint8_t);
void* dbgSerialGetSendCbCtx();
