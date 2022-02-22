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

#include "lcd.h"
#include "simulcd.h"
#include "rtos.h"
#include <string.h>
#include <utility>

pixel_t simuLcdBuf[DISPLAY_BUFFER_SIZE];

bool simuLcdRefresh = true;

void toplcdOff() {}

#if !defined(lcdOff)
void lcdOff() {}
#endif

void lcdCopy(void *dest, void *src)
{
  memcpy(dest, src, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

#if !defined(COLORLCD)

void lcdInit() {}

void lcdRefresh()
{
  // Mark screen dirty for async refresh
  simuLcdRefresh = true;

  memcpy(simuLcdBuf, displayBuf, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

#else

#include <lvgl/lvgl.h>

static void simuRefreshLcd(lv_disp_drv_t * disp_drv, uint16_t *buffer, const rect_t& copy_area)
{
#if defined(LCD_VERTICAL_INVERT)
  coord_t x1 = LCD_PHYS_W - copy_area.w - copy_area.x;
  coord_t y1 = LCD_PHYS_H - copy_area.h - copy_area.y;

  auto total = copy_area.w * copy_area.h;
  auto src = buffer + total - 1;
#else
  coord_t x1 = copy_area.x;
  coord_t y1 = copy_area.y;

  auto src = buffer;
#endif

  auto dst = simuLcdBuf + y1 * LCD_PHYS_W + x1;
  for (auto line = 0; line < copy_area.h; line++) {

    auto line_end = dst + copy_area.w;
    while (dst != line_end) {
#if defined(LCD_VERTICAL_INVERT)
      *(dst++) = *(src--);
#else
      *(dst++) = *(src++);
#endif
    }

    dst += LCD_PHYS_W - copy_area.w;
  }

  if (lv_disp_flush_is_last(disp_drv)) {
    // Mark screen dirty for async refresh
    simuLcdRefresh = true;
  } else {
    lv_disp_flush_ready(disp_drv);
  }
}

extern bool simu_shutdown;

static void simuLcdExitHandler(lv_disp_drv_t* disp_drv)
{
  if (simu_shutdown) {
    lv_disp_flush_ready(disp_drv);
  }
}

void lcdInit()
{
  memset(simuLcdBuf, 0, sizeof(simuLcdBuf));
  lcdSetWaitCb(simuLcdExitHandler);
  lcdSetFlushCb(simuRefreshLcd);
}

void DMAWait()
{
}

void DMAFillRect(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                 uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
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
