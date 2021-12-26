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

#ifndef _OPENTX_TYPES_H_
#define _OPENTX_TYPES_H_

#include "thirdparty/libopenui/src/libopenui_types.h"

typedef uint32_t tmr10ms_t;
typedef int32_t getvalue_t;
typedef uint32_t mixsrc_t;
typedef int32_t swsrc_t;
typedef int16_t safetych_t;
typedef int16_t gvar_t;
typedef uint32_t bitfield_channels_t;
typedef uint16_t FlightModesType;
#if defined(PCBFRSKY) || defined(PCBNV14) || defined(PCBPL18)
typedef uint16_t source_t;
#else
typedef uint8_t source_t;
#endif

#endif // _OPENTX_TYPES_H_
