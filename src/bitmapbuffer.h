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
#include "libopenui_helpers.h"
#include "debug.h"

constexpr uint8_t SOLID = 0xFF;
constexpr uint8_t DOTTED  = 0x55;
constexpr uint8_t STASHED = 0x33;

#define MOVE_OFFSET() coord_t offsetX = this->offsetX; x += offsetX; this->offsetX = 0; coord_t offsetY = this->offsetY; y += offsetY; this->offsetY = 0
#define APPLY_OFFSET() x += this->offsetX; y += this->offsetY
#define RESTORE_OFFSET()  this->offsetX = offsetX, this->offsetY = offsetY

#if defined(LCD_VERTICAL_INVERT) && 0
  #define MOVE_PIXEL_RIGHT(p, count)   p -= count
#else
  #define MOVE_PIXEL_RIGHT(p, count)   p += count
#endif

#define MOVE_TO_NEXT_RIGHT_PIXEL(p)    MOVE_PIXEL_RIGHT(p, 1)

#define USE_STB

enum BitmapFormats
{
  BMP_8BIT,
  BMP_RGB565,
  BMP_ARGB4444
};

struct _lv_draw_ctx_t;
typedef _lv_draw_ctx_t lv_draw_ctx_t;

struct _lv_obj_t;
typedef _lv_obj_t lv_obj_t;

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

    inline const pixel_t * getPixelPtrAbs(coord_t x, coord_t y) const
    {
#if defined(LCD_VERTICAL_INVERT) && 0
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
    RLEBitmap(uint8_t format, const uint8_t* rle_data);
    ~RLEBitmap();

    static int decode(uint8_t * dest, unsigned int destSize, const uint8_t * src);
};

class BitmapBuffer: public BitmapBufferBase<pixel_t>
{
  private:
    bool dataAllocated;
#if defined(DEBUG)
    bool leakReported;
#endif
    lv_obj_t* canvas = nullptr;
    lv_draw_ctx_t* draw_ctx = nullptr;

