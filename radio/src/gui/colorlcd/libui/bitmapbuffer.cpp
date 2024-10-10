/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "bitmapbuffer.h"

#include <math.h>

#include <string>

#include "bitmaps.h"
#include "board.h"
#include "dma2d.h"
#include "fonts.h"
#include "lvgl/src/draw/sw/lv_draw_sw.h"
#include "edgetx_helpers.h"
#include "strhelpers.h"

BitmapBuffer::BitmapBuffer(uint8_t format, uint16_t width, uint16_t height) :
    format(format),
    _width(width),
    _height(height),
    xmax(width),
    ymax(height),
    dataAllocated(true)
#if defined(DEBUG)
    ,
    leakReported(false)
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
    format(format),
    _width(width),
    _height(height),
    xmax(width),
    ymax(height),
    data(data),
    data_end(data + (width * height)),
    dataAllocated(false)
#if defined(DEBUG)
    ,
    leakReported(false)
#endif
{
}

BitmapBuffer::~BitmapBuffer()
{
  DMAWait();
  if (dataAllocated) {
#if !defined(BOOT)
    lv_obj_del(canvas);
#endif
    free(data);
  }
}

void BitmapBuffer::setData(uint16_t *d)
{
  if (!dataAllocated) {
    data = d;
    data_end = d + (_width * _height);
  }
}

void BitmapBuffer::clear(LcdFlags flags)
{
  drawSolidFilledRect(0, 0, _width - offsetX, _height - offsetY, flags);
}

void BitmapBuffer::clearClippingRect()
{
  xmin = 0;
  xmax = _width;
  ymin = 0;
  ymax = _height;
}

void BitmapBuffer::setClippingRect(coord_t xmin, coord_t xmax, coord_t ymin,
                                   coord_t ymax)
{
  this->xmin = xmin;
  this->xmax = xmax;
  this->ymin = ymin;
  this->ymax = ymax;
}

void BitmapBuffer::getClippingRect(coord_t &xmin, coord_t &xmax, coord_t &ymin,
                                   coord_t &ymax)
{
  xmin = this->xmin;
  xmax = this->xmax;
  ymin = this->ymin;
  ymax = this->ymax;
}

bool BitmapBuffer::applyClippingRect(coord_t& x, coord_t& y, coord_t& w,
                                     coord_t& h) const
{
  if (h < 0) {
    y += h;
    h = -h;
  }

  if (w < 0) {
    x += w;
    w = -w;
  }

  if (x >= xmax || y >= ymax) return false;

  if (y < ymin) {
    h += y - ymin;
    y = ymin;
  }

  if (x < xmin) {
    w += x - xmin;
    x = xmin;
  }

  if (y + h > ymax) h = ymax - y;

  if (x + w > xmax) w = xmax - x;

  return data && h > 0 && w > 0;
}

void BitmapBuffer::setOffset(coord_t offsetX, coord_t offsetY)
{
  this->offsetX = offsetX;
  this->offsetY = offsetY;
}

void BitmapBuffer::drawBitmap(coord_t x, coord_t y, const BitmapBuffer *bmp,
                              coord_t srcx, coord_t srcy, coord_t srcw,
                              coord_t srch, float scale)
{
  if (!data || !bmp) return;
  APPLY_OFFSET();
  if (x >= xmax || y >= ymax) return;

  coord_t bmpw = bmp->width();
  coord_t bmph = bmp->height();

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
    if (bmp->format == BMP_ARGB4444) {
      DMACopyAlphaBitmap(data, _width, _height, x, y, bmp->getData(), bmpw,
                         bmph, srcx, srcy, srcw, srch);
    } else {
      DMACopyBitmap(data, _width, _height, x, y, bmp->getData(), bmpw, bmph,
                    srcx, srcy, srcw, srch);
    }
    DMAWait();

#if __CORTEX_M >= 0x07
    SCB_CleanInvalidateDCache();
#endif
  } else {
    int scaledw = srcw * scale;
    int scaledh = srch * scale;

    if (x + scaledw > _width) scaledw = _width - x;
    if (y + scaledh > _height) scaledh = _height - y;

    if (format == BMP_ARGB4444) {
      for (int i = 0; i < scaledh; i++) {
        pixel_t *p = getPixelPtrAbs(x, y + i);
        const pixel_t *qstart =
            bmp->getPixelPtrAbs(srcx, srcy + int(i / scale));

        for (int j = 0; j < scaledw; j++) {
          const pixel_t *q = qstart;
          MOVE_PIXEL_RIGHT(q, int(j / scale));

          if (bmp->format == BMP_RGB565) {
            RGB_SPLIT(*q, r, g, b);
            drawPixel(p, ARGB_JOIN(0xF, r >> 1, g >> 2, b >> 1));

          } else {  // bmp->format == BMP_ARGB4444
            drawPixel(p, *q);
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(p);
        }
      }
    } else {  // format == BM_RGB565

      for (int i = 0; i < scaledh; i++) {
        pixel_t *p = getPixelPtrAbs(x, y + i);
        const pixel_t *qstart =
            bmp->getPixelPtrAbs(srcx, srcy + int(i / scale));

        for (int j = 0; j < scaledw; j++) {
          const pixel_t *q = qstart;
          MOVE_PIXEL_RIGHT(q, int(j / scale));

          if (bmp->format == BMP_RGB565) {
            drawPixel(p, *q);
          } else {  // bmp->format == BMP_ARGB4444
            ARGB_SPLIT(*q, a, r, g, b);
            drawAlphaPixel(p, a, RGB_JOIN(r << 1, g << 2, b << 1));
          }
          MOVE_TO_NEXT_RIGHT_PIXEL(p);
        }  // for j
      }    // for i
    }      // if format
  }        //  else (scale != 0) {
}

