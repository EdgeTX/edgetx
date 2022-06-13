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

//
// LZ4 bitmaps definitions
//

#define DEFINE_LZ4_BITMAP(name)                                      \
  struct _uncomp_##name {                                            \
    static uint8_t* raw;                                             \
    operator const uint8_t*();                                       \
  };                                                                 \
  extern _uncomp_##name name

#define IMPL_LZ4_BITMAP(name)                                        \
  _uncomp_##name::operator const uint8_t*()                          \
  {                                                                  \
    return _decompressed_mask( _##name, &raw);                       \
  };                                                                 \
  uint8_t* _uncomp_##name::raw = nullptr;                            \
  _uncomp_##name name

#define STATIC_LZ4_BITMAP(name)                 \
    DEFINE_LZ4_BITMAP(name);                    \
    IMPL_LZ4_BITMAP(name)

const uint8_t* _decompressed_mask(const uint8_t* lz4_compressed, uint8_t** raw);
