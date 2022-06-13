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

#include "lz4_bitmaps.h"
#include "libopenui/thirdparty/lz4/lz4.h"
#include "libopenui_helpers.h"

const uint8_t* _decompressed_mask(const uint8_t* lz4_compressed, uint8_t** raw)
{
  if (*raw == nullptr) {
    const uint16_t* hdr = (const uint16_t*)lz4_compressed;
    uint16_t width = hdr[0];
    uint16_t height = hdr[1];

    size_t len = *(uint32_t*)&hdr[2];

    // skip 8 bytes header
    lz4_compressed += 8;

    uint32_t pixels = width * height;
    *raw = (uint8_t*)malloc(align32(pixels + 4));

    uint16_t* raw_hdr = (uint16_t*)*raw;
    raw_hdr[0] = width;
    raw_hdr[1] = height;

    char* data = (char*)&raw_hdr[2];
    LZ4_decompress_safe((const char *)lz4_compressed, data, len, pixels);
  }

  return *raw;
}
