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

#ifndef _BITMAP_BUFFER_H_
#define _BITMAP_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libopenui_types.h"
#include "libopenui_defines.h"
#include "libopenui_depends.h"
#include "debug.h"

constexpr uint8_t SOLID = 0xFF;
constexpr uint8_t DOTTED  = 0x55;
constexpr uint8_t STASHED = 0x33;

#define MOVE_OFFSET() coord_t offsetX = this->offsetX; x += offsetX; this->offsetX = 0; coord_t offsetY = this->offsetY; y += offsetY; this->offsetY = 0
#define APPLY_OFFSET() x += offsetX; y += offsetY
#define RESTORE_OFFSET()  this->offsetX = offsetX, this->offsetY = offsetY

#if defined(PCBX10) && !defined(SIMU)
  #define MOVE_PIXEL_RIGHT(p, count)   p -= count
#else
  #define MOVE_PIXEL_RIGHT(p, count)   p += count
#endif

#define MOVE_TO_NEXT_RIGHT_PIXEL(p)    MOVE_PIXEL_RIGHT(p, 1)

#define USE_STB

enum BitmapFormats
{
  BMP_RGB565,
  BMP_ARGB4444
};

template<class T>
class BitmapBufferBase
{
  public:
    BitmapBufferBase(uint8_t format, uint16_t width, uint16_t height, T * data):
      format(format),
      width(width),
      height(height),
      xmax(width),
      ymax(height),
      data(data),
      data_end(data + (width * height))
    {
    }

    BitmapBufferBase(uint8_t format, T * data):
      format(format),
      width(*((uint16_t*)data)),
      height(*(((uint16_t*)data) + 1)),
      xmax(width),
      ymax(height),
      data(data + 2),
      data_end(data + 2 + (width * height))
    {
    }

    inline void clearClippingRect()
    {
      xmin = 0;
      xmax = width;
      ymin = 0;
      ymax = height;
    }

    inline void setClippingRect(coord_t xmin, coord_t xmax, coord_t ymin, coord_t ymax)
    {
      this->xmin = xmin;
      this->xmax = xmax;
      this->ymin = ymin;
      this->ymax = ymax;
    }

    inline void getClippingRect(coord_t & xmin, coord_t & xmax, coord_t & ymin, coord_t & ymax)
    {
      xmin = this->xmin;
      xmax = this->xmax;
      ymin = this->ymin;
      ymax = this->ymax;
    }

    inline void setOffset(coord_t offsetX, coord_t offsetY)
    {
      this->offsetX = offsetX;
      this->offsetY = offsetY;
    }

    coord_t getOffsetX() const
    {
      return offsetX;
    }

    coord_t getOffsetY() const
    {
      return offsetY;
    }

    inline uint8_t getFormat() const
    {
      return format;
    }

    inline uint16_t getWidth() const
    {
      return width;
    }

    inline uint16_t getHeight() const
    {
      return height;
    }

    inline T * getData() const
    {
      return data;
    }

    uint32_t getDataSize() const
    {
      return width * height * sizeof(T);
    }

    inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
#if defined(PCBX10) && !defined(SIMU)
      x = width - x - 1;
      y = height - y - 1;
#endif
      return &data[y*width + x];
    }

  protected:
    uint8_t format;
    coord_t width;
    coord_t height;
    coord_t xmin = 0;
    coord_t xmax;
    coord_t ymin = 0;
    coord_t ymax;
    coord_t offsetX = 0;
    coord_t offsetY = 0;
    T * data;
    T * data_end;
};

typedef BitmapBufferBase<const uint16_t> Bitmap;

class RLEBitmap:
  public BitmapBufferBase<uint16_t>
{
  public:
    RLEBitmap(uint8_t format, const uint8_t* rle_data) :
      BitmapBufferBase<uint16_t>(format, 0, 0, nullptr)
    {
      width = *((uint16_t *)rle_data);
      height = *(((uint16_t *)rle_data)+1);
      uint32_t pixels = width * height;
      data = (uint16_t*)malloc(pixels * sizeof(uint16_t));
      decode((uint8_t *)data, pixels * sizeof(uint16_t), rle_data+4);
      data_end = data + pixels;
    }

    ~RLEBitmap()
    {
      free(data);
    }

    static int decode(uint8_t * dest, unsigned int dest_size, const unsigned char* src)
    {
      //unsigned char count = 0;
      unsigned char prev_byte = 0;
      bool prev_byte_valid = false;

      const unsigned char * dest_end = dest + dest_size;
      unsigned char * d = dest;

      while (d < dest_end) {
        *d++ = *src;
        if (prev_byte_valid && (*src == prev_byte)) {
          src++;

          if (d + *src > dest + dest_size) {
//            TRACE("rle_decode_8bit: destination overflow!\n");
            return -1;
          }

          memset(d, prev_byte, *src);
          d += *src++;
          prev_byte_valid = false;
        }
        else {
          prev_byte = *src++;
          prev_byte_valid = true;
        }
      }

      return d - dest;
    }
};