void BitmapBuffer::drawAlphaPixel(pixel_t *p, uint8_t opacity, uint16_t color)
{
  // TRACE("BitmapBuffer::drawAlphaPixel()");
  if (opacity == OPACITY_MAX) {
    drawPixel(p, color);
  } else if (opacity != 0) {
    uint8_t bgWeight = OPACITY_MAX - opacity;
    RGB_SPLIT(color, red, green, blue);
    RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
    uint16_t r = (bgRed * bgWeight + red * opacity) / OPACITY_MAX;
    uint16_t g = (bgGreen * bgWeight + green * opacity) / OPACITY_MAX;
    uint16_t b = (bgBlue * bgWeight + blue * opacity) / OPACITY_MAX;
    drawPixel(p, RGB_JOIN(r, g, b));
  }
}

void BitmapBuffer::drawHorizontalLine(coord_t x, coord_t y, coord_t w,
                                      uint8_t pat, LcdFlags flags,
                                      uint8_t opacity)
{
  if (opacity == OPACITY_MAX) return;

  APPLY_OFFSET();

  coord_t h = 1;
  if (!applyClippingRect(x, y, w, h)) return;

  drawHorizontalLineAbs(x, y, w, pat, flags, opacity);
}

void BitmapBuffer::drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w,
                                         uint8_t pat, LcdFlags flags,
                                         uint8_t opacity)
{
  if (opacity == OPACITY_MAX) return;

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_draw_line_dsc_t line_dsc;
  lv_draw_line_dsc_init(&line_dsc);

  line_dsc.width = 1;
  line_dsc.opa = ((OPACITY_MAX - opacity) * LV_OPA_COVER) / OPACITY_MAX;
  line_dsc.color = makeLvColor(flags);

  if (pat == DOTTED) {
    line_dsc.dash_gap = 1;
    line_dsc.dash_width = 1;
  }

  lv_point_t p[2];
  p[0].x = x;
  p[0].y = y;
  p[1].x = x + w;
  p[1].y = y;

  if (draw_ctx) {
    lv_draw_line(draw_ctx, &line_dsc, &p[0], &p[1]);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_line(canvas, p, 2, &line_dsc);
  }
#endif
}

void BitmapBuffer::drawVerticalLine(coord_t x, coord_t y, coord_t h,
                                    uint8_t pat, LcdFlags flags,
                                    uint8_t opacity)
{
  if (opacity == OPACITY_MAX) return;

  APPLY_OFFSET();

  coord_t w = 1;
  if (!applyClippingRect(x, y, w, h)) return;

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_draw_line_dsc_t line_dsc;
  lv_draw_line_dsc_init(&line_dsc);

  line_dsc.width = 1;
  line_dsc.opa = ((OPACITY_MAX - opacity) * LV_OPA_COVER) / OPACITY_MAX;
  line_dsc.color = makeLvColor(flags);

  if (pat == DOTTED) {
    line_dsc.dash_gap = 1;
    line_dsc.dash_width = 1;
  }

  lv_point_t p[2];
  p[0].x = x;
  p[0].y = y;
  p[1].x = x;
  p[1].y = y + h;

  if (draw_ctx) {
    lv_draw_line(draw_ctx, &line_dsc, &p[0], &p[1]);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_line(canvas, p, 2, &line_dsc);
  }
#endif
}

void BitmapBuffer::drawRect(coord_t x, coord_t y, coord_t w, coord_t h,
                            uint8_t thickness, uint8_t pat, LcdFlags flags,
                            uint8_t opacity)
{
  drawFilledRect(x, y, thickness, h, pat, flags, opacity);
  drawFilledRect(x + w - thickness, y, thickness, h, pat, flags, opacity);
  drawFilledRect(x, y, w, thickness, pat, flags, opacity);
  drawFilledRect(x, y + h - thickness, w, thickness, pat, flags, opacity);
}

void BitmapBuffer::drawSolidRect(coord_t x, coord_t y, coord_t w, coord_t h,
                                 uint8_t thickness, LcdFlags flags)
{
  drawRect(x, y, w, h, thickness, SOLID, flags, 0);
}

void BitmapBuffer::drawSolidFilledRect(coord_t x, coord_t y, coord_t w,
                                       coord_t h, LcdFlags flags)
{
  drawFilledRect(x, y, w, h, SOLID, flags, 0);
}

