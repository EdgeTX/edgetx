/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

//
// Obsoleted definitions:
//  -> please check against libopenui_defines.h for conflicts
//  -> here we use the 4 most significant bits for our flags (32 bit unsigned)
//
// INVERS & BLINK are used in most scripts, let's offer a compatibility mode.
//
#undef INVERS
#undef BLINK

#define INVERS 0x10000000u
#define BLINK  0x20000000u

constexpr coord_t INVERT_BOX_MARGIN = 2;

extern BitmapBuffer* luaLcdBuffer;
