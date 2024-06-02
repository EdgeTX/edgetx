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

bool simuLcdRefresh = false;

void toplcdOff() {}

#if !defined(lcdOff)
void lcdOff() {}
#endif

#if !defined(COLORLCD)

pixel_t simuLcdBuf[DISPLAY_BUFFER_SIZE];

void lcdInit() {}

void lcdRefresh()
{
  // Mark screen dirty for async refresh
  simuLcdRefresh = true;

  memcpy(simuLcdBuf, displayBuf, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

#else

#include <lvgl/lvgl.h>

#if defined(LCD_VERTICAL_INVERT)
static pixel_t _LCD_BUF1[DISPLAY_BUFFER_SIZE] __SDRAM;
static pixel_t _LCD_BUF2[DISPLAY_BUFFER_SIZE] __SDRAM;

pixel_t* simuLcdBuf = _LCD_BUF1;
pixel_t* simuLcdBackBuf = _LCD_BUF2;
#if 0
// Copy 2 pixels at once to speed up a little
static void _copy_rotate_180(uint16_t* dst, uint16_t* src, const rect_t& copy_area)
{
  coord_t x1 = LCD_W - copy_area.w - copy_area.x;
  coord_t y1 = LCD_H - copy_area.h - copy_area.y;

  auto total = copy_area.w * copy_area.h;
  uint16_t* px_src = src + total - 2;

  auto px_dst = dst + y1 * LCD_W + x1;
  for (auto line = 0; line < copy_area.h; line++) {

    auto line_end = px_dst + (copy_area.w & ~1);
    while (px_dst != line_end) {
      uint32_t* px2_src = (uint32_t*)px_src;
      uint32_t* px2_dst = (uint32_t*)px_dst;

      uint32_t px = ((*px2_src & 0xFFFF0000) >> 16) | ((*px2_src & 0xFFFF) << 16);
      *px2_dst = px;

      px_src -= 2;
      px_dst += 2;
    }

    if (copy_area.w & 1) {
      *(px_dst++) = *(px_src+1);
      px_src--;
    }

    px_dst += LCD_W - copy_area.w;
  }
}

static void _rotate_area_180(lv_area_t& area)
{
  lv_coord_t tmp_coord;
  tmp_coord = area.y2;
  area.y2 = LCD_H - area.y1 - 1;
  area.y1 = LCD_H - tmp_coord - 1;
  tmp_coord = area.x2;
  area.x2 = LCD_W - area.x1 - 1;
  area.x1 = LCD_W - tmp_coord - 1;
}
#endif
static void _copy_screen_area(uint16_t* dst, uint16_t* src, const lv_area_t& copy_area)
{
  lv_coord_t x1 = copy_area.x1;
  lv_coord_t y1 = copy_area.y1;
  lv_coord_t area_w = copy_area.x2 - copy_area.x1 + 1;

  auto offset = y1 * LCD_W + x1;
  auto px_src = src + offset;
  auto px_dst = dst + offset;

  for (auto line = copy_area.y1; line <= copy_area.y2; line++) {
    memcpy(px_dst, px_src, area_w * sizeof(uint16_t));
    px_dst += LCD_W;
    px_src += LCD_W;
  }
}

static void _copy_area(uint16_t* dst, uint16_t* src, const rect_t& copy_area)
{
  lv_coord_t x1 = copy_area.x;
  lv_coord_t y1 = copy_area.y;

  auto offset = y1 * LCD_W + x1;
  auto px_src = src;
  auto px_dst = dst + offset;

  for (auto line = 0; line < copy_area.h; line++) {
    memcpy(px_dst, px_src, copy_area.w * sizeof(uint16_t));
    px_dst += LCD_W;
    px_src += copy_area.w;
  }
}
#else
pixel_t* simuLcdBuf = nullptr;
#endif

static void simuRefreshLcd(lv_disp_drv_t * disp_drv, uint16_t *buffer, const rect_t& copy_area)
{
#if !defined(LCD_VERTICAL_INVERT) // rename into "Use direct mode" ???
  // Direct mode: driver flush is called on final LVGL flush

  // simply set LVGL's buffer as our current frame buffer
  simuLcdBuf = buffer;

  // Trigger async refresh
  simuLcdRefresh = true;

#else
  // // copy / rotate current area
  // _copy_rotate_180(simuLcdBackBuf, buffer, copy_area);
  _copy_area(simuLcdBackBuf, buffer, copy_area);
  
  if (lv_disp_flush_is_last(disp_drv)) {
    // swap back/front
    if (simuLcdBuf == _LCD_BUF1) {
      simuLcdBuf = _LCD_BUF2;
      simuLcdBackBuf = _LCD_BUF1;
    } else {
      simuLcdBuf = _LCD_BUF1;
      simuLcdBackBuf = _LCD_BUF2;
    }

    // Trigger async refresh
    simuLcdRefresh = true;

    // Copy refreshed & rotated areas into new back buffer
    uint16_t* src = simuLcdBuf;
    uint16_t* dst = simuLcdBackBuf;

    lv_disp_t* disp = _lv_refr_get_disp_refreshing();
    for(int i = 0; i < disp->inv_p; i++) {
      if(disp->inv_area_joined[i]) continue;

      lv_area_t refr_area;
      lv_area_copy(&refr_area, &disp->inv_areas[i]);

      // TRACE("{%d,%d,%d,%d}", refr_area.x1,
      //       refr_area.y1, refr_area.x2, refr_area.y2);

      // _rotate_area_180(refr_area);
      _copy_screen_area(dst, src, refr_area);
    }
    
  } else {
    lv_disp_flush_ready(disp_drv);
  }  
#endif    
}

extern bool simu_shutdown;

static void simuLcdExitHandler(lv_disp_drv_t* disp_drv)
{
  if (simu_shutdown) {
    lv_disp_flush_ready(disp_drv);
  }
}

void lcdSetInitalFrameBuffer(void*) {}

void lcdInit()
{
#if defined(LCD_VERTICAL_INVERT)
  memset(_LCD_BUF1, 0, sizeof(_LCD_BUF1));
  memset(_LCD_BUF2, 0, sizeof(_LCD_BUF2));
#endif

  lcdSetWaitCb(simuLcdExitHandler);
  lcdSetFlushCb(simuRefreshLcd);
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

#define DMA2D_ARGB4444 ((uint32_t)0x00000004)

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
