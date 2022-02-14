/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it underresult the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <math.h>
#include "bitmapbuffer.h"
#include "libopenui_depends.h"
#include "libopenui_helpers.h"
#include "libopenui_file.h"
#include "font.h"

#include "lvgl/src/draw/sw/lv_draw_sw.h"



RLEBitmap::RLEBitmap(uint8_t format, const uint8_t* rle_data) :
  BitmapBufferBase<uint16_t>(format, 0, 0, nullptr)
{
  _width = *((uint16_t *)rle_data);
  _height = *(((uint16_t *)rle_data)+1);
  uint32_t pixels = _width * _height;
  data = (uint16_t*)malloc(align32(pixels * sizeof(uint16_t)));
  decode((uint8_t *)data, pixels * sizeof(uint16_t), rle_data+4);
  data_end = data + pixels;
}

RLEBitmap::~RLEBitmap()
{
  free(data);
}

int RLEBitmap::decode(uint8_t *dest, unsigned int destSize, const uint8_t *src)
{
  uint8_t prevByte = 0;
  bool prevByteValid = false;

  const uint8_t *destEnd = dest + destSize;
  uint8_t *d = dest;

  while (d < destEnd) {
    uint8_t byte = *src++;
    *d++ = byte;

    if (prevByteValid && byte == prevByte) {
      uint8_t count = *src++;

      if (d + count > destEnd) {
        TRACE("rle_decode_8bit: destination overflow!\n");
        return -1;
      }

      memset(d, byte, count);
      d += count;
      prevByteValid = false;
    } else {
      prevByte = byte;
      prevByteValid = true;
    }
  }

  return d - dest;
}

BitmapBuffer::BitmapBuffer(uint8_t format, uint16_t width, uint16_t height) :
    BitmapBufferBase<uint16_t>(format, width, height, nullptr),
    dataAllocated(true)
#if defined(DEBUG)
    , leakReported(false)
#endif
{
  data = (uint16_t *)malloc(align32(width * height * sizeof(uint16_t)));
  data_end = data + (width * height);

#if !defined(BOOT)
  // Assume we need a canvas here
  canvas = lv_canvas_create(nullptr);
  lv_canvas_set_buffer(canvas, data, width, height, LV_IMG_CF_TRUE_COLOR);
#endif
}

BitmapBuffer::BitmapBuffer(uint8_t format, uint16_t width, uint16_t height,
                           uint16_t *data) :
    BitmapBufferBase<uint16_t>(format, width, height, data),
    dataAllocated(false)
#if defined(DEBUG)
    , leakReported(false)
#endif
{
}

BitmapBuffer::~BitmapBuffer()
{
  if (dataAllocated) {
#if !defined(BOOT)
    lv_obj_del(canvas);
#endif
    free(data);
  }
}

template <class T>
void BitmapBuffer::drawBitmap(coord_t x, coord_t y, const T *bmp, coord_t srcx,
                              coord_t srcy, coord_t srcw, coord_t srch,
                              float scale)
{
  if (!data || !bmp) return;
  APPLY_OFFSET();
  if (x >= xmax || y >= ymax) return;
  drawBitmapAbs<T>(x, y, bmp, srcx, srcy, srcw, srch, scale);
}

template void BitmapBuffer::drawBitmap(
    coord_t, coord_t, BitmapBufferBase<unsigned short const> const *, coord_t,
    coord_t, coord_t, coord_t, float);

template void BitmapBuffer::drawBitmap(coord_t, coord_t, const BitmapBuffer *,
                                       coord_t, coord_t, coord_t, coord_t,
                                       float);

template void BitmapBuffer::drawBitmap(coord_t, coord_t, const RLEBitmap *,
                                       coord_t, coord_t, coord_t, coord_t,
                                       float);

template <class T>
void BitmapBuffer::drawScaledBitmap(const T *bitmap, coord_t x, coord_t y,
                                    coord_t w, coord_t h)
{
  if (bitmap) {
    float vscale = float(h) / bitmap->height();
    float hscale = float(w) / bitmap->width();
    float scale = vscale < hscale ? vscale : hscale;

    int xshift = (w - (bitmap->width() * scale)) / 2;
    int yshift = (h - (bitmap->height() * scale)) / 2;
    //TRACE("  BitmapBuffer::drawScaledBitmap()---- scale %f", scale);
    drawBitmap(x + xshift, y + yshift, bitmap, 0, 0, 0, 0, scale);
  }
}

template void BitmapBuffer::drawScaledBitmap(const BitmapBuffer *, coord_t,
                                             coord_t, coord_t, coord_t);

