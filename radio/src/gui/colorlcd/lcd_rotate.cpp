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


#include "lcd_rotate.h"

#include <string.h>

#define PORTRAIT_W LCD_H
#define PORTRAIT_H LCD_W

// Source rows prefetched ahead of the transpose
#define PREFETCH_ROWS 8

void LcdRotator::init(const LcdRotateOps* ops, LcdRotation rotation,
                      pixel_t* buf1, pixel_t* buf2, pixel_t* tiles)
{
  this->ops = ops;
  this->rotation = rotation;
  front = buf1;
  back = buf2;
  this->tiles[0] = tiles;
  this->tiles[1] = tiles + LCD_ROTATE_TILES_SIZE / 2;
  tile_idx = 0;
  dirty_n = 0;
  dirty_full = false;
  sync_pending = false;

  memset(buf1, 0, LCD_W * LCD_H * sizeof(pixel_t));
  memset(buf2, 0, LCD_W * LCD_H * sizeof(pixel_t));
  ops->clean(buf1, LCD_W * LCD_H * sizeof(pixel_t));
  ops->clean(buf2, LCD_W * LCD_H * sizeof(pixel_t));
}

// First portrait line of landscape columns x1..x2
coord_t LcdRotator::firstLine(coord_t x1, coord_t x2) const
{
  return rotation == LCD_ROTATE_90 ? PORTRAIT_H - 1 - x2 : x1;
}

// Transpose 'n' landscape columns of 'h' rows (stride 'w') into 'n' tile
// lines of stride 'ts' (even), in portrait line order. Works on 2x2 pixel
// blocks: two 32-bit loads, two packs, two 32-bit stores.
void LcdRotator::transposeStrip(pixel_t* tile, coord_t ts, const pixel_t* src,
                                coord_t w, coord_t h, coord_t n) const
{
  // column i goes to tile line i, or (n - 1 - i) when rotating
  pixel_t* line0 = rotation == LCD_ROTATE_90 ? tile + (n - 1) * ts : tile;
  const coord_t step = rotation == LCD_ROTATE_90 ? -ts : ts;

  coord_t y = 0;
  for (; y + 1 < h; y += 2) {
    const pixel_t* s0 = src + y * w;
    const pixel_t* s1 = s0 + w;
    __builtin_prefetch(s0 + PREFETCH_ROWS * w);
    __builtin_prefetch(s1 + PREFETCH_ROWS * w);

    pixel_t* d = line0 + y;
    coord_t i = 0;
    for (; i + 1 < n; i += 2) {
      uint32_t a, b;
      memcpy(&a, s0 + i, sizeof(a));  // (i, y)   (i+1, y)
      memcpy(&b, s1 + i, sizeof(b));  // (i, y+1) (i+1, y+1)
      uint32_t c0, c1;
#if defined(__ARM_FEATURE_DSP)
      // GCC does not combine the C version below into these
      asm("pkhbt %0, %1, %2, lsl #16" : "=r"(c0) : "r"(a), "r"(b));
      asm("pkhtb %0, %1, %2, asr #16" : "=r"(c1) : "r"(b), "r"(a));
#else
      c0 = (a & 0xFFFF) | (b << 16);
      c1 = (a >> 16) | (b & 0xFFFF0000);
#endif
      memcpy(d, &c0, sizeof(c0));     // column i
      d += step;
      memcpy(d, &c1, sizeof(c1));     // column i + 1
      d += step;
    }
    if (i < n) {
      d[0] = s0[i];
      d[1] = s1[i];
    }
  }

  if (y < h) {
    // odd height: last row on its own
    const pixel_t* s0 = src + y * w;
    pixel_t* d = line0 + y;
    for (coord_t i = 0; i < n; i++) {
      *d = s0[i];
      d += step;
    }
  }
}

void LcdRotator::copyRotate(const pixel_t* src, const rect_t& area)
{
  const coord_t w = area.w;
  const coord_t h = area.h;
  const coord_t ts = (h + 1) & ~1;

  for (coord_t col = 0; col < w; col += LCD_ROTATE_STRIP_W) {
    const coord_t n =
        (w - col < LCD_ROTATE_STRIP_W) ? w - col : LCD_ROTATE_STRIP_W;

    // The other tile may still be copied, but not this one: the copy before
    // the last one was complete when the last one was started.
    pixel_t* tile = tiles[tile_idx];
    tile_idx ^= 1;

    transposeStrip(tile, ts, src + col, w, h, n);
    ops->clean(tile, n * ts * sizeof(pixel_t));

    coord_t line = firstLine(area.x + col, area.x + col + n - 1);
    ops->copy(back + line * PORTRAIT_W + area.y, PORTRAIT_W, tile, ts, h, n);
  }
}

