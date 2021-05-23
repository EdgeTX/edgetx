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

#include "lcd.h"
#include "simulcd.h"
#include <string.h>
#include <utility>

pixel_t simuLcdBuf[DISPLAY_BUFFER_SIZE];
pixel_t simuLcdBackupBuf[DISPLAY_BUFFER_SIZE];

bool simuLcdRefresh = true;

void toplcdOff() {}

#if !defined(lcdOff)
void lcdOff() {}
#endif

void lcdCopy(void *dest, void *src)
{
  memcpy(dest, src, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

uint16_t *lcdGetBackupBuffer() { return (uint16_t *)simuLcdBackupBuf; }

#if !defined(COLORLCD)

void lcdInit() {}

void lcdRefresh()
{
  // Mark screen dirty for async refresh
  simuLcdRefresh = true;

#if defined(LCD_VERTICAL_INVERT)
  auto src = displayBuf + DISPLAY_BUFFER_SIZE - 1;
  auto dst = simuLcdBuf;
  auto end = dst + DISPLAY_BUFFER_SIZE;

  while (dst != end) {
    *(dst++) = *(src--);
  }
#else
  memcpy(simuLcdBuf, displayBuf, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
#endif
}

#else

pixel_t displayBuf1[DISPLAY_BUFFER_SIZE];
pixel_t displayBuf2[DISPLAY_BUFFER_SIZE];
pixel_t scratchBuf[DISPLAY_BUFFER_SIZE];

BitmapBuffer _lcd1(BMP_RGB565, LCD_W, LCD_H, displayBuf1);
BitmapBuffer _lcd2(BMP_RGB565, LCD_W, LCD_H, displayBuf2);

BitmapBuffer * lcd = &_lcd1;
BitmapBuffer * lcdFront = &_lcd2;

uint16_t *lcdGetScratchBuffer() { return static_cast<uint16_t *>(scratchBuf); }

void lcdStoreBackupBuffer()
{
  memcpy(simuLcdBackupBuf, lcd->getData(),
         DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

int lcdRestoreBackupBuffer()
{
  memcpy(lcd->getData(), simuLcdBackupBuf,
         DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
  return 1;
}

void lcdRefresh()
{
  // Mark screen dirty for async refresh
  simuLcdRefresh = true;

  pixel_t* lcdData = lcd->getData();
  
#if defined(LCD_VERTICAL_INVERT)
  auto src = lcdData + DISPLAY_BUFFER_SIZE - 1;
  auto dst = simuLcdBuf;
  auto end = dst + DISPLAY_BUFFER_SIZE;

  while (dst != end) {
    *(dst++) = *(src--);
  }
#else
  memcpy(simuLcdBuf, lcdData, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
#endif

  // Swap back & front buffers
  std::swap(lcd, lcdFront);
}

void lcdInit()
{
  _lcd1.clear();
  _lcd2.clear();
}

void DMAFillRect(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                 uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
#if defined(LCD_VERTICAL_INVERT)
  x = destw - (x + w);
  y = desth - (y + h);
#endif

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      dest[(y + i) * destw + x + j] = color;
    }
  }
}

void DMACopyBitmap(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                   uint16_t y, const uint16_t *src, uint16_t srcw,
                   uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                   uint16_t h)
{
#if defined(LCD_VERTICAL_INVERT)
  x = destw - (x + w);
  y = desth - (y + h);
  srcx = srcw - (srcx + w);
  srcy = srch - (srcy + h);
#endif

  for (int i = 0; i < h; i++) {
    memcpy(dest + (y + i) * destw + x, src + (srcy + i) * srcw + srcx, 2 * w);
  }
}

// 'src' has ARGB4444
// 'dest' has RGB565
void DMACopyAlphaBitmap(uint16_t *dest, uint16_t destw, uint16_t desth,
                        uint16_t x, uint16_t y, const uint16_t *src,
                        uint16_t srcw, uint16_t srch, uint16_t srcx,
                        uint16_t srcy, uint16_t w, uint16_t h)
{
#if defined(LCD_VERTICAL_INVERT)
  x = destw - (x + w);
  y = desth - (y + h);
  srcx = srcw - (srcx + w);
  srcy = srch - (srcy + h);
#endif

  for (coord_t line = 0; line < h; line++) {
    uint16_t *p = dest + (y + line) * destw + x;
    const uint16_t *q = src + (srcy + line) * srcw + srcx;
    for (coord_t col = 0; col < w; col++) {
      uint8_t alpha = *q >> 12;
      uint8_t red =
          ((((*q >> 8) & 0x0f) << 1) * alpha + (*p >> 11) * (0x0f - alpha)) /
          0x0f;
      uint8_t green = ((((*q >> 4) & 0x0f) << 2) * alpha +
                       ((*p >> 5) & 0x3f) * (0x0f - alpha)) /
                      0x0f;
      uint8_t blue = ((((*q >> 0) & 0x0f) << 1) * alpha +
                      ((*p >> 0) & 0x1f) * (0x0f - alpha)) /
                     0x0f;
      *p = (red << 11) + (green << 5) + (blue << 0);
      p++;
      q++;
    }
  }
}

// 'src' has A8/L8?
// 'dest' has RGB565
void DMACopyAlphaMask(uint16_t *dest, uint16_t destw, uint16_t desth,
                      uint16_t x, uint16_t y, const uint8_t *src, uint16_t srcw,
                      uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                      uint16_t h, uint16_t fg_color)
{
#if defined(LCD_VERTICAL_INVERT)
  x = destw - (x + w);
  y = desth - (y + h);
  srcx = srcw - (srcx + w);
  srcy = srch - (srcy + h);
#endif

  RGB_SPLIT(fg_color, red, green, blue);

  for (coord_t line = 0; line < h; line++) {
    uint16_t *p = dest + (y + line) * destw + x;
    const uint8_t *q = src + (srcy + line) * srcw + srcx;
    for (coord_t col = 0; col < w; col++) {
      uint16_t opacity = *q >> 4;  // convert to 4 bits (stored in 8bit for DMA)
      uint8_t bgWeight = OPACITY_MAX - opacity;
      RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
      uint16_t r = (bgRed * bgWeight + red * opacity) / OPACITY_MAX;
      uint16_t g = (bgGreen * bgWeight + green * opacity) / OPACITY_MAX;
      uint16_t b = (bgBlue * bgWeight + blue * opacity) / OPACITY_MAX;
      *p = RGB_JOIN(r, g, b);
      p++;
      q++;
    }
  }
}

void DMABitmapConvert(uint16_t *dest, const uint8_t *src, uint16_t w,
                      uint16_t h, uint32_t format)
{
  if (format == DMA2D_ARGB4444) {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = ARGB(src[0], src[1], src[2], src[3]);
        ++dest;
        src += 4;
      }
    }
  } else {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = RGB(src[1], src[2], src[3]);
        ++dest;
        src += 4;
      }
    }
  }
}
#endif