template <class T>
void BitmapBuffer::drawBitmapAbs(coord_t x, coord_t y, const T *bmp,
                                 coord_t srcx, coord_t srcy,
                                 coord_t srcw, coord_t srch,
                                 float scale)
{
  coord_t bmpw = bmp->width();
  coord_t bmph = bmp->height();
  //TRACE("  BitmapBuffer::drawBitmapAbs %dx%d %s->%s", bmpw, bmph,
  //           bmp->getFormat() == BMP_RGB565 ? "BMP_RGB565" : "BMP_ARGB4444",
  //                getFormat() == BMP_RGB565 ? "BMP_RGB565" : "BMP_ARGB4444");

  if (srcw == 0) srcw = bmpw;
  if (srch == 0) srch = bmph;
  if (srcx + srcw > bmpw) srcw = bmpw - srcx;
  if (srcy + srch > bmph) srch = bmph - srcy;

  if (scale == 0) {
    if (x < xmin) {
      srcw += x - xmin;
      srcx -= x - xmin;
      x = xmin;
    }
    if (y < ymin) {
      srch += y - ymin;
      srcy -= y - ymin;
      y = ymin;
    }
    if (x + srcw > xmax) {
      srcw = xmax - x;
    }
    if (y + srch > ymax) {
      srch = ymax - y;
    }
  } else {
    if (x < xmin) {
      srcw += (x - xmin) / scale;
      srcx -= (x - xmin) / scale;
      x = xmin;
    }
    if (y < ymin) {
      srch += (y - ymin) / scale;
      srcy -= (y - ymin) / scale;
      y = ymin;
    }
    if (x + srcw * scale > xmax) {
      srcw = (xmax - x) / scale;
    }
    if (y + srch * scale > ymax) {
      srch = (ymax - y) / scale;
    }
  }

  if (srcw <= 0 || srch <= 0) {
    return;
  }

  if (scale == 0) {
    if (bmp->getFormat() == BMP_ARGB4444) {
      DMACopyAlphaBitmap(data, _width, _height, x, y, bmp->getData(), bmpw,
                         bmph, srcx, srcy, srcw, srch);
    } else {
      DMACopyBitmap(data, _width, _height, x, y, bmp->getData(), bmpw, bmph,
                    srcx, srcy, srcw, srch);
    }
  } else {

    int scaledw = srcw * scale;
    int scaledh = srch * scale;

    if (x + scaledw > _width) scaledw = _width - x;
    if (y + scaledh > _height) scaledh = _height - y;

    if ( format == BMP_ARGB4444)  {

      for (int i = 0; i < scaledh; i++) {
        pixel_t *p = getPixelPtrAbs(x, y + i);
        const pixel_t *qstart = bmp->getPixelPtrAbs(srcx, srcy + int(i / scale));

        for (int j = 0; j < scaledw; j++) {
          const pixel_t *q = qstart;
          MOVE_PIXEL_RIGHT(q, int(j / scale));

          if (bmp->getFormat() == BMP_RGB565) {
            RGB_SPLIT(*q, r, g, b);
            drawPixel(p, ARGB_JOIN(0xF, r>>1, g>>2, b>>1));

          } else {  // bmp->getFormat() == BMP_ARGB4444
            drawPixel(p, *q);
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(p);
        }
      }
    } else  {   // format == BM_RGB565

      for (int i = 0; i < scaledh; i++) {
        pixel_t *p = getPixelPtrAbs(x, y + i);
        const pixel_t *qstart = bmp->getPixelPtrAbs(srcx, srcy + int(i / scale));

        for (int j = 0; j < scaledw; j++) {
          const pixel_t *q = qstart;
          MOVE_PIXEL_RIGHT(q, int(j / scale));

          if (bmp->getFormat() == BMP_RGB565) {
            drawPixel(p, *q);
          } else { // bmp->getFormat() == BMP_ARGB4444
            ARGB_SPLIT(*q, a, r, g, b);
            drawAlphaPixel(p, a, RGB_JOIN(r << 1, g << 2, b << 1));
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(p);
        } // for j
      } //for i
    } // if format
  } //  else (scale != 0) {
}

template
void BitmapBuffer::drawBitmapAbs(coord_t, coord_t, const BitmapBuffer *,
                                 coord_t srcx, coord_t srcy,
                                 coord_t srcw, coord_t srch,
                                 float scale);

void BitmapBuffer::drawAlphaPixel(pixel_t *p, uint8_t opacity, uint16_t color)
{
  //TRACE("BitmapBuffer::drawAlphaPixel()");
  if (opacity == OPACITY_MAX) {
    drawPixel(p, color);
  }
  else if (opacity != 0) {
    uint8_t bgWeight = OPACITY_MAX - opacity;
    RGB_SPLIT(color, red, green, blue);
    RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
    uint16_t r = (bgRed * bgWeight + red * opacity) / OPACITY_MAX;
    uint16_t g = (bgGreen * bgWeight + green * opacity) / OPACITY_MAX;
    uint16_t b = (bgBlue * bgWeight + blue * opacity) / OPACITY_MAX;
    drawPixel(p, RGB_JOIN(r, g, b));
  }
}

void BitmapBuffer::drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  APPLY_OFFSET();

  coord_t h = 1;
  if (!applyClippingRect(x, y, w, h))
    return;

  drawHorizontalLineAbs(x, y, w, pat, flags, opacity);
}

void BitmapBuffer::drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  pixel_t * p = getPixelPtrAbs(x, y);
  pixel_t color = COLOR_VAL(flags);

  // Opacity needs to be inverted:
  //   0 : Opaque
  //  15 : Translucid
  //
  opacity = 0x0F - opacity;

  if (pat == SOLID) {
    while (w--) {
      drawAlphaPixel(p, opacity, color);
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
  else {
    while (w--) {
      if (pat & 1) {
        drawAlphaPixel(p, opacity, color);
        pat = (pat >> 1) | 0x80;
      }
      else {
        pat = pat >> 1;
      }
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
}

void BitmapBuffer::drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  APPLY_OFFSET();

  coord_t w = 1;
  if (!applyClippingRect(x, y, w, h))
    return;

  // Opacity needs to be inverted:
  //   0 : Opaque
  //  15 : Translucid
  //
  opacity = 0x0F - opacity;

  pixel_t color = COLOR_VAL(flags);
  if (pat == SOLID) {
    while (h--) {
      drawAlphaPixelAbs(x, y, opacity, color);
      y++;
    }
  }
  else {
    if (pat==DOTTED && !(y%2)) {
      pat = ~pat;
    }
    while (h--) {
      if (pat & 1) {
        drawAlphaPixelAbs(x, y, opacity, color);
        pat = (pat >> 1) | 0x80;
      }
      else {
        pat = pat >> 1;
      }
      y++;
    }
  }
}

//
// Liang-barsky clipping algo
//
static float lb_max(float arr[], int n)
{
  float m = 0;
  for (int i = 0; i < n; ++i)
    if (m < arr[i]) m = arr[i];
  return m;
}

// this function gives the minimum
static float lb_min(float arr[], int n)
{
  float m = 1;
  for (int i = 0; i < n; ++i)
    if (m > arr[i]) m = arr[i];
  return m;
}

bool BitmapBuffer::liangBarskyClipper(coord_t& x1, coord_t& y1, coord_t& x2, coord_t& y2)
{
  // defining variables
  float p1 = -(x2 - x1);
  float p2 = -p1;
  float p3 = -(y2 - y1);
  float p4 = -p3;

  float q1 = x1 - xmin;
  float q2 = xmax - x1;
  float q3 = y1 - ymin;
  float q4 = ymax - y1;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) ||
      (p4 == 0 && q4 < 0)) {
    return false;
  }

  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1;  // for negative p1, add it to negative array
      posarr[posind++] = r2;  // and add p2 to positive array
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }

  float xn1, yn1, xn2, yn2;
  float rn1, rn2;
  rn1 = lb_max(negarr, negind);  // maximum of negative array
  rn2 = lb_min(posarr, posind);  // minimum of positive array

  if (rn1 > rn2) {  // reject
    return false;
  }

  xn1 = x1 + p2 * rn1;
  yn1 = y1 + p4 * rn1;  // computing new points

  xn2 = x1 + p2 * rn2;
  yn2 = y1 + p4 * rn2;

  x1 = coord_t(xn1);
  y1 = coord_t(yn1);
  x2 = coord_t(xn2);
  y2 = coord_t(yn2);

  return true;
}

