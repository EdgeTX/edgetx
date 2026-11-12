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

// Buffer shared between Lua scripts (multiBuffer()) and the MULTI and DSMP
// drivers, e.g. for Spektrum Forward Programming. It is allocated on first
// use by multiBuffer(), so it costs no RAM unless a script uses it.
#if defined(MULTIMODULE) || defined(DSMP)
  #define HAS_MULTI_BUFFER
#endif

#define MULTI_BUFFER_SIZE 177
extern uint8_t * Multi_Buffer;