void BitmapBuffer::drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h,
                                  uint8_t pat, LcdFlags flags, uint8_t opacity)
{
  if (opacity == OPACITY_MAX) return;

  APPLY_OFFSET();
  if (!applyClippingRect(x, y, w, h)) return;

  if (SOLID != pat) {
    // If we have a pattern, draw line by line
    for (coord_t i = y; i < y + h; i++) {
      drawHorizontalLineAbs(x, i, w, pat, flags, opacity);
    }
    return;
  }

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = ((OPACITY_MAX - opacity) * LV_OPA_COVER) / OPACITY_MAX;
  rect_dsc.bg_color = makeLvColor(flags);

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_area_t coords = {
      (lv_coord_t)x,
      (lv_coord_t)y,
      (lv_coord_t)(x + w - 1),
      (lv_coord_t)(y + h - 1),
  };

  if (draw_ctx) {
    lv_draw_rect(draw_ctx, &rect_dsc, &coords);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_rect(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        coords.y2 - coords.y1 + 1, &rect_dsc);
  }
#endif
}

coord_t BitmapBuffer::drawSizedText(coord_t x, coord_t y, const char *s,
                                    uint8_t len, LcdFlags flags)
{
  if (!s) return x;
  MOVE_OFFSET();

  // LVGL does not handle non-null terminated strings
  static char buffer[256];
  strncpy(buffer, s, len);
  buffer[len] = '\0';

  coord_t pos = x;
  const coord_t orig_pos = pos;

  lv_draw_label_dsc_t label_draw_dsc;
  lv_draw_label_dsc_init(&label_draw_dsc);

  const lv_font_t *font = getFont(flags);
  label_draw_dsc.font = font;

  auto color = COLOR_VAL(flags);
  label_draw_dsc.color =
      lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_point_t p;
  lv_txt_get_size(&p, buffer, font, label_draw_dsc.letter_space,
                  label_draw_dsc.line_space, LV_COORD_MAX, 0);

  lv_coord_t lv_x = (lv_coord_t)x;
  lv_coord_t lv_y = (lv_coord_t)y;

  lv_area_t coords = {
      lv_x,
      lv_y,
      lv_x,
      lv_y,
  };

  coords.x2 += p.x - 1;
  coords.y2 += p.y - 1;

  if (flags & RIGHT) {
    label_draw_dsc.align = LV_TEXT_ALIGN_RIGHT;
    coords.x1 -= p.x;
    coords.x2 -= p.x;
  } else if (flags & CENTERED) {
    label_draw_dsc.align = LV_TEXT_ALIGN_CENTER;
    coords.x1 -= p.x / 2;
    coords.x2 -= p.x / 2;
  }

  if (draw_ctx) {
    lv_draw_label(draw_ctx, &label_draw_dsc, &coords, buffer, nullptr);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_text(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        &label_draw_dsc, buffer);
  }
#endif

  RESTORE_OFFSET();

  pos += p.x;

  return ((flags & RIGHT) ? orig_pos : pos) - offsetX;
}

// Resize and convert to LVGL image data format with alpha blending to
// background color
void BitmapBuffer::resizeToLVGL(coord_t w, coord_t h)
{
  // Scale values from ARGB4444 to RGB565 with alpha
  static uint8_t rbcnv[16] = {0,  2,  4,  6,  8,  10, 12, 14,
                              17, 19, 21, 23, 25, 27, 29, 31};
  static uint8_t gcnv[16] = {0,  4,  8,  13, 17, 21, 25, 29,
                             34, 38, 42, 46, 50, 55, 59, 63};
  static uint8_t alpha[16] = {0, 17, 34, 51, 68, 85, 102, 119,
                              136, 153, 170, 187, 204, 221, 238, 255};

  float vscale = float(h) / (float)height();
  float hscale = float(w) / (float)width();
  float scale;
  coord_t scaledw, scaledh;
  if (vscale < hscale) {
    scale = vscale;
    scaledw = width() * scale;
    scaledh = h;
  } else {
    scale = hscale;
    scaledw = w;
    scaledh = height() * scale;
  }

  uint8_t *ndata =
      (uint8_t *)malloc(align32(scaledw * scaledh * 3));

  if (ndata) {
    uint8_t *dst = ndata;
    for (int i = 0; i < scaledh; i += 1) {
      pixel_t *src = &data[(coord_t)(i / scale) * width()];
      for (int j = 0; j < scaledw; j += 1) {
        ARGB_SPLIT(src[(coord_t)(j / scale)], a, r, g, b);
        auto c = RGB_JOIN(rbcnv[r], gcnv[g], rbcnv[b]);
        *dst++ = c & 0xFF;
        *dst++ = c >> 8;
        *dst++ = alpha[a];
      }
    }

    delete data;
    data = (pixel_t*)ndata;
    _width = scaledw;
    _height = scaledh;
    data_end = data + ((scaledw * scaledh * 3 + 1) / 2);
  }
}