void LcdRotator::addDirty(const rect_t& area)
{
  if (dirty_full) return;
  if (dirty_n < LCD_ROTATE_DIRTY_MAX) {
    dirty[dirty_n++] = {(lv_coord_t)area.x, (lv_coord_t)area.y,
                        (lv_coord_t)(area.x + area.w - 1),
                        (lv_coord_t)(area.y + area.h - 1)};
  } else {
    dirty_full = true;
  }
}

// 'a' minus 'b' as up to 4 non-overlapping areas, or -1 if they do not
// intersect. Unlike _lv_area_diff(), it is exact: the pieces do not overlap
// 'b', and nothing is lost when the intersection is a single line.
static int areaDiff(lv_area_t* res, const lv_area_t& a, const lv_area_t& b)
{
  if (b.x1 > a.x2 || b.x2 < a.x1 || b.y1 > a.y2 || b.y2 < a.y1) return -1;

  int n = 0;
  if (b.y1 > a.y1)  // above
    res[n++] = {a.x1, a.y1, a.x2, (lv_coord_t)(b.y1 - 1)};
  if (b.y2 < a.y2)  // below
    res[n++] = {a.x1, (lv_coord_t)(b.y2 + 1), a.x2, a.y2};

  lv_coord_t y1 = b.y1 > a.y1 ? b.y1 : a.y1;
  lv_coord_t y2 = b.y2 < a.y2 ? b.y2 : a.y2;
  if (b.x1 > a.x1)  // left
    res[n++] = {a.x1, y1, (lv_coord_t)(b.x1 - 1), y2};
  if (b.x2 < a.x2)  // right
    res[n++] = {(lv_coord_t)(b.x2 + 1), y1, a.x2, y2};

  return n;
}

// Copy a landscape area from the front to the back buffer
void LcdRotator::syncCopy(const lv_area_t& area)
{
  uint32_t offset = firstLine(area.x1, area.x2) * PORTRAIT_W + area.y1;
  ops->copy(back + offset, PORTRAIT_W, front + offset, PORTRAIT_W,
            area.y2 - area.y1 + 1, area.x2 - area.x1 + 1);
}

// Bring the back buffer (previous front) up to date with the areas drawn
// during the last frame, except for what the frame being drawn now redraws.
void LcdRotator::syncBackBuffer(const lv_area_t* redrawn, uint32_t redrawn_n)
{
  ops->wait_shown();

  lv_area_t* todo = sync_areas[0];
  lv_area_t* next = sync_areas[1];
  uint32_t todo_n = 0;

  if (dirty_full) {
    todo[todo_n++] = {0, 0, LCD_W - 1, LCD_H - 1};
  } else {
    for (uint32_t i = 0; i < dirty_n; i++) todo[todo_n++] = dirty[i];
  }

  for (uint32_t j = 0; j < redrawn_n && todo_n > 0; j++) {
    uint32_t next_n = 0;
    for (uint32_t i = 0; i < todo_n; i++) {
      lv_area_t res[4];
      int res_n = areaDiff(res, todo[i], redrawn[j]);
      if (res_n < 0 || next_n + res_n > SYNC_MAX) {
        // untouched, or no room to split it: copy it whole
        if (next_n < SYNC_MAX) next[next_n++] = todo[i];
        else syncCopy(todo[i]);
      } else {
        for (int k = 0; k < res_n; k++) next[next_n++] = res[k];
      }
    }

    lv_area_t* tmp = todo;
    todo = next;
    next = tmp;
    todo_n = next_n;
  }

  for (uint32_t i = 0; i < todo_n; i++) syncCopy(todo[i]);

  dirty_n = 0;
  dirty_full = false;
  sync_pending = false;
}

void LcdRotator::flush(const pixel_t* buffer, const rect_t& area, bool last,
                       const lv_area_t* redrawn, uint32_t redrawn_n)
{
  if (sync_pending) syncBackBuffer(redrawn, redrawn_n);

  // LVGL may share the copy engine (e.g. DMA2D fills)
  ops->wait();

  copyRotate(buffer, area);
  addDirty(area);

  if (last) {
    pixel_t* tmp = front;
    front = back;
    back = tmp;

    ops->wait();
    ops->show(front);
    sync_pending = true;
  }
}

void LcdRotator::flush(lv_disp_drv_t* disp_drv, const pixel_t* buffer,
                       const rect_t& area)
{
  // LVGL knows all the areas of a frame when it flushes the first one
  lv_area_t redrawn[LV_INV_BUF_SIZE];
  uint32_t redrawn_n = 0;
  lv_disp_t* disp = _lv_refr_get_disp_refreshing();
  if (sync_pending && disp) {
    for (uint32_t j = 0; j < disp->inv_p; j++) {
      if (!disp->inv_area_joined[j]) redrawn[redrawn_n++] = disp->inv_areas[j];
    }
  }

  flush(buffer, area, lv_disp_flush_is_last(disp_drv), redrawn, redrawn_n);
}
