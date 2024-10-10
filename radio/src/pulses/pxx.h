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

#define PXX_SEND_BIND                      0x01
#define PXX_SEND_FAILSAFE                  (1 << 4)
#define PXX_SEND_RANGECHECK                (1 << 5)

#define PXX2_LOWSPEED_BAUDRATE             230400
#define PXX2_HIGHSPEED_BAUDRATE            450000

/* microseconds */
#define PXX2_DEFAULT_PERIOD                4000

/* microseconds longest period (isrm/accst) + 1ms heartbeat backup*/
#define PXX2_MAX_HEARTBEAT_PERIOD          (9000 + 1000)

#define PXX2_TOOLS_PERIOD                  1000/*us*/
#define PXX2_FRAME_MAXLENGTH               64

#define PXX1_DEFAULT_PERIOD                9000/*us*/
#define PXX1_DEFAULT_SERIAL_BAUDRATE       115200

#define PXX1_FAST_PERIOD                   4000/*us*/
#define PXX1_FAST_SERIAL_BAUDRATE          450000

#define EXTMODULE_PXX1_SERIAL_PERIOD       4000/*us*/
#define EXTMODULE_PXX1_SERIAL_BAUDRATE     420000

// Releases S.PORT usage from
// the module driver using it.
bool pxxClearSPort();
