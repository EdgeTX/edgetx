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

#pragma once

#include "libopenui_defines.h"

struct MaskBitmap;
class TelemetryItem;

constexpr uint8_t SOLID = 0xFF;
constexpr uint8_t DOTTED = 0x55;
constexpr uint8_t STASHED = 0x33;

#define MOVE_OFFSET()              \
  coord_t offsetX = this->offsetX; \
  x += offsetX;                    \
  this->offsetX = 0;               \
  coord_t offsetY = this->offsetY; \
  y += offsetY;                    \
  this->offsetY = 0

#define APPLY_OFFSET() \
  x += this->offsetX;  \
  y += this->offsetY

#define RESTORE_OFFSET() this->offsetX = offsetX, this->offsetY = offsetY

#define MOVE_PIXEL_RIGHT(p, count) p += count

#define MOVE_TO_NEXT_RIGHT_PIXEL(p) MOVE_PIXEL_RIGHT(p, 1)

#define USE_STB

enum BitmapFormats { BMP_INVALID = -1, BMP_RGB565 = 0, BMP_ARGB4444 };

class BitmapBuffer
{
 public:
  BitmapBuffer(uint8_t format, uint16_t width, uint16_t height);
  BitmapBuffer(uint8_t format, uint16_t width, uint16_t height, uint16_t* data);

  ~BitmapBuffer();

  void setDrawCtx(lv_draw_ctx_t* ctx) { draw_ctx = ctx; }

  void setData(uint16_t* d);

  void clear(LcdFlags flags = 0);

  inline const pixel_t* getPixelPtrAbs(coord_t x, coord_t y) const
  {
    return &data[y * _width + x];
  }

  inline pixel_t* getPixelPtrAbs(coord_t x, coord_t y)
  {
    return &data[y * _width + x];
  }

  inline void drawPixel(coord_t x, coord_t y, pixel_t value)
  {
    APPLY_OFFSET();

    coord_t w = 1, h = 1;
    if (!applyClippingRect(x, y, w, h)) return;

    drawPixelAbs(x, y, value);
  }

  void drawAlphaPixel(pixel_t* p, uint8_t opacity, uint16_t color);

  inline void drawAlphaPixel(coord_t x, coord_t y, uint8_t opacity,
                             pixel_t value)
  {
    APPLY_OFFSET();

    coord_t w = 1, h = 1;
    if (!applyClippingRect(x, y, w, h)) return;

    pixel_t* p = getPixelPtrAbs(x, y);
    drawAlphaPixel(p, opacity, value);
  }

