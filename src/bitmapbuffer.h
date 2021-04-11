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

#pragma once

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
#define APPLY_OFFSET() x += this->offsetX; y += this->offsetY
#define RESTORE_OFFSET()  this->offsetX = offsetX, this->offsetY = offsetY

#if defined(LCD_VERTICAL_INVERT)
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
      _width(width),
      _height(height),
      xmax(width),
      ymax(height),
      data(data),
      data_end(data + (width * height))
    {
    }

    BitmapBufferBase(uint8_t format, T * data):
      format(format),
      _width(*((uint16_t*)data)),
      _height(*(((uint16_t*)data) + 1)),
      xmax(_width),
      ymax(_height),
      data(data + 2),
      data_end(data + 2 + (_width * _height))
    {
    }

    inline void clearClippingRect()
    {
      xmin = 0;
      xmax = _width;
      ymin = 0;
      ymax = _height;
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

    inline void clearOffset()
    {
      setOffset(0, 0);
    }

    inline void reset()
    {
      clearOffset();
      clearClippingRect();
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

    inline uint16_t width() const
    {
      return _width;
    }

    inline uint16_t height() const
    {
      return _height;
    }

    inline T * getData() const
    {
      return data;
    }

    uint32_t getDataSize() const
    {
      return _width * _height * sizeof(T);
    }

    inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
#if defined(LCD_VERTICAL_INVERT)
      x = _width - x - 1;
      y = _height - y - 1;
#endif
      return &data[y * _width + x];
    }

  protected:
    uint8_t format;
    coord_t _width;
    coord_t _height;
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
      _width = *((uint16_t *)rle_data);
      _height = *(((uint16_t *)rle_data)+1);
      uint32_t pixels = _width * _height;
      data = (uint16_t*)malloc(pixels * sizeof(uint16_t));
      decode((uint8_t *)data, pixels * sizeof(uint16_t), rle_data+4);
      data_end = data + pixels;
    }

    ~RLEBitmap()
    {
      free(data);
    }

    static int decode(uint8_t * dest, unsigned int destSize, const uint8_t * src)
    {
      uint8_t prevByte = 0;
      bool prevByteValid = false;

      const uint8_t * destEnd = dest + destSize;
      uint8_t * d = dest;

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
        }
        else {
          prevByte = byte;
          prevByteValid = true;
        }
      }

      return d - dest;
    }
};

class BitmapBuffer: public BitmapBufferBase<pixel_t>
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
      data = (uint16_t *)malloc(width * height * sizeof(uint16_t));
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
      drawSolidFilledRect(0, 0, _width - offsetX, _height - offsetY, flags);
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
#if defined(LCD_VERTICAL_INVERT)
      x = _width - x - 1;
      y = _height - y - 1;
#endif
      return &data[y * _width + x];
    }

    inline pixel_t * getPixelPtr(coord_t x, coord_t y)
    {
#if defined(LCD_VERTICAL_INVERT)
      x = _width - x - 1;
      y = _height - y - 1;
#endif
      return &data[y * _width + x];
    }

    inline void drawPixel(coord_t x, coord_t y, pixel_t value)
    {
      APPLY_OFFSET();

      coord_t w = 1, h = 1;
      if (!applyClippingRect(x, y, w, h))
        return;

      drawPixelAbs(x, y, value);
    }

    void drawAlphaPixel(pixel_t * p, uint8_t opacity, uint16_t color);

    inline void drawAlphaPixel(coord_t x, coord_t y, uint8_t opacity, pixel_t value)
    {
      APPLY_OFFSET();

      coord_t w = 1, h = 1;
      if (!applyClippingRect(x, y, w, h))
        return;

      drawAlphaPixelAbs(x, y, opacity, value);
    }

    void drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat = SOLID, LcdFlags flags = 0);

    void drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat = SOLID, LcdFlags flags = 0);

    void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags att);

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

    void drawCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags = 0);

    void drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags = 0);

    void drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, int startAngle, int endAngle, LcdFlags flags = 0);

    void drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle);

    void drawBitmapPatternPie(coord_t x0, coord_t y0, const uint8_t * img, LcdFlags flags, int startAngle, int endAngle);

    static BitmapBuffer * loadBitmap(const char * filename);

    static BitmapBuffer * loadMask(const char * filename);

    static BitmapBuffer * loadMaskOnBackground(const char * filename, LcdFlags foreground, LcdFlags background);

    void drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, LcdFlags flags, coord_t offsetX = 0, coord_t width = 0);

    void drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, const BitmapBuffer * srcBitmap, coord_t offsetX = 0, coord_t offsetY = 0, coord_t width = 0, coord_t height = 0);

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
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0, float scale = 0)
    {
      if (!data || !bmp)
        return;

      APPLY_OFFSET();

      if (x >= xmax || y >= ymax)
        return;

      coord_t bmpw = bmp->width();
      coord_t bmph = bmp->height();

      if (srcw == 0)
        srcw = bmpw;
      if (srch == 0)
        srch = bmph;
      if (srcx + srcw > bmpw)
        srcw = bmpw - srcx;
      if (srcy + srch > bmph)
        srch = bmph - srcy;

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
      }
      else {
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
          DMACopyAlphaBitmap(data, _width, _height, x, y, bmp->getData(), bmpw, bmph, srcx, srcy, srcw, srch);
        }
        else {
          DMACopyBitmap(data, _width, _height, x, y, bmp->getData(), bmpw, bmph, srcx, srcy, srcw, srch);
        }
      }
      else {
        int scaledw = srcw * scale;
        int scaledh = srch * scale;

        if (x + scaledw > _width)
          scaledw = _width - x;
        if (y + scaledh > _height)
          scaledh = _height - y;

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
      if (bitmap) {
        float vscale = float(h) / bitmap->height();
        float hscale = float(w) / bitmap->width();
        float scale = vscale < hscale ? vscale : hscale;

        int xshift = (w - (bitmap->width() * scale)) / 2;
        int yshift = (h - (bitmap->height() * scale)) / 2;
        drawBitmap(x + xshift, y + yshift, bitmap, 0, 0, 0, 0, scale);
      }
    }

    BitmapBuffer * horizontalFlip() const;

    BitmapBuffer * verticalFlip() const;

    BitmapBuffer * invertMask() const;

  protected:
    static BitmapBuffer * load_bmp(const char * filename);
    static BitmapBuffer * load_stb(const char * filename);

    inline bool applyClippingRect(coord_t & x, coord_t & y, coord_t & w, coord_t & h)
    {
      if (h < 0) {
        y += h;
        h = -h;
      }

      if (w < 0) {
        x += w;
        w = -w;
      }

      if (x >= xmax || y >= ymax)
        return false;

      if (y < ymin) {
        h += y - ymin;
        y = ymin;
      }

      if (x < xmin) {
        w += x - xmin;
        x = xmin;
      }

      if (y + h > ymax)
        h = ymax - y;

      if (x + w > xmax)
        w = xmax - x;

      return data && h > 0 && w > 0;
    }

    uint8_t drawChar(coord_t x, coord_t y, const uint8_t * font, const uint16_t * spec, unsigned int index, LcdFlags flags);

    inline void drawPixelAbs(coord_t x, coord_t y, pixel_t value)
    {
      pixel_t * p = getPixelPtr(x, y);
      drawPixel(p, value);
    }

    inline void drawAlphaPixelAbs(coord_t x, coord_t y, uint8_t opacity, uint16_t color)
    {
      pixel_t * p = getPixelPtr(x, y);
      drawAlphaPixel(p, opacity, color);
    }

    void drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags);
};

extern BitmapBuffer * lcd;

