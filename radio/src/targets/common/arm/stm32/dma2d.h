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

#include "edgetx_types.h"

#if !defined(SIMU)
static inline void DMAWait()
{
  while(DMA2D->CR & DMA2D_CR_START);
}
#else
static inline void DMAWait() {}
#endif

void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaMask(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint8_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h, uint16_t bg_color);
void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format);
