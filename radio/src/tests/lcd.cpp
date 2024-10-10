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
/* -*- coding: utf-8 -*- */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QApplication>
#include <QPainter>
#include <math.h>
#include <gtest/gtest.h>

#define SWAP_DEFINED
#include "edgetx.h"
#include "location.h"
#include "targets/simu/simulcd.h"

void doPaint(QPainter & p)
{
  QRgb rgb = qRgb(161, 161, 161);

  p.setBackground(QBrush(rgb));
  p.eraseRect(0, 0, LCD_W, LCD_H);

  if (1) {
#if LCD_W < 212
    rgb = qRgb(0, 0, 0);
    p.setPen(rgb);
    p.setBrush(QBrush(rgb));
#endif

#if LCD_W >= 212
    unsigned int previousDepth = 0xFF;
#endif

    for (int y=0; y<LCD_H; y++) {
#if LCD_W >= 212
      unsigned int idx = (y/2) * LCD_W;
#else
      unsigned int idx = (y/8) * LCD_W;
      unsigned int mask = (1 << (y%8));
#endif
      for (int x=0; x<LCD_W; x++, idx++) {
#if LCD_W < 212
        if (simuLcdBuf[idx] & mask) {
          p.drawPoint(x, y);
        }
#else
        unsigned int z = (y & 1) ? (simuLcdBuf[idx] >> 4) : (simuLcdBuf[idx] & 0x0F);
        if (z) {
          if (z != previousDepth) {
            previousDepth = z;
            rgb = qRgb(161-(z*161)/15, 161-(z*161)/15, 161-(z*161)/15);
            p.setPen(rgb);
            p.setBrush(QBrush(rgb));
          }
          p.drawPoint(x, y);
        }
#endif
      }
    }
  }
}

bool checkScreenshot(const QString & test)
{
  lcdRefresh();
  QImage buffer(LCD_W, LCD_H, QImage::Format_RGB32);
  QPainter p(&buffer);
  doPaint(p);
  QString filename(QString("%1_%2x%3.png").arg(test).arg(LCD_W).arg(LCD_H));
  QImage reference(TESTS_PATH "/images/bw/" + filename);

  if (buffer == reference) {
    return true;
  }
  else {
    buffer.save(TESTS_PATH "/images/bw/failed_" + filename);
    return false;
  }
}

#if !defined(COLORLCD)
TEST(outdezNAtt, test_unsigned)
{
  lcdClear();
  lcdDrawNumber(0, 0, 65530, LEFT);
  EXPECT_TRUE(checkScreenshot("unsigned")) << "Unsigned numbers will be bad displayed";
}

TEST(outdezNAtt, testBigNumbers)
{
  lcdClear();
  lcdDrawNumber(0, 0, 1234567, LEFT);
  lcdDrawNumber(0, FH, -1234567, LEFT);
  EXPECT_TRUE(checkScreenshot("big_numbers"));
}

TEST(Lcd, Invers_0_0)
{
  lcdClear();
  lcdDrawText(0, 0, "Test", INVERS);
  EXPECT_TRUE(checkScreenshot("invers_0_0"));
}

TEST(Lcd, Invers_0_1)
{
  lcdClear();
  lcdDrawText(0, 1, "Test", INVERS);
  EXPECT_TRUE(checkScreenshot("invers_0_1"));
}

TEST(Lcd, Prec2_Left)
{
  lcdClear();
  lcdDrawNumber(0, 0, 2, PREC2|LEFT);
  EXPECT_TRUE(checkScreenshot("prec2_left"));
}

TEST(Lcd, Prec2_Right)
{
  lcdClear();
  lcdDrawNumber(LCD_W, LCD_H-FH, 2, PREC2|RIGHT);
  EXPECT_TRUE(checkScreenshot("prec2_right"));
}

TEST(Lcd, Prec1_Dblsize_Invers)
{
  lcdClear();
  lcdDrawNumber(LCD_W, 10, 51, PREC1|DBLSIZE|INVERS|RIGHT);
  EXPECT_TRUE(checkScreenshot("prec1_dblsize_invers"));
}

TEST(Lcd, Line_Wrap)
{
  lcdClear();
  lcdDrawText(LCD_W-10, 0, "TEST");
  EXPECT_TRUE(checkScreenshot("line_wrap"));
}

TEST(Lcd, DblsizeBottomRight)
{
  lcdClear();
  lcdDrawText(LCD_W-20, LCD_H-16, "TEST", DBLSIZE);
  EXPECT_TRUE(checkScreenshot("dblsize_bottom_right"));
}

TEST(Lcd, Smlsize_drawStringWithIndex)
{
  lcdClear();
  drawStringWithIndex(0, 0, "FM", 0, SMLSIZE);
  EXPECT_TRUE(checkScreenshot("smlsize_drawstringwithindex"));
}