class BitmapBuffer: public BitmapBufferBase<uint16_t>
{
  private:
    bool dataAllocated;
#if defined(DEBUG)
    bool leakReported;
#endif

  public:

    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height):
      BitmapBufferBase<uint16_t>(format, width, height, nullptr),
      dataAllocated(true)
#if defined(DEBUG)
      , leakReported(false)
#endif
    {
      data = (uint16_t *)malloc(width*height*sizeof(uint16_t));
      data_end = data + (width * height);
    }

    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height, uint16_t * data):
      BitmapBufferBase<uint16_t>(format, width, height, data),
      dataAllocated(false)
#if defined(DEBUG)
      , leakReported(false)
#endif
    {
    }

    ~BitmapBuffer()
    {
      if (dataAllocated) {
        free(data);
      }
    }

    inline void setFormat(uint8_t format)
    {
      this->format = format;
    }

    inline void clear(LcdFlags flags=0)
    {
      drawSolidFilledRect(0, 0, width - offsetX, height - offsetY, flags);
    }

    inline void drawPixel(pixel_t * p, pixel_t value)
    {
      if (data && (data <= p || p < data_end)) {
        *p = value;
      }
#if defined(DEBUG)
      else if (!leakReported) {
        leakReported = true;
        TRACE("BitmapBuffer(%p).drawPixel(): buffer overrun, data: %p, written at: %p", this, data, p);
      }
#endif
    }

    inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
#if defined(PCBX10) && !defined(SIMU)
      x = width - x - 1;
      y = height - y - 1;