void BitmapBuffer::drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
                            uint8_t pat, LcdFlags flags)
{
  // Offsets
  x1 += offsetX;
  y1 += offsetY;
  x2 += offsetX;
  y2 += offsetY;

  if (!liangBarskyClipper(x1, y1, x2, y2))
    return;
  
  // No 'opacity' here, only 'color'
  pixel_t color = COLOR_VAL(flags);

  int dx = x2 - x1;      /* the horizontal distance of the line */
  int dy = y2 - y1;      /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs >> 1;
  int y = dxabs >> 1;
  int px = x1;
  int py = y1;

  if (dxabs >= dyabs) {
    /* the line is more horizontal than vertical */
    for (int i = 0; i <= dxabs; i++) {
      if ((1 << (px % 8)) & pat) {
        drawPixelAbs(px, py, color);
      }
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
    }
  }
  else {
    /* the line is more vertical than horizontal */
    for (int i = 0; i <= dyabs; i++) {
      if ((1 << (py % 8)) & pat) {
        drawPixelAbs(px, py, color);
      }
      x += dxabs;
      if (x >= dyabs) {
        x -= dyabs;
        px += sdx;
      }
      py += sdy;
    }
  }
}

void BitmapBuffer::drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness, uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  for (unsigned i = 0; i < thickness; i++) {
    drawVerticalLine(x + i, y, h, pat, flags, opacity);
    drawVerticalLine(x + w - 1 - i, y, h, pat, flags, opacity);
    drawHorizontalLine(x, y + h - 1 - i, w, pat, flags, opacity);
    drawHorizontalLine(x, y + i, w, pat, flags, opacity);
  }
}

void BitmapBuffer::drawSolidFilledRect(coord_t x, coord_t y, coord_t w,
                                       coord_t h, LcdFlags flags)
{
  drawFilledRect(x, y, w, h, SOLID, flags, 0);
}

void BitmapBuffer::drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h,
                                  uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  APPLY_OFFSET();
  if (!applyClippingRect(x, y, w, h)) return;

  if (SOLID != pat) {
    // If we have a pattern, draw line by line
    for (coord_t i = y; i < y + h; i++) {
      drawHorizontalLineAbs(x, i, w, pat, flags, opacity);
    }
    return;
  }

  lv_draw_sw_blend_dsc_t blend_dsc = {0};
  blend_dsc.blend_mode = LV_BLEND_MODE_NORMAL;

  if (opacity == OPACITY_MAX) {
    // we don't draw fully transparent things
    return;
  } else if (!opacity) {
    blend_dsc.opa = LV_OPA_COVER;
  } else {
    blend_dsc.opa = ((OPACITY_MAX - opacity) * LV_OPA_COVER) / OPACITY_MAX;
  }

  auto color = COLOR_VAL(flags);
  blend_dsc.color =
      lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_coord_t lv_x = (lv_coord_t)x;
  lv_coord_t lv_y = (lv_coord_t)y;

  lv_area_t coords = {
      lv_x,
      lv_y,
      lv_x,
      lv_y,
  };

  coords.x2 += w - 1;
  coords.y2 += h - 1;

  if (draw_ctx) {
    lv_area_t clipped_coords;
    if (!_lv_area_intersect(&clipped_coords, &coords, draw_ctx->clip_area))
      return;
    blend_dsc.blend_area = &clipped_coords;
    lv_draw_sw_blend(draw_ctx, &blend_dsc);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);

    rect_dsc.blend_mode = blend_dsc.blend_mode;
    rect_dsc.bg_color = blend_dsc.color;
    rect_dsc.bg_opa = blend_dsc.opa;
    lv_canvas_draw_rect(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        coords.y2 - coords.y1 + 1, &rect_dsc);
  }
#endif
}