TEST(Lcd, vline)
{
  lcdClear();
  for (int x=0; x<100; x+=2) {
    lcdDrawSolidVerticalLine(x, x/2, 12);
  }
  EXPECT_TRUE(checkScreenshot("vline"));
}

TEST(Lcd, vline_x_lt0)
{
  lcdClear();
  lcdDrawSolidVerticalLine(50, -10, 12);
  lcdDrawSolidVerticalLine(100, -10, 1);
  EXPECT_TRUE(checkScreenshot("vline_lt0"));
}

TEST(Lcd, Smlsize)
{
  lcdClear();
  lcdDrawText(0, 0, "TESTgy,", SMLSIZE);
  lcdDrawText(10, 22, "TESTgy,", SMLSIZE|INVERS);
  lcdDrawSolidFilledRect(8, 40, 100, 20);
  lcdDrawText(10, 42, "TESTgy,", SMLSIZE);

  bool invert = false;
  for(int i=0; i<3; i++) {
    lcdDrawText(40+(4*i), 0+(4*i), "ABC", SMLSIZE|(invert?INVERS:0));
    invert = !invert;
  }

  EXPECT_TRUE(checkScreenshot("smlsize"));
}

TEST(Lcd, Stdsize)
{
  lcdClear();
  lcdDrawText(0, 0, "TEST", 0);
  lcdDrawText(10, 22, "TEST", INVERS);
  lcdDrawSolidFilledRect(8, 40, 100, 20);
  lcdDrawText(10, 42, "TEST", 0);

  bool invert = false;
  for(int i=0; i<3; i++) {
    lcdDrawText(40+(4*i), 0+(4*i), "ABC", (invert?INVERS:0));
    invert = !invert;
  }

  EXPECT_TRUE(checkScreenshot("stdsize"));
}

TEST(Lcd, Midsize)
{
  lcdClear();
  lcdDrawText(0, 0, "TEST", MIDSIZE);
  lcdDrawText(10, 22, "TEST", MIDSIZE|INVERS);
  lcdDrawSolidFilledRect(8, 40, 100, 20);
  lcdDrawText(10, 42, "TEST", MIDSIZE);

  bool invert = false;
  for(int i=0; i<3; i++) {
    lcdDrawText(40+(4*i), 0+(4*i), "ABC", MIDSIZE|(invert?INVERS:0));
    invert = !invert;
  }

  EXPECT_TRUE(checkScreenshot("midsize"));
}

TEST(Lcd, MidsizeNextPosPrec1)
{
  lcdClear();

  lcdDrawText(10, 1, "80", MIDSIZE);
  lcdDrawText(lcdNextPos, 1, "V", MIDSIZE);

  lcdDrawNumber(10, 15, 80, MIDSIZE | PREC1);
  lcdDrawText(lcdNextPos, 15, "V", MIDSIZE);

  EXPECT_TRUE(checkScreenshot("MidsizeNextPosPrec1"));
}

TEST(Lcd, Dblsize)
{
  lcdClear();
  lcdDrawText(2, 10, "TST", DBLSIZE);
  lcdDrawText(42, 10, "TST", DBLSIZE|INVERS);
  lcdDrawSolidFilledRect(80, 8, 46, 24);
  lcdDrawText(82, 10, "TST", DBLSIZE);

  bool invert = false;
  for(int i=0; i<3; i++) {
    lcdDrawText(10+(4*i), 30+(4*i), "ABC", DBLSIZE|(invert?INVERS:0));
    invert = !invert;
  }

  EXPECT_TRUE(checkScreenshot("dblsize"));
}

#define TEST_CHAR_RIGHT     "\302\200"
#define TEST_CHAR_LEFT      "\302\201"
#define TEST_CHAR_UP        "\302\202"
#define TEST_CHAR_DOWN      "\302\203"

#define TEST_CHAR_DELTA     "\302\210"
#define TEST_CHAR_STICK     "\302\211"
#define TEST_CHAR_POT       "\302\212"
#define TEST_CHAR_SLIDER    "\302\213"
#define TEST_CHAR_SWITCH    "\302\214"
#define TEST_CHAR_TRIM      "\302\215"
#define TEST_CHAR_INPUT     "\302\216"
#define TEST_CHAR_FUNCTION  "\302\217"
#define TEST_CHAR_CYC       "\302\220"
#define TEST_CHAR_TRAINER   "\302\221"
#define TEST_CHAR_CHANNEL   "\302\222"
#define TEST_CHAR_TELEMETRY "\302\223"
#define TEST_CHAR_LUA       "\302\224"

//#define EXTRA_TEST TEST_CHAR_RIGHT TEST_CHAR_LEFT TEST_CHAR_UP TEST_CHAR_DOWN
#define EXTRA_TEST TEST_CHAR_DELTA TEST_CHAR_STICK TEST_CHAR_POT TEST_CHAR_SLIDER \
  TEST_CHAR_SWITCH TEST_CHAR_TRIM TEST_CHAR_INPUT

