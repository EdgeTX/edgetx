
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

#include <inttypes.h>

#define SPACEMOUSE_BAUDRATE               ( 38400 )
#define SPACEMOUSE_CHANNEL_COUNT          ( 6 )

#define SPACEMOUSE_INPUT_OFFSET           ( 8192 )
#define SPACEMOUSE_OUTPUT_RANGE           ( 700.0f )
#define SPACEMOUSE_OUTPUT_OFFSET          ( 350 )
#define SPACEMOUSE_OUTPUT_CONV_FACTOR     ( 2048.0f / SPACEMOUSE_OUTPUT_RANGE )  /* Internal channel resolution / SPACEMOUSE_OUTPUT_RANGE */

#define SPACEMOUSE_PROTO_HEADER           0x96
#define SPACEMOUSE_PROTO_FOOTER           0x8D

#define SPACEMOUSE_CMD_REQUEST_DATA       0xAC
#define SPACEMOUSE_CMD_SET_ZERO_POSITION  0xAD
#define SPACEMOUSE_CMD_AUTO_DATA_ON       0xAE
#define SPACEMOUSE_CMD_AUTO_DATA_OFF      0xAF

#define SPACEMOUSE_LENGTH_HEADER          1
#define SPACEMOUSE_LENGTH_DATA            (2*SPACEMOUSE_CHANNEL_COUNT)
#define SPACEMOUSE_LENGTH_CHECKSUM        2
#define SPACEMOUSE_LENGTH_FOOTER          1
#define SPACEMOUSE_LENGTH_TELEGRAM        (SPACEMOUSE_LENGTH_HEADER + SPACEMOUSE_LENGTH_DATA + SPACEMOUSE_LENGTH_CHECKSUM + SPACEMOUSE_LENGTH_FOOTER)


#if defined(DEBUG)
extern bool spacemouseTraceEnabled;
#endif

// Setup SpaceMouse serial port
void spacemouseSetSerialDriver(void* ctx, const etx_serial_driver_t* drv);

// Fetch SpaceMouse data
int16_t get_spacemouse_value(uint8_t ch);

// Periodic processing
void spacemouseStart();
void spacemouseStop();

void spacemousePoll();
void spacemouseTare();
void spacemouseStartStreaming();
void spacemouseStopStreaming();
