/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _MEMCPY_H_
#define _MEMCPY_H_

#include <string.h>

#if !defined(SIMU)

inline void safeMemcpy(void * dest, const void * src, unsigned int size)
{
  if (((uint32_t)dest & 0x03) || ((uint32_t)src & 0x03)) {
    for (unsigned i = 0; i < size; i++) {
      ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
    }
  }
  else {
    memcpy(dest, src, size);
  }
}

#define memcpy safeMemcpy

#endif

#endif // _HELPERS_H_