// TEST(Lcd, Extra_chars)
// {
//   lcdClear();
//   lcdDrawText(0, 10, EXTRA_TEST);
//   EXPECT_TRUE(checkScreenshot("extra_chars"));
// }

#if defined(TRANSLATIONS_FR)
TEST(Lcd, fr_chars)
{
  lcdClear();
  lcdDrawText(0, 10, u8"éèàîç");
  EXPECT_TRUE(checkScreenshot("fr_chars"));
}
#endif

#if defined(PCBTARANIS) && LCD_W >= 212
TEST(Lcd, DrawSwitch)
{
  lcdClear();
  drawSwitch(0,  10, SWSRC_FIRST_SWITCH, 0, false);
  drawSwitch(30, 10, SWSRC_FIRST_SWITCH, SMLSIZE);
  // drawSwitch(60, 10, SWSRC_SA0, MIDSIZE); missing arrows in this font
  drawSwitch(90, 10, SWSRC_FIRST_SWITCH, DBLSIZE);
  EXPECT_TRUE(checkScreenshot("drawswitch"));
}
#endif

#if defined(PCBTARANIS) && LCD_W >= 212
TEST(Lcd, BMPWrapping)
{
  lcdClear();
  uint8_t bitmap[2+40*40/2];
  lcdLoadBitmap(bitmap, TESTS_PATH "/images/bw/plane.bmp", 40, 40);
  lcdDrawBitmap(200, 0, bitmap);
  lcdDrawBitmap(200, 60, bitmap);
  lcdDrawBitmap(240, 60, bitmap);     // x too big
  lcdDrawBitmap(20, 200, bitmap);     // y too big
  EXPECT_TRUE(checkScreenshot("bmpwrapping"));
}
#endif

#if defined(PCBTARANIS) && LCD_W >= 212
TEST(Lcd, lcdDrawHorizontalLine)
{
  lcdClear();
  lcdDrawHorizontalLine(0, 10, LCD_W, DOTTED);
  lcdDrawHorizontalLine(0, 20, LCD_W, SOLID);
  lcdDrawHorizontalLine(50, 30, LCD_W, 0xEE);    //too wide
  lcdDrawHorizontalLine(50, LCD_H + 10, 20, SOLID);    //too low
  lcdDrawHorizontalLine(250, 30, LCD_W, SOLID);    //x outside display
  EXPECT_TRUE(checkScreenshot("lcdDrawHorizontalLine"));
}
#endif

#if defined(PCBTARANIS) && LCD_W >= 212
TEST(Lcd, lcdDrawVerticalLine)
{
  lcdClear();
  lcdDrawVerticalLine(10, 0, LCD_H, DOTTED);
  lcdDrawVerticalLine(20, 0, LCD_H, SOLID);
  lcdDrawVerticalLine(30, 30, LCD_H, 0xEE);    //too high
  lcdDrawVerticalLine(40, LCD_H + 10, 20, SOLID);    //too low
  lcdDrawVerticalLine(250, LCD_H + 10, LCD_H, SOLID);    //x outside display
  EXPECT_TRUE(checkScreenshot("lcdDrawVerticalLine"));
}
#endif

template <int padding> class TestBuffer
{
private:
  uint8_t * buf;
  uint32_t size;
public:
  TestBuffer(uint32_t size) : buf(0), size(size) {
    buf = new uint8_t[size + padding * 2];
    memset(buf, 0xA5, padding);
    memset(buf+padding, 0x00, size);
    memset(buf+padding+size, 0x5A, padding);
  };
  ~TestBuffer() { if (buf) delete[] buf; };
  uint8_t * buffer() { return buf + padding; };
  void leakCheck() const {
    uint8_t paddingCompareBuf[padding];
    memset(paddingCompareBuf, 0xA5, padding);
    if (memcmp(buf, paddingCompareBuf, padding) != 0) {
      ADD_FAILURE() << "buffer leaked low";
    };
    memset(paddingCompareBuf, 0x5A, padding);
    if (memcmp(buf+padding+size, paddingCompareBuf, padding) != 0) {
      ADD_FAILURE() << "buffer leaked high";
    }
  };
};

