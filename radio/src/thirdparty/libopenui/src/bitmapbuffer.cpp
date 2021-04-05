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
#include "libopenui_globals.h"
#include "libopenui_helpers.h"
#include "libopenui_file.h"
#include "font.h"

void BitmapBuffer::drawAlphaPixel(pixel_t * p, uint8_t opacity, uint16_t color)
{
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

void BitmapBuffer::drawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags)
{
  APPLY_OFFSET();

  // line is off-screen
  if (y >= _height || y >= ymax || y < ymin)
    return;

  // clip width
  if (x + w > _width) {
    w = _width - x;
  }

  drawHorizontalLineAbs(x, y, w, pat, flags);
}

void BitmapBuffer::drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags flags)
{
  pixel_t * p = getPixelPtr(x, y);
  pixel_t color = lcdColorTable[COLOR_IDX(flags)];
  uint8_t opacity = 0x0F - (flags >> 24);

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

void BitmapBuffer::drawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat, LcdFlags flags)
{
  APPLY_OFFSET();

  if (x < xmin || x >= xmax)
    return;

  if (h < 0) {
    y += h;
    h = -h;
  }

  if (y < ymin) {
    h += y - ymin;
    y = ymin;
  }

  if (y + h > ymax) {
    h = ymax - y;
  }

  if (h <= 0)
    return;

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];
  uint8_t opacity = 0x0F - (flags >> 24);

  if (pat == SOLID) {
    while (h--) {
      drawAlphaPixel(x, y, opacity, color);
      y++;
    }
  }
  else {
    if (pat==DOTTED && !(y%2)) {
      pat = ~pat;
    }
    while (h--) {
      if (pat & 1) {
        drawAlphaPixel(x, y, opacity, color);
        pat = (pat >> 1) | 0x80;
      }
      else {
        pat = pat >> 1;
      }
      y++;
    }
  }
}

void BitmapBuffer::drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags flags)
{
  // Offsets
  x1 += offsetX;
  y1 += offsetY;
  x2 += offsetX;
  y2 += offsetY;

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];

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
        drawPixel(px, py, color);
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
        drawPixel(px, py, color);
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

void BitmapBuffer::drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness, uint8_t pat, LcdFlags flags)
{
  for (unsigned i = 0; i < thickness; i++) {
    drawVerticalLine(x + i, y, h, pat, flags);
    drawVerticalLine(x + w - 1 - i, y, h, pat, flags);
    drawHorizontalLine(x, y + h - 1 - i, w, pat, flags);
    drawHorizontalLine(x, y + i, w, pat, flags);
  }
}

void BitmapBuffer::drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags)
{
  APPLY_OFFSET();

  if (x >= xmax || y >= ymax)
    return;

  if (h < 0) {
    y += h;
    h = -h;
  }
  if (y < ymin) {
    h += y-ymin;
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

  if (!data || h<=0 || w<=0)
    return;

  DMAFillRect(data, _width, _height, x, y, w, h, lcdColorTable[COLOR_IDX(flags)]);
}

void BitmapBuffer::drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags flags)
{
  APPLY_OFFSET();

  // start is off-screen
  if (x >= xmax || y >= ymax)
    return;

  // make height positive
  if (h < 0) {
    y += h;
    h = -h;
  }

  // clip y
  if (y < ymin) {
    h += y-ymin;
    y = ymin;
  }

  if (y + h > ymax)
    h = ymax - y;

  // clip x
  if (x < xmin) {
    w += x - xmin;
    x = xmin;
  }

  if (x + w > xmax)
    w = xmax - x;

  for (coord_t i = y; i < y + h; i++) {
    drawHorizontalLineAbs(x, i, w, pat, flags);
  }
}