#endif
      return &data[y*width + x];
    }

    inline pixel_t * getPixelPtr(coord_t x, coord_t y)
    {
#if defined(PCBX10) && !defined(SIMU)
      x = width - x - 1;
      y = height - y - 1;
#endif
      return &data[y*width + x];
    }

    inline void drawPixel(coord_t x, coord_t y, pixel_t value)
    {
      pixel_t * p = getPixelPtr(x, y);
      drawPixel(p, value);
    }

    void drawAlphaPixel(pixel_t * p, uint8_t opacity, uint16_t color);

    inline void drawAlphaPixel(coord_t x, coord_t y, uint8_t opacity, uint16_t color)
    {
      pixel_t * p = getPixelPtr(x, y);
      drawAlphaPixel(p, opacity, color);
    }

    void drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat = SOLID, LcdFlags flags = 0);

    void drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat = SOLID, LcdFlags flags = 0);

    inline void drawSolidHorizontalLine(coord_t x, coord_t y, coord_t w, LcdFlags flags)
    {
      drawSolidFilledRect(x, y, w, 1, flags);
    }

    inline void drawSolidVerticalLine(coord_t x, coord_t y, coord_t h, LcdFlags flags)
    {
      drawSolidFilledRect(x, y, 1, h, flags);
    }

    void drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness = 1, uint8_t pat = SOLID, LcdFlags flags = 0);

    inline void drawSolidRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness = 1, LcdFlags flags = 0)
    {
      drawSolidFilledRect(x, y, thickness, h, flags);
      drawSolidFilledRect(x+w-thickness, y, thickness, h, flags);
      drawSolidFilledRect(x, y, w, thickness, flags);
      drawSolidFilledRect(x, y+h-thickness, w, thickness, flags);
    }

    void drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags = 0);

    void drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat = SOLID, LcdFlags flags = 0);

    void invertRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags = 0);

    void drawCircle(int x0, int y0, int radius);

    void drawPie(int x0, int y0, int radius, int startAngle, int endAngle);

    void drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle);

    void drawBitmapPatternPie(coord_t x0, coord_t y0, const uint8_t * img, LcdFlags flags, int startAngle, int endAngle);

    static BitmapBuffer * load(const char * filename);

    static BitmapBuffer * loadMask(const char * filename);

    static BitmapBuffer * loadMaskOnBackground(const char * filename, LcdFlags foreground, LcdFlags background);

    void drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, LcdFlags flags, coord_t offset=0, coord_t width=0);

    void drawBitmapPattern(coord_t x, coord_t y, const uint8_t * bmp, LcdFlags flags, coord_t offset=0, coord_t width=0);

    coord_t drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags=0);

    coord_t drawText(coord_t x, coord_t y, const char * s, LcdFlags flags = 0)
    {
      return drawSizedText(x, y, s, 255, flags);
    }

    coord_t drawTextAtIndex(coord_t x, coord_t y, const char * s, uint8_t idx, LcdFlags flags = 0)
    {
      char length = *s++;
      return drawSizedText(x, y, s+length*idx, length, flags);
    }

    coord_t drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr);

    template<class T>
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx=0, coord_t srcy=0, coord_t w=0, coord_t h=0, float scale=0)
    {
      APPLY_OFFSET();

      if (x >= xmax) return;
      if (y >= ymax) return;

      if (!data || !bmp)
        return;

      coord_t srcw = bmp->getWidth();
      coord_t srch = bmp->getHeight();

      if (w == 0)
        w = srcw;
      if (h == 0)
        h = srch;
      if (srcx + w > srcw)
        w = srcw - srcx;
      if (srcy + h > srch)
        h = srch - srcy;

      if (x < xmin) {
        w += x-xmin;
        srcx -= x-xmin;
        x = xmin;
      }

      if (y < ymin) {
        h += y-ymin;
        srcy -= y-ymin;
        y = ymin;
      }

      if (x + w > xmax) {
        w = xmax - x;
      }

      if (y + h > ymax) {
        h = ymax - y;
      }

      if (w <= 0 || h <= 0)
        return;

      // TRACE("x=%d xmin=%d xmax=%d w=%d", x, xmin, xmax, w);

      if (scale == 0) {
        /*if (x + w > width) {
          w = width - x;
        }
        if (y + h > height) {
          h = height - y;
        }*/
        if (bmp->getFormat() == BMP_ARGB4444) {
          DMACopyAlphaBitmap(data, width, height, x, y, bmp->getData(), srcw, srch, srcx, srcy, w, h);
        }
        else {
          DMACopyBitmap(data, width, height, x, y, bmp->getData(), srcw, srch, srcx, srcy, w, h);
        }
      }
      else {
        int scaledw = w * scale;
        int scaledh = h * scale;

        if (x + scaledw > width)
          scaledw = width - x;
        if (y + scaledh > height)
          scaledh = height - y;

        for (int i = 0; i < scaledh; i++) {
          pixel_t * p = getPixelPtr(x, y + i);
          const pixel_t * qstart = bmp->getPixelPtr(srcx, srcy + int(i / scale));
          for (int j = 0; j < scaledw; j++) {
            const pixel_t * q = qstart;
            MOVE_PIXEL_RIGHT(q, int(j / scale));
            if (bmp->getFormat() == BMP_ARGB4444) {
              ARGB_SPLIT(*q, a, r, g, b);
              drawAlphaPixel(p, a, RGB_JOIN(r<<1, g<<2, b<<1));
            }
            else {
              drawPixel(p, *q);
            }
            MOVE_TO_NEXT_RIGHT_PIXEL(p);
          }
        }
      }
    }

    template<class T>
    void drawScaledBitmap(const T * bitmap, coord_t x, coord_t y, coord_t w, coord_t h)
    {
      float vscale = float(h) / bitmap->getHeight();
      float hscale = float(w) / bitmap->getWidth();
      float scale = vscale < hscale ? vscale : hscale;

      int xshift = (w - (bitmap->getWidth() * scale)) / 2;
      int yshift = (h - (bitmap->getHeight() * scale)) / 2;
      drawBitmap(x + xshift, y + yshift, bitmap, 0, 0, 0, 0, scale);
    }

  protected:
    static BitmapBuffer * load_bmp(const char * filename);
    static BitmapBuffer * load_stb(const char * filename);
    uint8_t drawChar(coord_t x, coord_t y, const uint8_t * font, const uint16_t * spec, unsigned int index, LcdFlags flags);
};

extern BitmapBuffer * lcd;

#endif // _BITMAP_BUFFER_H_
