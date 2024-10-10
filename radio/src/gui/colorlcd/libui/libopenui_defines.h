/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "debug.h"
#include "hal.h"
#include "edgetx_types.h"
#include "colors.h"
#include "keys.h"

// Used by Lua API
#define INVERS                         0x01u
#define BLINK                          0x1000u
#define TIMEHOUR                       0x2000u

/* drawText flags */
#define LEFT                           0x00u /* align left */
#define VCENTERED                      0x02u /* align center vertically */
#define CENTERED                       0x04u /* align center */
#define RIGHT                          0x08u /* align right */
#define SHADOWED                       0x80u /* black copy at +1 +1 */
// 0x1000u used by Lua in api_colorlcd.h
// 0x8000u used by Lua in api_colorlcd.h

/* drawNumber flags */
#define LEADING0                       0x10u
#define PREC1                          0x20u
#define PREC2                          0x30u
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

/* telemetry flags */
#define NO_UNIT                        0x40u

#define LV_OBJ_FLAG_ENCODER_ACCEL LV_OBJ_FLAG_USER_1