void BitmapBuffer::invertRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags)
{
  APPLY_OFFSET();

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];
  RGB_SPLIT(color, red, green, blue);

  for (int i = y; i < y + h; i++) {
    pixel_t * p = getPixelPtr(x, i);
    for (int j = 0; j < w; j++) {
      // TODO ASSERT_IN_DISPLAY(p);
      RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
      drawPixel(p, RGB_JOIN(0x1F + red - bgRed, 0x3F + green - bgGreen, 0x1F + blue - bgBlue));
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
}

void BitmapBuffer::drawCircle(coord_t x, coord_t y, coord_t radius, LcdFlags flags)
{
  int x1 = radius;
  int y1 = 0;
  int decisionOver2 = 1 - x1;
  pixel_t color = lcdColorTable[COLOR_IDX(flags)];

  APPLY_OFFSET();

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

class Slope {
  public:
    explicit Slope(int angle) {
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

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];
  APPLY_OFFSET();

  coord_t width = *((uint16_t *)img);
  coord_t height = *(((uint16_t *)img) + 1);
  const uint8_t * q = img + 4;

  int w2 = width / 2;
  int h2 = height / 2;

  for (int y1 = h2 - 1; y1 >= 0; y1--) {
    for (int x1 = w2 - 1; x1 >= 0; x1--) {
      Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
      if (slope.isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 - y1, q[(h2 - y1) * width + w2 + x1], color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 + y1, q[(h2 + y1) * width + w2 + x1], color);
      }
      if (slope.invertHorizontal().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 + y1, q[(h2 + y1) * width + w2 - x1], color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 - y1, q[(h2 - y1) * width + w2 - x1], color);
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

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];
  APPLY_OFFSET();

  coord_t internalDist = internalRadius * internalRadius;
  coord_t externalDist = externalRadius * externalRadius;

  for (int y1 = 0; y1 <= externalRadius; y1++) {
    for (int x1 = 0; x1 <= externalRadius; x1++) {
      auto dist = x1 * x1 + y1 * y1;
      if (dist >= internalDist && dist <= externalDist) {
        Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
        if (slope.isBetween(startSlope, endSlope))
          drawPixel(x + x1, y - y1, color);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixel(x + x1, y + y1, color);
        if (slope.invertHorizontal().isBetween(startSlope, endSlope))
          drawPixel(x - x1, y + y1, color);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixel(x - x1, y - y1, color);
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

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];

  for (coord_t row = 0; row < height; row++) {
    if (y + row < ymin || y + row >= ymax)
      continue;
    pixel_t * p = getPixelPtr(x, y + row);
    const pixel_t * q = mask->getPixelPtr(offsetX, row);
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

  for (coord_t row = 0; row < height; row++) {
    if (y + row < ymin || y + row >= ymax)
      continue;
    pixel_t * p = getPixelPtr(x, y + row);
    const pixel_t * q = mask->getPixelPtr(offsetX, offsetY + row);
    for (coord_t col = 0; col < width; col++) {
      drawAlphaPixel(p, *((uint8_t *)q), *srcBitmap->getPixelPtr(row, col));
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
      MOVE_TO_NEXT_RIGHT_PIXEL(q);
    }
  }
}

void BitmapBuffer::drawBitmapPattern(coord_t x, coord_t y, const uint8_t * bmp, LcdFlags flags, coord_t offset, coord_t width)
{
  APPLY_OFFSET();

  coord_t w = *((uint16_t *)bmp);
  coord_t height = *(((uint16_t *)bmp)+1);

  if (!width || width > w) {
    width = w;
  }

  if (x + width > xmax) {
    width = xmax - x;
  }

  if (y >= ymax || x >= xmax || width <= 0 || x + width < xmin || y + height < ymin) {
    return;
  }

  pixel_t color = lcdColorTable[COLOR_IDX(flags)];

  for (coord_t row=0; row<height; row++) {
    if (y + row < ymin || y + row >= ymax)
      continue;
    const uint8_t * q = bmp + 4 + row*w + offset;
    for (coord_t col=0; col<width; col++) {
      coord_t xpixel, ypixel;
      if (flags & VERTICAL) {
        xpixel = x + row;
        ypixel = y - col;
      }
      else {
        xpixel = x + col;
        ypixel = y + row;
      }
      if (xpixel >= xmin && xpixel < xmax) {
        pixel_t * p = getPixelPtr(xpixel, ypixel);
        if (p)
          drawAlphaPixel(p, *q, color);
      }
      q++;
    }
  }
}

uint8_t BitmapBuffer::drawChar(coord_t x, coord_t y, const uint8_t * font, const uint16_t * spec, unsigned int index, LcdFlags flags)
{
  coord_t offset = spec[index + 1];
  coord_t width = spec[index + 2] - offset;
  if (width > 0) {
    drawBitmapPattern(x, y, font, flags, offset, width);
  }
  return width;
}

#define INCREMENT_POS(delta)    do { if (flags & VERTICAL) y -= delta; else x += delta; } while(0)

coord_t BitmapBuffer::drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags)
{
  MOVE_OFFSET();

  int height = getFontHeight(flags);

  if (y + height <= ymin || y >= ymax) {
    RESTORE_OFFSET();
    return x;
  }

  uint32_t fontindex = FONT_INDEX(flags);
  const unsigned char * font = fontsTable[fontindex];
  const uint16_t * fontspecs = fontspecsTable[fontindex];

  if (flags & (RIGHT | CENTERED)) {
    int width = getTextWidth(s, len, flags);
    if (flags & RIGHT) {
      INCREMENT_POS(-width);
    }
    else if (flags & CENTERED) {
      INCREMENT_POS(-width / 2);
    }
  }

  coord_t & pos = (flags & VERTICAL) ? y : x;
  const coord_t orig_pos = pos;

  while (len--) {
    unsigned int c = uint8_t(*s);
    // TRACE("c = %d %o 0x%X '%c'", c, c, c, c);

    if (!c) {
      break;
    }
    else if (c >= 0xFE) {
      // CJK char
      s++;
      c = uint8_t(*s) + ((c & 0x01u) << 8u) - 1;
      // TRACE("CJK = %d", c);
      if (c >= 0x101)
        c -= 1;
      c += CJK_FIRST_LETTER_INDEX;
      uint8_t width = drawChar(x, y, font, fontspecs, c, flags);
      INCREMENT_POS(width + CHAR_SPACING);
    }
    else if (c >= 0x20) {
      uint8_t width = drawChar(x, y, font, fontspecs, getMappedChar(c), flags);
      if ((flags & SPACING_NUMBERS_CONST) && c >= '0' && c <= '9')
        INCREMENT_POS(getCharWidth('9', fontspecs) + CHAR_SPACING);
      else
        INCREMENT_POS(width + CHAR_SPACING);
    }
    else if (c == '\n') {
      pos = orig_pos;
      if (flags & VERTICAL)
        x += height;
      else
        y += height;
    }

    s++;
  }

  RESTORE_OFFSET();

  return ((flags & RIGHT) ? orig_pos : pos) - offsetX;
}

coord_t BitmapBuffer::drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len, const char * prefix, const char * suffix)
{
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
  flags &= ~LEADING0;
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
  const char * ext = getFileExtension(filename);
  if (ext && !strcmp(ext, ".bmp"))
    return load_bmp(filename);
  else
    return load_stb(filename);
}

BitmapBuffer * BitmapBuffer::loadMask(const char * filename)
{
  BitmapBuffer * bitmap = BitmapBuffer::loadBitmap(filename);
  if (bitmap) {
    pixel_t * p = bitmap->getPixelPtr(0, 0);
    for (int i = bitmap->width() * bitmap->height(); i > 0; i--) {
      *((uint8_t *)p) = OPACITY_MAX - ((*p) >> 12);
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
        pixel_t * dst = bmp->getPixelPtr(0, i);
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
        pixel_t * dst = bmp->getPixelPtr(0, i);
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
              for (pixel_t * p = bmp->getPixelPtr(j, i); p != bmp->getPixelPtr(0, h); MOVE_TO_NEXT_RIGHT_PIXEL(p)) {
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
        pixel_t * dst = bmp->getPixelPtr(0, i);
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

#include "thirdparty/Stb/stb_image.h"

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
  FRESULT result = f_open(&imgFile, filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return nullptr;
  }

  int w, h, n;
  unsigned char * img = stbi_load_from_callbacks(&stbCallbacks, &imgFile, &w, &h, &n, 4);
  f_close(&imgFile);

  if (!img) {
    return nullptr;
  }

  // convert to RGB565 or ARGB4444 format
  BitmapBuffer * bmp = new BitmapBuffer(n == 4 ? BMP_ARGB4444 : BMP_RGB565, w, h);
  if (bmp == nullptr) {
    TRACE("load_stb() malloc failed");
    stbi_image_free(img);
    return nullptr;
  }

#if 0
  DMABitmapConvert(bmp->data, img, w, h, n == 4 ? DMA2D_ARGB4444 : DMA2D_RGB565);
#else
  pixel_t * dest = bmp->getPixelPtr(0, 0);
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
  else {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = RGB(p[0], p[1], p[2]);
        MOVE_TO_NEXT_RIGHT_PIXEL(dest);
        p += 4;
      }
    }
  }
#endif

  stbi_image_free(img);
  return bmp;
}