#if defined(PCBTARANIS) && LCD_W >= 212
TEST(Lcd, lcdDrawBitmapLoadAndDisplay)
{
  lcdClear();
  // Test proper BMP files, they should display correctly
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(7, 32));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/4b_7x32.bmp", 7, 32) != NULL);
    bitmap.leakCheck();
    lcdDrawBitmap(10, 2, bitmap.buffer());
  }
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(6, 32));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/1b_6x32.bmp", 6, 32) != NULL);
    bitmap.leakCheck();
    lcdDrawBitmap(20, 2, bitmap.buffer());
  }
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(31, 31));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/4b_31x31.bmp", 31, 31) != NULL);
    bitmap.leakCheck();
    lcdDrawBitmap(30, 2, bitmap.buffer());
  }
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(39, 32));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/1b_39x32.bmp", 39, 32) != NULL);
    bitmap.leakCheck();
    lcdDrawBitmap(70, 2, bitmap.buffer());
  }
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(20, 20));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/4b_20x20.bmp", 20, 20) != NULL);
    bitmap.leakCheck();
    lcdDrawBitmap(120, 2, bitmap.buffer());
  }
  EXPECT_TRUE(checkScreenshot("lcdDrawBitmapLoadAndDisplay"));

  // Test various bad BMP files, they should not display
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(LCD_W+1, 32));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), "", LCD_W+1, 32) == NULL) << "to wide";
    bitmap.leakCheck();
  }
  {
    TestBuffer<1000>  bitmap(BITMAP_BUFFER_SIZE(10, 10));
    EXPECT_TRUE(lcdLoadBitmap(bitmap.buffer(), TESTS_PATH "/images/bw/1b_39x32.bmp", 10, 10) == NULL) << "to small buffer";
    bitmap.leakCheck();
  }
}
#endif

#if defined(PCBTARANIS)

void drawDiamond(int x, int y, int size)
{
  int x1 = x - size;
  int x2 = x;
  int x3 = x + size;

  int y1 = y - size;
  int y2 = y;
  int y3 = y + size;

  lcdDrawLine( x1, y2, x2, y1, SOLID, FORCE);
  lcdDrawLine( x2, y1, x3, y2, SOLID, FORCE);
  lcdDrawLine( x3, y2, x2, y3, SOLID, FORCE);
  lcdDrawLine( x2, y3, x1, y2, SOLID, FORCE);
}

TEST(Lcd, lcdDrawLine)
{
  int start, length, xOffset;
  uint8_t pattern;

  lcdClear();

  start = 2;
  pattern = SOLID;
  length = 40;
  xOffset = 0;
  lcdDrawLine(start+(length>0?1:-1)+xOffset, start, start+(length>0?1:-1)+xOffset+length, start, pattern, 0);
  lcdDrawLine(start+xOffset, start+(length>0?1:-1), start+xOffset, start+(length>0?1:-1)+length, pattern, 0);

  start = 4;
  pattern = DOTTED;
  length = 40;
  xOffset = 0;
  lcdDrawLine(start+(length>0?1:-1)+xOffset, start, start+(length>0?1:-1)+xOffset+length, start, pattern, 0);
  lcdDrawLine(start+xOffset, start+(length>0?1:-1), start+xOffset, start+(length>0?1:-1)+length, pattern, 0);

  start = 56;
  pattern = SOLID;
  length = -40;
  xOffset = 65;
  lcdDrawLine(start+(length>0?1:-1)+xOffset, start, start+(length>0?1:-1)+xOffset+length, start, pattern, 0);
  lcdDrawLine(start+xOffset, start+(length>0?1:-1), start+xOffset, start+(length>0?1:-1)+length, pattern, 0);

  start = 54;
  pattern = DOTTED;
  length = -40;
  xOffset = 65;
  lcdDrawLine(start+(length>0?1:-1)+xOffset, start, start+(length>0?1:-1)+xOffset+length, start, pattern, 0);
  lcdDrawLine(start+xOffset, start+(length>0?1:-1), start+xOffset, start+(length>0?1:-1)+length, pattern, 0);

  // 45 deg lines
  lcdDrawLine( 25, 30, 35, 30, SOLID, FORCE );
  lcdDrawLine( 30, 25, 30, 35, SOLID, FORCE );

  drawDiamond(30, 30, 10);
  drawDiamond(30, 30, 20);

  // slanted lines
  lcdDrawLine( 60, 10, 100, 10, SOLID, FORCE );
  lcdDrawLine( 60, 10, 100, 20, SOLID, FORCE );
  lcdDrawLine( 60, 10, 100, 30, SOLID, FORCE );
  lcdDrawLine( 60, 10, 100, 40, SOLID, FORCE );
  lcdDrawLine( 60, 10, 100, 50, SOLID, FORCE );

  lcdDrawLine( 60, 10, 100, 50, SOLID, FORCE );
  lcdDrawLine( 60, 10,  90, 50, SOLID, FORCE );
  lcdDrawLine( 60, 10,  80, 50, SOLID, FORCE );
  lcdDrawLine( 60, 10,  70, 50, SOLID, FORCE );
  lcdDrawLine( 60, 10,  60, 50, SOLID, FORCE );

  EXPECT_TRUE(checkScreenshot("lcdDrawLine"));
}
#endif
#endif