  public:
    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height);
    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height, uint16_t * data);

    ~BitmapBuffer();

    void setDrawCtx(lv_draw_ctx_t* ctx) { draw_ctx = ctx; }
    void setData(uint16_t* d) {
      if (!dataAllocated) {
        data = d;
        data_end = d + (_width * _height);
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

    inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
      APPLY_OFFSET();

      coord_t w = 1, h = 1;
      if (!applyClippingRect(x, y, w, h))
        return nullptr;

      return getPixelPtrAbs(x, y);
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

    void drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat = SOLID, LcdFlags flags = 0, uint8_t opacity = 0);

    void drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat = SOLID, LcdFlags flags = 0, uint8_t opacity = 0);

    void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags att);

    inline void drawSolidHorizontalLine(coord_t x, coord_t y, coord_t w, LcdFlags flags)
    {
      drawSolidFilledRect(x, y, w, 1, flags);
    }

    inline void drawSolidVerticalLine(coord_t x, coord_t y, coord_t h, LcdFlags flags)
    {
      drawSolidFilledRect(x, y, 1, h, flags);
    }

    void drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness = 1, uint8_t pat = SOLID, LcdFlags flags = 0, uint8_t opacity = 0);

    inline void drawSolidRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness = 1, LcdFlags flags = 0)
    {
      drawSolidFilledRect(x, y, thickness, h, flags);
      drawSolidFilledRect(x+w-thickness, y, thickness, h, flags);
      drawSolidFilledRect(x, y, w, thickness, flags);
      drawSolidFilledRect(x, y+h-thickness, w, thickness, flags);
    }

    void drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags = 0);

    void drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat = SOLID, LcdFlags flags = 0, uint8_t opacity = 0);

    void invertRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags = 0);

    void drawFilledTriangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t x3, coord_t y3, LcdFlags flags = 0, uint8_t opacity = 0);

    void drawCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags = 0);

    void drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags = 0);

    void drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, int startAngle, int endAngle, LcdFlags flags = 0);

    void drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle);

    void drawBitmapPatternPie(coord_t x0, coord_t y0, const uint8_t * img, LcdFlags flags, int startAngle, int endAngle);

    static BitmapBuffer * loadBitmap(const char * filename);
    static BitmapBuffer * loadRamBitmap(const uint8_t * buffer, int len);

    static BitmapBuffer * loadMask(const char * filename);
    static BitmapBuffer * load8bitMask(const uint8_t * lbm);
    static BitmapBuffer * load8bitMaskLZ4(const uint8_t * compressed_data);

    static BitmapBuffer * loadMaskOnBackground(const char * filename, LcdFlags foreground, LcdFlags background);
    static BitmapBuffer * load8bitMaskOnBackground(const uint8_t * lbm, LcdFlags foreground, LcdFlags background);

    static uint8_t * loadFont(const uint8_t * lbm, int len, int& w, int& h);
  
    void drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, LcdFlags flags, coord_t offsetX = 0, coord_t width = 0);

    void drawMask(coord_t x, coord_t y, const BitmapBuffer * mask, const BitmapBuffer * srcBitmap, coord_t offsetX = 0, coord_t offsetY = 0, coord_t width = 0, coord_t height = 0);

    void drawBitmapPattern(coord_t x, coord_t y, const uint8_t * bmp, LcdFlags flags, coord_t offset=0, coord_t width=0);

    coord_t drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags=0);

    coord_t drawText(coord_t x, coord_t y, const char * s, LcdFlags flags = 0)
    {
      if (!s) return x;
      return drawSizedText(x, y, s, strlen(s), flags);
    }

    coord_t drawTextAtIndex(coord_t x, coord_t y, const char** s, uint8_t idx, LcdFlags flags = 0)
    {
      return drawText(x, y, s[idx], flags);
    }

    static void formatNumberAsString(char *buffer, const uint8_t buffer_size, int32_t val, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr);

    coord_t drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr);

    template<class T>
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0, float scale = 0);

    template<class T>
    void drawScaledBitmap(const T * bitmap, coord_t x, coord_t y, coord_t w, coord_t h);

    BitmapBuffer * horizontalFlip() const;

    BitmapBuffer * verticalFlip() const;

    BitmapBuffer * invertMask() const;

  protected:
    static BitmapBuffer * load_bmp(const char * filename);
    static BitmapBuffer * load_stb(const char * filename);
    static BitmapBuffer * load_stb_buffer(const uint8_t * buffer, int len);
    static BitmapBuffer * convert_stb_bitmap(uint8_t * img, int w, int h, int n);

    inline bool applyClippingRect(coord_t & x, coord_t & y, coord_t & w, coord_t & h) const
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

    template <class T>
    void drawBitmapAbs(coord_t x, coord_t y, const T* bmp, coord_t srcx = 0,
                       coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0,
                       float scale = 0);

    inline void drawPixel(pixel_t * p, pixel_t value)
    {
      if (data && (data <= p && p < data_end)) {
        *p = value;
      }
#if defined(DEBUG)
      else if (!leakReported) {
        leakReported = true;
        TRACE("BitmapBuffer(%p).drawPixel(): buffer overrun, data: %p, written at: %p", this, data, p);
      }
#endif
    }

    inline const pixel_t * getPixelPtrAbs(coord_t x, coord_t y) const
    {
#if defined(LCD_VERTICAL_INVERT) && 0
      x = _width - x - 1;
      y = _height - y - 1;
#endif
      return &data[y * _width + x];
    }

    inline pixel_t * getPixelPtrAbs(coord_t x, coord_t y)
    {
#if defined(LCD_VERTICAL_INVERT) && 0
      x = _width - x - 1;
      y = _height - y - 1;
#endif
      return &data[y * _width + x];
    }

    inline void drawPixelAbs(coord_t x, coord_t y, pixel_t value)
    {
      pixel_t * p = getPixelPtrAbs(x, y);
      drawPixel(p, value);
    }

    inline void drawAlphaPixelAbs(coord_t x, coord_t y, uint8_t opacity, uint16_t color)
    {
      pixel_t * p = getPixelPtrAbs(x, y);
      drawAlphaPixel(p, opacity, color);
    }

    void drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags, uint8_t opacity);

    bool liangBarskyClipper(coord_t& x1, coord_t& y1, coord_t& x2, coord_t& y2);
};

class LZ4Bitmap : public BitmapBuffer
{
 public:
  LZ4Bitmap(uint8_t format, const uint8_t* compressed_data);
  ~LZ4Bitmap();
};

class LZ4Mask : public BitmapBufferBase<uint8_t>
{
 public:
  LZ4Mask(const uint8_t* compressed_data);
  ~LZ4Mask();
};

// Back buffer to draw
extern BitmapBuffer * lcd;

// Buffer currently on display
extern BitmapBuffer * lcdFront;