  void drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat = SOLID,
                          LcdFlags flags = 0, uint8_t opacity = 0);

  void drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat = SOLID,
                        LcdFlags flags = 0, uint8_t opacity = 0);

  void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat,
                LcdFlags att);

  void drawRect(coord_t x, coord_t y, coord_t w, coord_t h,
                uint8_t thickness = 1, uint8_t pat = SOLID, LcdFlags flags = 0,
                uint8_t opacity = 0);

  void drawSolidRect(coord_t x, coord_t y, coord_t w, coord_t h,
                     uint8_t thickness = 1, LcdFlags flags = 0);

  void drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h,
                           LcdFlags flags = 0);

  void drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h,
                      uint8_t pat = SOLID, LcdFlags flags = 0,
                      uint8_t opacity = 0);

  static BitmapBuffer* loadBitmap(const char* filename,
                                  BitmapFormats fmt = BMP_INVALID);

  void resizeToLVGL(coord_t w, coord_t h);

  coord_t drawSizedText(coord_t x, coord_t y, const char* s, uint8_t len,
                        LcdFlags flags = 0);

  coord_t drawText(coord_t x, coord_t y, const char* s, LcdFlags flags = 0)
  {
    if (!s) return x;
    return drawSizedText(x, y, s, strlen(s), flags);
  }

  void drawBitmap(coord_t x, coord_t y, const BitmapBuffer* bmp,
                  coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0,
                  coord_t srch = 0, float scale = 0);

  void clearClippingRect();

  void setOffset(coord_t offsetX, coord_t offsetY);

  inline void clearOffset() { setOffset(0, 0); }

  inline void reset()
  {
    clearOffset();
    clearClippingRect();
  }

  coord_t getOffsetX() const { return offsetX; }

  coord_t getOffsetY() const { return offsetY; }

  inline uint16_t width() const { return _width; }
  inline uint16_t height() const { return _height; }

  inline pixel_t* getData() const { return data; }

  uint32_t getDataSize() const { return _width * _height * sizeof(pixel_t); }

  // Lua API functions
  void setClippingRect(coord_t xmin, coord_t xmax, coord_t ymin, coord_t ymax);
  void getClippingRect(coord_t& xmin, coord_t& xmax, coord_t& ymin,
                       coord_t& ymax);

  MaskBitmap* to8bitMask(size_t* size) const;

  void invertRect(coord_t x, coord_t y, coord_t w, coord_t h,
                  LcdFlags flags = 0);

  void drawFilledTriangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
                          coord_t x3, coord_t y3, LcdFlags flags = 0,
                          uint8_t opacity = 0);
  void drawCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags = 0, coord_t thickness = 1);
  void drawFilledCircle(coord_t x, coord_t y, coord_t radius,
                        LcdFlags flags = 0);
  void drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius,
                         coord_t externalRadius, int startAngle, int endAngle,
                         LcdFlags flags = 0);

  void drawBitmapPatternPie(coord_t x0, coord_t y0, const MaskBitmap* img,
                            LcdFlags flags, int startAngle, int endAngle);
  void drawBitmapPattern(coord_t x, coord_t y, const MaskBitmap* bmp,
                         LcdFlags flags, coord_t offset = 0, coord_t width = 0);
  void drawScaledBitmap(const BitmapBuffer* bitmap, coord_t x, coord_t y,
                        coord_t w, coord_t h);
  coord_t drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags = 0,
                     uint8_t len = 0, const char* prefix = nullptr,
                     const char* suffix = nullptr);
  point_t drawTextLines(coord_t left, coord_t top, coord_t width, coord_t height,
                     const char* str, LcdFlags flags);
  void drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags flags = 0);
  void drawSource(coord_t x, coord_t y, mixsrc_t idx, LcdFlags flags = 0);
  coord_t drawSwitch(coord_t x, coord_t y, int32_t idx, LcdFlags flags = 0);
  void drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor,
                             int32_t value, LcdFlags flags = 0);

 protected:
  void drawValueWithUnit(coord_t x, coord_t y, int val, uint8_t unit,
                         LcdFlags flags = 0);

  coord_t drawGPSCoord(coord_t x, coord_t y, int32_t value,
                       const char* direction, LcdFlags flags,
                       bool seconds = true);
  void drawGPSPosition(coord_t x, coord_t y, int32_t longitude,
                       int32_t latitude, LcdFlags flags = 0);

  void drawDate(coord_t x, coord_t y, TelemetryItem& telemetryItem,
                LcdFlags att);

  bool applyClippingRect(coord_t& x, coord_t& y, coord_t& w,
                         coord_t& h) const;

  inline void drawPixel(pixel_t* p, pixel_t value)
  {
    if (data && (data <= p && p < data_end)) {
      *p = value;
    }
#if defined(DEBUG)
    else if (!leakReported) {
      leakReported = true;
      TRACE(
          "BitmapBuffer(%p).drawPixel(): buffer overrun, data: %p, written at: "
          "%p",
          this, data, p);
    }
#endif
  }

  inline void drawPixelAbs(coord_t x, coord_t y, pixel_t value)
  {
    pixel_t* p = getPixelPtrAbs(x, y);
    drawPixel(p, value);
  }

  void drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, uint8_t pat,
                             LcdFlags flags, uint8_t opacity);

  bool liangBarskyClipper(coord_t& x1, coord_t& y1, coord_t& x2, coord_t& y2);

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
  pixel_t* data;
  pixel_t* data_end;

 private:
  bool dataAllocated = false;
#if defined(DEBUG)
  bool leakReported;
#endif
  lv_obj_t* canvas = nullptr;
  lv_draw_ctx_t* draw_ctx = nullptr;
};

// Back buffer to draw
extern BitmapBuffer* lcd;

// Buffer currently on display
extern BitmapBuffer* lcdFront;