void BitmapBuffer::invertRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags)
{
  APPLY_OFFSET();

  if (!applyClippingRect(x, y, w, h))
    return;

  // No 'opacity' here, only 'color'
  pixel_t color = COLOR_VAL(flags);
  RGB_SPLIT(color, red, green, blue);

  for (int i = y; i < y + h; i++) {
    pixel_t * p = getPixelPtrAbs(x, i);
    for (int j = 0; j < w; j++) {
      // TODO ASSERT_IN_DISPLAY(p);
      RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
      drawPixel(p, RGB_JOIN(0x1F + red - bgRed, 0x3F + green - bgGreen, 0x1F + blue - bgBlue));
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
}

/*
  Notice: BitmapBuffer::drawFilledTriangle use the implementation
          made for the Adafruit GFX library, which is licensed under BSD
          license.

  Software License Agreement (BSD License)

  Copyright (c) 2012 Adafruit Industries.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
void BitmapBuffer::drawFilledTriangle(coord_t x0, coord_t y0, coord_t x1,
                                      coord_t y1, coord_t x2, coord_t y2,
                                      LcdFlags flags, uint8_t opacity)
{
  coord_t a, b;

  #define SWAP(a, b) {coord_t tmp = b; b = a; a = tmp;}

  if (y0 > y1) { SWAP(y0, y1); SWAP(x0, x1); }
  if (y1 > y2) { SWAP(y2, y1); SWAP(x2, x1); }
  if (y0 > y1) { SWAP(y0, y1); SWAP(x0, x1); }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    drawHorizontalLine(a, y0, b - a + 1, SOLID, flags, opacity);
    return;
  }

  coord_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
  coord_t sa = 0, sb = 0;
  coord_t last = (y1 == y2) ? y1 : y1 - 1;
  coord_t y;

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    if (a > b) SWAP(a, b);
    drawHorizontalLine(a, y, b - a + 1, SOLID, flags, opacity);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);

  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    if (a > b) SWAP(a, b);
    drawHorizontalLine(a, y, b - a + 1, SOLID, flags, opacity);
  }
}

void BitmapBuffer::drawCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags)
{
  int x1 = radius;
  int y1 = 0;
  int decisionOver2 = 1 - x1;
  pixel_t color = COLOR_VAL(flags);

  while (y1 <= x1) {
    drawPixel(x1 + x, y1 + y, color);
    drawPixel(y1 + x, x1 + y, color);
    drawPixel(-x1 + x, y1 + y, color);
    drawPixel(-y1 + x, x1 + y, color);
    drawPixel(-x1 + x, -y1 + y, color);
    drawPixel(-y1 + x, -x1 + y, color);
    drawPixel(x1 + x, -y1 + y, color);
    drawPixel(y1 + x, -x1 + y, color);
    y1++;
    if (decisionOver2 <= 0) {
      decisionOver2 += 2 * y1 + 1;
    }
    else {
      x1--;
      decisionOver2 += 2 * (y1 - x1) + 1;
    }
  }
}

void BitmapBuffer::drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags)
{
  coord_t imax = ((coord_t)((coord_t)radius * 707)) / 1000 + 1;
  coord_t sqmax = (coord_t)radius * (coord_t)radius + (coord_t)radius / 2;
  coord_t x1 = radius;
  drawSolidHorizontalLine(x - radius, y, radius * 2, flags);
  for (coord_t i = 1; i <= imax; i++) {
    if ((i * i + x1 * x1) > sqmax) {
      // Draw lines from outside
      if (x1 > imax) {
        drawSolidHorizontalLine(x - i + 1, y + x1, (i - 1) * 2, flags);
        drawSolidHorizontalLine(x - i + 1, y - x1, (i - 1) * 2, flags);
      }
      x1--;
    }
    // Draw lines from inside (center)
    drawSolidHorizontalLine(x - x1, y + i, x1 * 2, flags);
    drawSolidHorizontalLine(x - x1, y - i, x1 * 2, flags);
  }
}

class Slope
{
  public:
    explicit Slope(int angle)
    {
      if (angle < 0)
        angle += 360;
      if (angle > 360)
        angle %= 360;
      float radians = float(angle) * (M_PI / 180.0f);
      if (angle == 0) {
        left = false;
        value = 100000;
      }
      else if (angle == 360) {
        left = true;
        value = 100000;
      }
      else if (angle >= 180) {
        left = true;
        value = -(cosf(radians) * 100 / sinf(radians));
      }
      else {
        left = false;
        value = (cosf(radians) * 100 / sinf(radians));
      }
    }

    Slope(bool left, int value):
      left(left),
      value(value)
    {
    }

    bool isBetween(const Slope & start, const Slope & end) const
    {
      if (left) {
        if (start.left) {
          if (end.left)
            return end.value > start.value ? (value <= end.value && value >= start.value) : (value <= end.value || value >= start.value);
          else
            return value >= start.value;
        }
        else {
          if (end.left)
            return value <= end.value;
          else
            return end.value > start.value;
        }
      }
      else {
        if (start.left) {
          if (end.left)
            return start.value > end.value;
          else
            return value >= end.value;
        }
        else {
          if (end.left)
            return value <= start.value;
          else
            return end.value < start.value ? (value >= end.value && value <= start.value) : (value <= start.value || value >= end.value);
        }
      }
    }

    Slope & invertVertical()
    {
      value = -value;
      return *this;
    }

    Slope & invertHorizontal()
    {
      left = !left;
      return *this;
    }

  protected:
    bool left;
    int value;
};

void BitmapBuffer::drawBitmapPatternPie(coord_t x, coord_t y, const uint8_t * img, LcdFlags flags, int startAngle, int endAngle)
{
  if (endAngle == startAngle) {
    endAngle += 1;
  }

  Slope startSlope(startAngle);
  Slope endSlope(endAngle);

  pixel_t color = COLOR_VAL(flags);

  coord_t width = *((uint16_t *)img);
  coord_t height = *(((uint16_t *)img) + 1);
  const uint8_t * q = img + 4;

  int w2 = width / 2;
  int h2 = height / 2;

  for (int y1 = h2 - 1; y1 >= 0; y1--) {
    for (int x1 = w2 - 1; x1 >= 0; x1--) {
      Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
      if (slope.isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 - y1, q[(h2 - y1) * width + w2 + x1]>>4, color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 + y1, q[(h2 + y1) * width + w2 + x1]>>4, color);
      }
      if (slope.invertHorizontal().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 + y1, q[(h2 + y1) * width + w2 - x1]>>4, color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 - y1, q[(h2 - y1) * width + w2 - x1]>>4, color);
      }
    }
  }
}

void BitmapBuffer::drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, int startAngle, int endAngle, LcdFlags flags)
{
  if (endAngle == startAngle) {
    endAngle += 1;
  }

  Slope startSlope(startAngle);
  Slope endSlope(endAngle);

  pixel_t color = COLOR_VAL(flags);
  APPLY_OFFSET();

  coord_t internalDist = internalRadius * internalRadius;
  coord_t externalDist = externalRadius * externalRadius;

  for (int y1 = 0; y1 <= externalRadius; y1++) {
    for (int x1 = 0; x1 <= externalRadius; x1++) {
      auto dist = x1 * x1 + y1 * y1;
      if (dist >= internalDist && dist <= externalDist) {
        Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
        if (slope.isBetween(startSlope, endSlope))
          drawPixelAbs(x + x1, y - y1, color);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixelAbs(x + x1, y + y1, color);
        if (slope.invertHorizontal().isBetween(startSlope, endSlope))
          drawPixelAbs(x - x1, y + y1, color);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixelAbs(x - x1, y - y1, color);
      }
    }
  }
}

void BitmapBuffer::drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, LcdFlags flags, coord_t offsetX, coord_t width)
{
  if (!mask)
    return;

  APPLY_OFFSET();

  coord_t w = mask->width();
  coord_t height = mask->height();

  if (!width || width > w) {
    width = w;
  }

  if (x + width > xmax) {
    width = xmax - x;
  }

  if (x < xmin) {
    width += x - xmin;
    offsetX -= x - xmin;
    x = xmin;
  }

  if (y >= ymax || x >= xmax || width <= 0 || x + width < xmin || y + height < ymin)
    return;

  // TODO: This should be doable the same way as with
  //       drawBitmapPattern() (just with ARGB as input).
  //
  pixel_t color = COLOR_VAL(flags);

  for (coord_t row = 0; row < height; row++) {
    if (y + row < ymin || y + row >= ymax)
      continue;
    pixel_t * p = getPixelPtrAbs(x, y + row);
    const pixel_t * q = mask->getPixelPtrAbs(offsetX, row);
    for (coord_t col = 0; col < width; col++) {
      drawAlphaPixel(p, *((uint8_t *)q), color);
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
      MOVE_TO_NEXT_RIGHT_PIXEL(q);
    }
  }
}

void BitmapBuffer::drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, const BitmapBuffer * srcBitmap, coord_t offsetX, coord_t offsetY, coord_t width, coord_t height)
{
  if (!mask || !srcBitmap)
    return;

  APPLY_OFFSET();

  coord_t maskWidth = mask->width();
  coord_t maskHeight = mask->height();

  if (!width || width > maskWidth) {
    width = maskWidth;
  }

  if (!height || height > maskHeight) {
    height = maskHeight;
  }

  if (x + width > xmax) {
    width = xmax - x;
  }

  if (x < xmin) {
    width += x - xmin;
    offsetX -= x - xmin;
    x = xmin;
  }

  if (y >= ymax || x >= xmax || width <= 0 || x + width < xmin || y + height < ymin)
    return;


  // TODO: This should be doable the same way as with
  //       drawBitmapPattern() (just with ARGB as input).
  //
  for (coord_t row = 0; row < height; row++) {
    if (y + row < ymin || y + row >= ymax)
      continue;
    pixel_t * p = getPixelPtrAbs(x, y + row);
    const pixel_t * q = mask->getPixelPtrAbs(offsetX, offsetY + row);
    for (coord_t col = 0; col < width; col++) {
      drawAlphaPixel(p, *((uint8_t *)q), *srcBitmap->getPixelPtrAbs(row, col));
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
      MOVE_TO_NEXT_RIGHT_PIXEL(q);
    }
  }
}

// Apply a mask ('bmp') + color ('flags') on top of current pixels:
//
//  drawAlphaPixel(bmp[x][y], pixel(x,y), color)
//
//
//
void BitmapBuffer::drawBitmapPattern(coord_t x, coord_t y, const uint8_t * bmp, LcdFlags flags, coord_t offset, coord_t width)
{
  APPLY_OFFSET();

  coord_t bmpw = *((uint16_t *)bmp); // 'w' -> width of the font file
  coord_t bmph = *(((uint16_t *)bmp)+1); // height of the font file

  // skip header
  bmp += 4;

  coord_t srcx = offset;
  coord_t srcy = 0;

  coord_t srcw = (width != 0 ? width : bmpw);
  coord_t srch = bmph;
  if (srcx + srcw > bmpw) srcw = bmpw - srcx;
  if (srcy + srch > bmph) srch = bmph - srcy;

  if (x < xmin) {
    srcw += x - xmin;
    srcx -= x - xmin;
    x = xmin;
  }
  if (y < ymin) {
    srch += y - ymin;
    srcy -= y - ymin;
    y = ymin;
  }
  if (x + srcw > xmax) {
    srcw = xmax - x;
  }
  if (y + srch > ymax) {
    srch = ymax - y;
  }

  if (srcw <= 0 || srch <= 0) {
    return;
  }

  DMACopyAlphaMask(data, _width, _height, x, y, bmp, bmpw, bmph,
                   srcx, srcy, srcw, srch, COLOR_VAL(flags));
}

// TODO: find a better place to define this
extern lv_color_t makeLvColor(uint32_t colorFlags);

coord_t BitmapBuffer::drawSizedText(coord_t x, coord_t y, const char *s,
                                    uint8_t len, LcdFlags flags)
{
  if (!s) return x;
  MOVE_OFFSET();

  int height = getFontHeight(flags);

  if (y + height <= ymin || y >= ymax) {
    RESTORE_OFFSET();
    return x;
  }

  coord_t pos = x;
  const coord_t orig_pos = pos;

  lv_draw_label_dsc_t label_draw_dsc;
  lv_draw_label_dsc_init(&label_draw_dsc);

  const lv_font_t* font = getFont(flags);
  label_draw_dsc.font = font;

  auto color = COLOR_VAL(flags);
  label_draw_dsc.color =
      lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }
  
  lv_point_t p;
  lv_txt_get_size(&p, s, font, label_draw_dsc.letter_space,
                  label_draw_dsc.line_space, LV_COORD_MAX, 0);

  lv_coord_t lv_x = (lv_coord_t)x;
  lv_coord_t lv_y = (lv_coord_t)y;
  
  lv_area_t coords = {
    lv_x, lv_y, lv_x, lv_y,
  };

  coords.x2 += p.x - 1;
  coords.y2 += p.y - 1;

  if (flags & RIGHT) {
    label_draw_dsc.align = LV_TEXT_ALIGN_RIGHT;
  } else if (flags & CENTERED) {
    label_draw_dsc.align = LV_TEXT_ALIGN_CENTER;
    coords.x1 -= p.x / 2;
    coords.x2 -= p.x / 2;
  }

  if (draw_ctx) {
    lv_draw_label(draw_ctx, &label_draw_dsc, &coords, s, nullptr);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_text(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        &label_draw_dsc, s);
  }
#endif
  
  RESTORE_OFFSET();

  pos += p.x;
  return ((flags & RIGHT) ? orig_pos : pos) - offsetX;
}

void BitmapBuffer::formatNumberAsString(char *buffer, uint8_t buffer_size, int32_t val, LcdFlags flags, uint8_t len, const char * prefix, const char * suffix)
{
  if (buffer == nullptr) {
    return;
  }

  char str[48+1]; // max=16 for the prefix, 16 chars for the number, 16 chars for the suffix
  char *s = str + 32;
  *s = '\0';
  int idx = 0;
  int mode = MODE(flags);
  bool neg = false;
  if (val < 0) {
    val = -val;
    neg = true;
  }
  do {
    *--s = '0' + (val % 10);
    ++idx;
    val /= 10;
    if (mode != 0 && idx == mode) {
      mode = 0;
      *--s = '.';
      if (val == 0)
        *--s = '0';
    }
  } while (val != 0 || mode > 0 || (mode == MODE(LEADING0) && idx < len));
  if (neg) *--s = '-';

  // TODO needs check on all string lengths ...
  if (prefix) {
    int len = strlen(prefix);
    if (len <= 16) {
      s -= len;
      strncpy(s, prefix, len);
    }
  }
  if (suffix) {
    strncpy(&str[32], suffix, 16);
  }
  strncpy(buffer, s, buffer_size);
}

coord_t BitmapBuffer::drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len, const char * prefix, const char * suffix)
{
  char s[49];
  formatNumberAsString(s, 49, val, flags, len, prefix, suffix);
  return drawText(x, y, s, flags);
}

void drawSolidRect(BitmapBuffer * dc, coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness, LcdFlags flags)
{
  dc->drawSolidFilledRect(x, y, thickness, h, flags);
  dc->drawSolidFilledRect(x+w-thickness, y, thickness, h, flags);
  dc->drawSolidFilledRect(x, y, w, thickness, flags);
  dc->drawSolidFilledRect(x, y+h-thickness, w, thickness, flags);
}

//void BitmapBuffer::drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle)
//{
//  const uint16_t * q = img;
//  coord_t width = *q++;
//  coord_t height = *q++;
//
//  int slopes[4];
//  if (!evalSlopes(slopes, startAngle, endAngle))
//    return;
//
//  int w2 = width/2;
//  int h2 = height/2;
//
//  for (int y=h2-1; y>=0; y--) {
//    for (int x=w2-1; x>=0; x--) {
//      int slope = (x==0 ? 99000 : y*100/x);
//      if (slope >= slopes[0] && slope < slopes[1]) {
//        *getPixelPtr(x0+w2+x, y0+h2-y) = q[(h2-y)*width + w2+x];
//      }
//      if (-slope >= slopes[0] && -slope < slopes[1]) {
//        *getPixelPtr(x0+w2+x, y0+h2+y) = q[(h2+y)*width + w2+x];
//      }
//      if (slope >= slopes[2] && slope < slopes[3]) {
//        *getPixelPtr(x0+w2-x, y0+h2-y) = q[(h2-y)*width + w2-x];
//      }
//      if (-slope >= slopes[2] && -slope < slopes[3]) {
//        *getPixelPtr(x0+w2-x, y0+h2+y)  = q[(h2+y)*width + w2-x];
//      }
//    }
//  }
//}
//

BitmapBuffer * BitmapBuffer::loadBitmap(const char * filename)
{
  //TRACE("  BitmapBuffer::loadBitmap(%s)", filename);
  const char * ext = getFileExtension(filename);
  if (ext && !strcmp(ext, ".bmp"))
    return load_bmp(filename);
  else
    return load_stb(filename);
}

BitmapBuffer * BitmapBuffer::loadRamBitmap(const uint8_t * buffer, int len)
{
  return load_stb_buffer(buffer, len);
}

BitmapBuffer * BitmapBuffer::loadMask(const char * filename)
{
  BitmapBuffer * bitmap = BitmapBuffer::loadBitmap(filename);
  if (bitmap) {
    pixel_t * p = bitmap->getPixelPtrAbs(0, 0);
    if (bitmap->getFormat() == BMP_ARGB4444) {
      for (int i = bitmap->width() * bitmap->height(); i > 0; i--) {
        // invert red and use as alpha
        ARGB_SPLIT(*p, a __attribute__((unused)), r, g, b);
        
        *((uint8_t *)p) = OPACITY_MAX - (r + g + b) / 3;
        MOVE_TO_NEXT_RIGHT_PIXEL(p);
      }
    } else { // BMP_RGB565
      for (int i = bitmap->width() * bitmap->height(); i > 0; i--) {
        // invert msb 4 bits red and use as alpha
        *((uint8_t *)p) =
            OPACITY_MAX -
            ((GET_RED(*p) >> 1) + (GET_GREEN(*p) >> 2) + (GET_BLUE(*p) >> 1)) /
                3;
        MOVE_TO_NEXT_RIGHT_PIXEL(p);
      }
    }
  }
  return bitmap;
}

BitmapBuffer * BitmapBuffer::load8bitMask(const uint8_t * lbm)
{
  BitmapBuffer * bitmap = new BitmapBuffer(BMP_RGB565,lbm[0],lbm[1]);
  if (bitmap) {
    pixel_t * p = bitmap->getPixelPtrAbs(0, 0);
    const uint8_t * src = lbm + 2;
    for (int i = bitmap->width() * bitmap->height(); i > 0; i--) {
      *((uint8_t *)p) = (*(src++) >> 4);
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
  return bitmap;
}

BitmapBuffer * BitmapBuffer::invertMask() const
{
  BitmapBuffer * result = new BitmapBuffer(format, width(), height());
  pixel_t * srcData = data;
  pixel_t * destData = result->data;
  for (auto y = 0; y < height(); y++) {
    for (auto x = 0; x < width(); x++) {
      destData[x] = OPACITY_MAX - (uint8_t)srcData[x];
    }
    srcData += width();
    destData += width();
  }
  return result;
}

BitmapBuffer * BitmapBuffer::horizontalFlip() const
{
  BitmapBuffer * result = new BitmapBuffer(format, width(), height());
  pixel_t * srcData = data;
  pixel_t * destData = result->data;
  for (uint8_t y = 0; y < height(); y++) {
    for (uint8_t x = 0; x < width(); x++) {
      destData[x] = srcData[width() - 1 - x];
    }
    srcData += width();
    destData += width();
  }
  return result;
}

BitmapBuffer * BitmapBuffer::verticalFlip() const
{
  BitmapBuffer * result = new BitmapBuffer(format, width(), height());
  for (uint8_t y = 0; y < height(); y++) {
    for (uint8_t x = 0; x < width(); x++) {
      result->data[y * width() + x] = data[(height() - 1 - y) * width() + x];
    }
  }
  return result;
}

BitmapBuffer * BitmapBuffer::loadMaskOnBackground(const char * filename, LcdFlags foreground, LcdFlags background)
{
  BitmapBuffer * result = nullptr;
  BitmapBuffer * mask = BitmapBuffer::loadMask(filename);
  if (mask) {
    result = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
    if (result) {
      result->clear(background);
      result->drawMask(0, 0, mask, foreground);
    }
    delete mask;
  }
  return result;
}

BitmapBuffer * BitmapBuffer::load8bitMaskOnBackground(const uint8_t * lbm, LcdFlags foreground, LcdFlags background)
{
  BitmapBuffer * result = nullptr;
  BitmapBuffer * mask = BitmapBuffer::load8bitMask(lbm);
  if (mask) {
    result = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
    if (result) {
      result->clear(background);
      result->drawMask(0, 0, mask, foreground);
    }
    delete mask;
  }
  return result;
}

FIL imgFile __DMA;

BitmapBuffer * BitmapBuffer::load_bmp(const char * filename)
{
  UINT read;
  uint8_t palette[16];
  uint8_t bmpBuf[LCD_W]; /* maximum with LCD_W */
  uint8_t * buf = &bmpBuf[0];

  FRESULT result = f_open(&imgFile, filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return nullptr;
  }

  if (f_size(&imgFile) < 14) {
    f_close(&imgFile);
    return nullptr;
  }

  result = f_read(&imgFile, buf, 14, &read);
  if (result != FR_OK || read != 14) {
    f_close(&imgFile);
    return nullptr;
  }

  if (buf[0] != 'B' || buf[1] != 'M') {
    f_close(&imgFile);
    return nullptr;
  }

  uint32_t fsize  = *((uint32_t *)&buf[2]);
  uint32_t hsize  = *((uint32_t *)&buf[10]); /* header size */

  uint32_t len = limit<uint32_t>(4, hsize - 14, 32);
  result = f_read(&imgFile, buf, len, &read);
  if (result != FR_OK || read != len) {
    f_close(&imgFile);
    return nullptr;
  }

  uint32_t ihsize = *((uint32_t *)&buf[0]); /* extra header size */

  /* invalid extra header size */
  if (ihsize + 14 > hsize) {
    f_close(&imgFile);
    return nullptr;
  }

  /* sometimes file size is set to some headers size, set a real size in that case */
  if (fsize == 14 || fsize == ihsize + 14)
    fsize = f_size(&imgFile) - 2;

  /* declared file size less than header size */
  if (fsize <= hsize) {
    f_close(&imgFile);
    return nullptr;
  }

  uint32_t w, h;

  switch (ihsize) {
    case  40: // windib
    case  56: // windib v3
    case  64: // OS/2 v2
    case 108: // windib v4
    case 124: // windib v5
      w  = *((uint32_t *)&buf[4]);
      h = *((uint32_t *)&buf[8]);
      buf += 12;
      break;
    case  12: // OS/2 v1
      w  = *((uint16_t *)&buf[4]);
      h = *((uint16_t *)&buf[6]);
      buf += 8;
      break;
    default:
      f_close(&imgFile);
      return nullptr;
  }
  //TRACE("  BitmapBuffer::load_bmp() %dx%d", w, h);
  if (*((uint16_t *)&buf[0]) != 1) { /* planes */
    f_close(&imgFile);
    return nullptr;
  }

  uint16_t depth = *((uint16_t *)&buf[2]);

  buf = &bmpBuf[0];

  if (depth == 4) {
    if (f_lseek(&imgFile, hsize - 64) != FR_OK || f_read(&imgFile, buf, 64, &read) != FR_OK || read != 64) {
      f_close(&imgFile);
      return nullptr;
    }
    for (uint8_t i = 0; i < 16; i++) {
      palette[i] = buf[4*i];
    }
  }
  else {
    if (f_lseek(&imgFile, hsize) != FR_OK) {
      f_close(&imgFile);
      return nullptr;
    }
  }

  BitmapBuffer * bmp = new BitmapBuffer(BMP_RGB565, w, h);
  if (bmp == nullptr || bmp->getData() == nullptr) {
    f_close(&imgFile);
    return nullptr;
  }

  uint32_t rowSize;
  bool hasAlpha = false;

  switch (depth) {
    case 16:
      for (int i = h - 1; i >= 0; i--) {
        pixel_t * dst = bmp->getPixelPtrAbs(0, i);
        for (unsigned int j = 0; j < w; j++) {
          result = f_read(&imgFile, (uint8_t *)dst, 2, &read);
          if (result != FR_OK || read != 2) {
            f_close(&imgFile);
            delete bmp;
            return nullptr;
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(dst);
        }
      }
      break;

    case 32:
      for (int i = h - 1; i >= 0; i--) {
        pixel_t * dst = bmp->getPixelPtrAbs(0, i);
        for (unsigned int j = 0; j < w; j++) {
          uint32_t pixel;
          result = f_read(&imgFile, (uint8_t *)&pixel, 4, &read);
          if (result != FR_OK || read != 4) {
            f_close(&imgFile);
            delete bmp;
            return nullptr;
          }
          if (hasAlpha) {
            *dst = ARGB(pixel & 0xff, (pixel >> 24) & 0xff, (pixel >> 16) & 0xff, (pixel >> 8) & 0xff);
          }
          else {
            if ((pixel & 0xff) == 0xff) {
              *dst = RGB(pixel >> 24, (pixel >> 16) & 0xff, (pixel >> 8) & 0xff);
            }
            else {
              hasAlpha = true;
              bmp->setFormat(BMP_ARGB4444);
              for (pixel_t * p = bmp->getPixelPtrAbs(j, i); p != bmp->getPixelPtrAbs(0, h); MOVE_TO_NEXT_RIGHT_PIXEL(p)) {
                pixel_t tmp = *p;
                *p = ((tmp >> 1) & 0x0f) + (((tmp >> 7) & 0x0f) << 4) + (((tmp >> 12) & 0x0f) << 8);
              }
              *dst = ARGB(pixel & 0xff, (pixel >> 24) & 0xff, (pixel >> 16) & 0xff, (pixel >> 8) & 0xff);
            }
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(dst);
        }
      }
      break;

    case 1:
      break;

    case 4:
      rowSize = ((4*w+31)/32)*4;
      for (int32_t i=h-1; i>=0; i--) {
        result = f_read(&imgFile, buf, rowSize, &read);
        if (result != FR_OK || read != rowSize) {
          f_close(&imgFile);
          delete bmp;
          return nullptr;
        }
        pixel_t * dst = bmp->getPixelPtrAbs(0, i);
        for (uint32_t j=0; j<w; j++) {
          uint8_t index = (buf[j/2] >> ((j & 1) ? 0 : 4)) & 0x0F;
          uint8_t val = palette[index];
          *dst = RGB(val, val, val);
          MOVE_TO_NEXT_RIGHT_PIXEL(dst);
        }
      }
      break;

    default:
      f_close(&imgFile);
      delete bmp;
      return nullptr;
  }

  f_close(&imgFile);
  return bmp;
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR

// #define TRACE_STB_MALLOC

#if defined(TRACE_STB_MALLOC)
#define STBI_MALLOC(sz)                     stb_malloc(sz)
#define STBI_REALLOC_SIZED(p,oldsz,newsz)   stb_realloc(p,oldsz,newsz)
#define STBI_FREE(p)                        stb_free(p)

void * stb_malloc(unsigned int size)
{
  void * res = malloc(size);
  TRACE("malloc %d = %p", size, res);
  return res;
}

void stb_free(void *ptr)
{
  TRACE("free %p", ptr);
  free(ptr);
}

void *stb_realloc(void *ptr, unsigned int oldsz, unsigned int newsz)
{
  void * res =  realloc(ptr, newsz);
  TRACE("realloc %p, %d -> %d = %p", ptr, oldsz, newsz, res);
  return res;
}
#endif // #if defined(TRACE_STB_MALLOC)

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#undef __I
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_BMP
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// fill 'data' with 'size' bytes.  return number of bytes actually read
int stbc_read(void *user, char * data, int size)
{
  FIL * fp = (FIL *)user;
  UINT br = 0;
  FRESULT res = f_read(fp, data, size, &br);
  if (res == FR_OK) {
    return (int)br;
  }
  return 0;
}

// skip the next 'n' bytes, or 'unget' the last -n bytes if negative
void stbc_skip(void *user, int n)
{
  FIL * fp = (FIL *)user;
  f_lseek(fp, f_tell(fp) + n);
}

// returns nonzero if we are at end of file/data
int stbc_eof(void *user)
{
  FIL * fp = (FIL *)user;
  int res = f_eof(fp);
  return res;
}

// callbacks for stb-image
const stbi_io_callbacks stbCallbacks = {
  stbc_read,
  stbc_skip,
  stbc_eof
};

BitmapBuffer * BitmapBuffer::load_stb(const char * filename)
{
  //TRACE("  BitmapBuffer::load_stb(%s)", filename);

  FRESULT result = f_open(&imgFile, filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return nullptr;
  }

  int x, y, nn;
  stbi_info_from_callbacks(&stbCallbacks, &imgFile, &x, &y, &nn);
  f_close(&imgFile);
  //TRACE("  BitmapBuffer::load_stb()----Info File %s, %d, %d, %d", filename, x, y, nn);

  result = f_open(&imgFile, filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return nullptr;
  }

  int w, h, n;
  unsigned char * img = stbi_load_from_callbacks(&stbCallbacks, &imgFile, &w, &h, &n, 4);
  f_close(&imgFile);

  if (!img) {
    TRACE("load_stb(%s) failed: %s", filename, stbi_failure_reason());
    return nullptr;
  }

  //TRACE("  BitmapBuffer::load_stb()----Info File %s, %d, %d, %d/%d", filename, x, y, nn, n);
  BitmapBuffer * bmp = convert_stb_bitmap(img, w, h, n);
  stbi_image_free(img);
  return bmp;
}

BitmapBuffer * BitmapBuffer::load_stb_buffer(const uint8_t * buffer, int len)
{
  int w, h, n;
  unsigned char * img = stbi_load_from_memory(buffer, len, &w, &h, &n, 4);
  if (!img) {
    TRACE("load_stb_buffer(%p,%d) failed: %s", buffer, len, stbi_failure_reason());
    return nullptr;
  }

  BitmapBuffer * bmp = convert_stb_bitmap(img, w, h, n);
  stbi_image_free(img);
  return bmp;
}

BitmapBuffer * BitmapBuffer::convert_stb_bitmap(uint8_t * img, int w, int h, int n)
{
  // convert to RGB565 or ARGB4444 format
  //TRACE("  BitmapBuffer::convert_stb_bitmap(%d)", n);
  BitmapBuffer * bmp = new BitmapBuffer(n == 4 ? BMP_ARGB4444 : BMP_RGB565, w, h);
  if (bmp == nullptr) {
    TRACE("convert_stn_bitmap: malloc failed");
    return nullptr;
  }

#if 0 // use Stb's "stbi__vertically_flip_on_load" instead of this?
  DMABitmapConvert(bmp->data, img, w, h, n == 4 ? DMA2D_ARGB4444 : DMA2D_RGB565);
#else
  pixel_t * dest = bmp->getPixelPtrAbs(0, 0);
  const uint8_t * p = img;
  if (n == 4) {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = ARGB(p[3], p[0], p[1], p[2]);
        MOVE_TO_NEXT_RIGHT_PIXEL(dest);
        p += 4;
      }
    }
  }
  else { // assume 3 bytes, packed in groups of 4, I guess
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = RGB(p[0], p[1], p[2]);
        MOVE_TO_NEXT_RIGHT_PIXEL(dest);
        p += 4;
      }
    }
  }
#endif

  return bmp;
}

uint8_t * BitmapBuffer::loadFont(const uint8_t * lbm, int len, int& w, int& h)
{
  int n;
  unsigned char * font = stbi_load_from_memory(lbm, len, &w, &h, &n, 1);
  if (!font) {
    TRACE("load_stb_buffer(%p,%d) failed: %s", font, len, stbi_failure_reason());
    return nullptr;
  }

  return font;
}
