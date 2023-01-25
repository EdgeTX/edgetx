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

#ifndef _PULSES_PXX_H_
#define _PULSES_PXX_H_

#include "hal.h"
#include "pulses_common.h"

#define PXX_SEND_BIND                      0x01
#define PXX_SEND_FAILSAFE                  (1 << 4)
#define PXX_SEND_RANGECHECK                (1 << 5)

#define PXX2_LOWSPEED_BAUDRATE             230400
#define PXX2_HIGHSPEED_BAUDRATE            450000
#define PXX2_NO_HEARTBEAT_PERIOD           4000/*us*/
#define PXX2_MAX_HEARTBEAT_PERIOD          (9000 + 1000)/*us longest period (isrm/accst) + 1ms heartbeat backup*/
#if defined(INTMODULE_HEARTBEAT)
  #define PXX2_PERIOD                      PXX2_MAX_HEARTBEAT_PERIOD
#else
  #define PXX2_PERIOD                      PXX2_NO_HEARTBEAT_PERIOD
#endif
#define PXX2_TOOLS_PERIOD                  1000/*us*/
#define PXX2_FRAME_MAXLENGTH               64

#define PXX_PULSES_PERIOD                  9000/*us*/
#define EXTMODULE_PXX1_SERIAL_PERIOD       4000/*us*/
#define EXTMODULE_PXX1_SERIAL_BAUDRATE     420000

#if defined(PXX_FREQUENCY_HIGH)
  #define INTMODULE_PXX1_SERIAL_BAUDRATE   450000
  #if defined(INTMODULE_HEARTBEAT)
    // use backup trigger (1 ms later)
    #define INTMODULE_PXX1_SERIAL_PERIOD   (4000 + 1000)/*us*/
  #else
    #define INTMODULE_PXX1_SERIAL_PERIOD   4000/*us*/
  #endif
#else
  #define INTMODULE_PXX1_SERIAL_BAUDRATE   115200
  #if defined(INTMODULE_HEARTBEAT)
    // use backup trigger (1 ms later)
    #define INTMODULE_PXX1_SERIAL_PERIOD   (9000 + 1000)/*us*/
  #else
    #define INTMODULE_PXX1_SERIAL_PERIOD   9000/*us*/
  #endif
#endif

#endif
