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


#pragma once

#include <stdint.h>

#include "edgetx_types.h"
#include <lvgl/lvgl.h>

//
// Landscape UI (LCD_W x LCD_H) on a portrait panel (LCD_H x LCD_W) that
// cannot rotate the picture without tearing.
//
// Such panels (e.g. ST7365, ST7796S) can rotate the picture themselves
// (MADCTL MV), but they refresh the glass from their GRAM along their own
// portrait lines, i.e. along the landscape columns, while the host writes
// landscape rows. Both sweeps cross on every frame that changes: a diagonal
// line shows between the old and the new picture. So the panel is driven in
// its native portrait order, and each area LVGL flushes is rotated into a
// portrait frame buffer.
//
// LVGL is not in direct mode: it renders into a (preferably internal and
// fast) draw buffer, possibly in bands. The CPU transposes strips of each
// flushed area into tiles, which the platform copies into the back buffer
// (e.g. with a DMA2D). The last flush of a frame swaps the buffers on the
// next vertical blanking. Before the next frame is drawn, the areas of the
// previous one are copied into the other buffer, except for what the new
// frame redraws anyway.
//

// Landscape (x, y) is shown at portrait pixel y of line...
enum LcdRotation : uint8_t {
  LCD_ROTATE_90,         // LCD_W - 1 - x
  LCD_ROTATE_TRANSPOSE,  // x
};

// Platform operations
struct LcdRotateOps {
  // Start a 2D copy of w x h pixels (strides in pixels). It may be called
  // before the previous copy is complete.
  void (*copy)(pixel_t* dst, uint32_t dst_stride, const pixel_t* src,
               uint32_t src_stride, uint32_t w, uint32_t h);
  // Wait for all copies to complete
  void (*wait)();
  // Make the CPU writes to [p, p + len) visible to copy()
  void (*clean)(const void* p, uint32_t len);
  // Show 'buffer' from the next vertical blanking on
  void (*show)(pixel_t* buffer);
  // Wait until the buffer passed to show() is shown
  void (*wait_shown)();
};

// Landscape columns transposed per tile. 16 pixels is one 32-byte cache line
// of each source row.
#define LCD_ROTATE_STRIP_W 16

// Pixels needed for the tiles (double buffered: the CPU fills one while the
// other is being copied)
#define LCD_ROTATE_TILES_SIZE (2 * LCD_ROTATE_STRIP_W * LCD_H)

// Landscape areas flushed into the back buffer per frame before giving up
// tracking them (LVGL may flush an area in several bands)
#define LCD_ROTATE_DIRTY_MAX (2 * LV_INV_BUF_SIZE)

class LcdRotator
{
 public:
  // 'buf1' and 'buf2': LCD_W x LCD_H pixels, written only by ops->copy().
  // 'tiles': LCD_ROTATE_TILES_SIZE pixels, written by the CPU and read by
  // ops->copy().
  void init(const LcdRotateOps* ops, LcdRotation rotation, pixel_t* buf1,
            pixel_t* buf2, pixel_t* tiles);

  pixel_t* frontBuffer() const { return front; }

  // LVGL flush callback
  void flush(lv_disp_drv_t* disp_drv, const pixel_t* buffer,
             const rect_t& area);

  // 'buffer' holds 'area' only (stride area.w). 'redrawn': all the areas of
  // the frame this flush is part of.
  void flush(const pixel_t* buffer, const rect_t& area, bool last,
             const lv_area_t* redrawn, uint32_t redrawn_n);

 protected:
  const LcdRotateOps* ops = nullptr;
  LcdRotation rotation = LCD_ROTATE_90;

  pixel_t* front = nullptr;
  pixel_t* back = nullptr;
  pixel_t* tiles[2] = {nullptr, nullptr};
  uint8_t tile_idx = 0;

  // Landscape areas written into the back buffer since the last swap
  lv_area_t dirty[LCD_ROTATE_DIRTY_MAX];
  uint32_t dirty_n = 0;
  bool dirty_full = false;
  bool sync_pending = false;

  // Areas left to copy into the back buffer after removing what the next
  // frame redraws (each removal splits an area in up to 4)
  static constexpr uint32_t SYNC_MAX = 2 * LCD_ROTATE_DIRTY_MAX;
  lv_area_t sync_areas[2][SYNC_MAX];

  coord_t firstLine(coord_t x1, coord_t x2) const;
  void transposeStrip(pixel_t* tile, coord_t ts, const pixel_t* src,
                      coord_t w, coord_t h, coord_t n) const;
  void copyRotate(const pixel_t* src, const rect_t& area);
  void addDirty(const rect_t& area);
  void syncCopy(const lv_area_t& area);
  void syncBackBuffer(const lv_area_t* redrawn, uint32_t redrawn_n);
};
