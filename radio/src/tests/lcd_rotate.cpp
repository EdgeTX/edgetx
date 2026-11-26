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


#if defined(COLORLCD)

#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "lcd_rotate.h"

namespace
{

// Copy engine that completes a copy only when the next one starts, or on
// wait(), like the DMA2D: reusing a source too early corrupts the result.
struct PendingCopy {
  pixel_t* dst;
  uint32_t dst_stride;
  const pixel_t* src;
  uint32_t src_stride;
  uint32_t w, h;
};

bool copy_pending = false;
PendingCopy pending;
pixel_t* shown = nullptr;

void runPending()
{
  if (!copy_pending) return;
  for (uint32_t l = 0; l < pending.h; l++) {
    for (uint32_t i = 0; i < pending.w; i++) {
      pending.dst[l * pending.dst_stride + i] =
          pending.src[l * pending.src_stride + i];
    }
  }
  copy_pending = false;
}

void fakeCopy(pixel_t* dst, uint32_t dst_stride, const pixel_t* src,
              uint32_t src_stride, uint32_t w, uint32_t h)
{
  runPending();
  ASSERT_GE(dst_stride, w);
  ASSERT_GE(src_stride, w);
  pending = {dst, dst_stride, src, src_stride, w, h};
  copy_pending = true;
}

void fakeWait() { runPending(); }
void fakeClean(const void*, uint32_t) {}
void fakeShow(pixel_t* buffer) { shown = buffer; }
void fakeWaitShown() {}

const LcdRotateOps fake_ops = {fakeCopy, fakeWait, fakeClean, fakeShow,
                               fakeWaitShown};

class LcdRotateTest : public ::testing::TestWithParam<LcdRotation>
{
 protected:
  std::vector<pixel_t> buf1, buf2, tiles;
  std::vector<pixel_t> screen;  // landscape picture LVGL has drawn
  LcdRotator rotator;
  std::mt19937 rng{1234};
  pixel_t next_color = 1;

  void SetUp() override
  {
    buf1.assign(LCD_W * LCD_H, 0xDEAD);
    buf2.assign(LCD_W * LCD_H, 0xBEEF);
    tiles.assign(LCD_ROTATE_TILES_SIZE, 0);
    screen.assign(LCD_W * LCD_H, 0);
    copy_pending = false;
    shown = nullptr;
    rotator.init(&fake_ops, GetParam(), buf1.data(), buf2.data(),
                 tiles.data());
  }

  int rnd(int lo, int hi)
  {
    return std::uniform_int_distribution<int>(lo, hi)(rng);
  }

  lv_area_t randomArea()
  {
    if (rnd(0, 9) == 0) return {0, 0, LCD_W - 1, LCD_H - 1};
    lv_coord_t x1 = rnd(0, LCD_W - 1), y1 = rnd(0, LCD_H - 1);
    lv_coord_t x2 = rnd(x1, LCD_W - 1), y2 = rnd(y1, LCD_H - 1);
    return {x1, y1, x2, y2};
  }

  // Draws a frame of 'areas' like LVGL does: each area in bands of up to
  // 'band_h' lines, from a buffer holding only the band.
  void drawFrame(const std::vector<lv_area_t>& areas, coord_t band_h)
  {
    std::vector<pixel_t> band;
    for (size_t a = 0; a < areas.size(); a++) {
      const lv_area_t& area = areas[a];
      coord_t w = area.x2 - area.x1 + 1;
      for (coord_t y = area.y1; y <= area.y2; y += band_h) {
        coord_t h = std::min<coord_t>(band_h, area.y2 - y + 1);
        band.resize(w * h);
        for (coord_t j = 0; j < h; j++) {
          for (coord_t i = 0; i < w; i++) {
            pixel_t c = next_color++;
            if (!next_color) next_color = 1;
            screen[(y + j) * LCD_W + area.x1 + i] = c;
            band[j * w + i] = c;
          }
        }
        bool last = (a == areas.size() - 1) && (y + h > area.y2);
        rotator.flush(band.data(), {area.x1, y, w, h}, last, areas.data(),
                      areas.size());
      }
    }
  }

  void checkShown(int frame)
  {
    ASSERT_EQ(shown, rotator.frontBuffer());
    ASSERT_FALSE(copy_pending);
    for (coord_t y = 0; y < LCD_H; y++) {
      for (coord_t x = 0; x < LCD_W; x++) {
        coord_t line = GetParam() == LCD_ROTATE_90 ? LCD_W - 1 - x : x;
        ASSERT_EQ(shown[line * LCD_H + y], screen[y * LCD_W + x])
            << "frame " << frame << " at landscape (" << x << ", " << y
            << ")";
      }
    }
  }
};

}  // namespace

TEST_P(LcdRotateTest, FullFrame)
{
  for (int frame = 0; frame < 4; frame++) {
    drawFrame({{0, 0, LCD_W - 1, LCD_H - 1}}, LCD_H / 2);
    checkShown(frame);
  }
}

TEST_P(LcdRotateTest, SmallAreasKeepTheRest)
{
  drawFrame({{0, 0, LCD_W - 1, LCD_H - 1}}, LCD_H);
  checkShown(0);
  // single pixels, odd sizes, and a strip wider than a tile
  drawFrame({{5, 7, 5, 7}}, LCD_H);
  checkShown(1);
  drawFrame({{1, 2, 18, 4}, {LCD_W - 3, LCD_H - 2, LCD_W - 1, LCD_H - 1}},
            LCD_H);
  checkShown(2);
  drawFrame({{100, 10, 140, 200}}, 7);
  checkShown(3);
  drawFrame({{0, 0, 0, LCD_H - 1}}, 1);
  checkShown(4);
}

TEST_P(LcdRotateTest, RandomFrames)
{
  for (int frame = 0; frame < 300; frame++) {
    std::vector<lv_area_t> areas(rnd(1, LV_INV_BUF_SIZE));
    for (auto& area : areas) area = randomArea();
    // more bands than LCD_ROTATE_DIRTY_MAX from time to time
    coord_t band_h = rnd(0, 3) ? LCD_H / 2 : rnd(1, 16);
    drawFrame(areas, band_h);
    checkShown(frame);
  }
}

INSTANTIATE_TEST_SUITE_P(Rotation, LcdRotateTest,
                         ::testing::Values(LCD_ROTATE_90,
                                           LCD_ROTATE_TRANSPOSE));

#endif
