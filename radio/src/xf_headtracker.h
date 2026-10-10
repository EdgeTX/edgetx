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

// XF (Xianfei) headtracker, expansion port set to "Wireless PPM" in
// CwHeadTracker: E7 9B, 8 x uint16 little-endian PPM channels in 1/24 us,
// CRC16-CCITT over the first 18 bytes, big-endian.
#define XFHT_BAUDRATE (115200)
#define XFHT_CHANNEL_COUNT (8)
#define XFHT_HEADER1 0xE7
#define XFHT_HEADER2 0x9B
#define XFHT_FRAME_LENGTH (2 + 2 * XFHT_CHANNEL_COUNT + 2)
#define XFHT_TICKS_PER_US (24)
#define XFHT_TIMEOUT_MS (500)
// Frames take under 2 ms every 40 ms: a byte after this gap starts one
#define XFHT_FRAME_GAP_MS (30)
// Roll, pitch and yaw are CH4-CH6 in CwHeadTracker's default mapping;
// CH7 and CH8 (mode, sensitivity) are not used
#define XFHT_FIRST_AXIS (3)
#define XFHT_AXIS_COUNT (3)

void xfhtSetSerialDriver(void* ctx, const etx_serial_driver_t* drv);

// Public for the unit tests, which cannot run the port timer
void xfhtParseByte(uint8_t c);

// Axis (0 roll, 1 pitch, 2 yaw) in the mixer's -1024..1024 range, 0 when no
// valid frame came in the last XFHT_TIMEOUT_MS
int16_t get_xfht_value(uint8_t axis);
