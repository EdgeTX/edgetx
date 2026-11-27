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
// Copies what LVGL renders into two frame buffers owned by the LCD driver.
//
// LVGL is not in direct mode: it renders into a draw buffer, possibly in
// bands, which can then live in a small and fast memory (e.g. internal SRAM
// instead of SDRAM). Each flushed area is copied into the back buffer (e.g.
// with a DMA2D), possibly rotated. The last flush of a frame swaps the
// buffers on the next vertical blanking. Before the next frame is drawn, the
// areas of the previous one are copied into the other buffer, except for
// what the new frame redraws anyway.
//
// Portrait panels mounted in landscape (e.g. ST7365, ST7796S) can rotate the
// picture themselves (MADCTL MV), but they refresh the glass from their GRAM
// along their own portrait lines, i.e. along the landscape columns, while the
// host writes landscape rows. Both sweeps cross on every frame that changes:
// a diagonal line shows between the old and the new picture. So such panels
// are driven in their native portrait order, and the UI is rotated while
// copying: the CPU transposes strips of each flushed area into tiles, which
// are then copied into portrait frame buffers.
//

// UI pixel (x, y) is shown at frame buffer pixel...
enum LcdRotation : uint8_t {
  LCD_ROTATE_NONE,       // (x, y)
  LCD_ROTATE_180,        // (LCD_W - 1 - x, LCD_H - 1 - y): panel upside down
  // Portrait frame buffers (LCD_H pixels wide, LCD_W lines high), for a
  // portrait panel mounted in landscape (see above):
  LCD_ROTATE_90,         // (y, LCD_W - 1 - x)
  LCD_ROTATE_TRANSPOSE,  // (y, x)
};

// Platform operations
struct LcdFlushOps {
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

// UI columns transposed per tile for the portrait rotations. 16 pixels is
// one 32-byte cache line of each source row.
#define LCD_FLUSH_STRIP_W 16

// Rotated areas are prepared by the CPU in tiles, which are then copied into
// the back buffer. Double buffered: the CPU fills one while the other one is
// being copied.
#define LCD_FLUSH_TILES_SIZE (2 * LCD_FLUSH_STRIP_W * LCD_H)

// Areas flushed into the back buffer per frame before giving up tracking
// them (LVGL may flush an area in several bands)
#define LCD_FLUSH_DIRTY_MAX (2 * LV_INV_BUF_SIZE)

class LcdFlusher
{
 public:
  // 'buf1' and 'buf2': LCD_W x LCD_H pixels, written only by ops->copy().
  // 'tiles': LCD_FLUSH_TILES_SIZE pixels, written by the CPU and read by
  // ops->copy(), only needed when rotating.
  void init(const LcdFlushOps* ops, LcdRotation rotation, pixel_t* buf1,
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
  const LcdFlushOps* ops = nullptr;
  LcdRotation rotation = LCD_ROTATE_NONE;

  pixel_t* front = nullptr;
  pixel_t* back = nullptr;
  pixel_t* tiles[2] = {nullptr, nullptr};
  uint8_t tile_idx = 0;

  // UI areas written into the back buffer since the last swap
  lv_area_t dirty[LCD_FLUSH_DIRTY_MAX];
  uint32_t dirty_n = 0;
  bool dirty_full = false;
  bool sync_pending = false;

  // Areas left to copy into the back buffer after removing what the next
  // frame redraws (each removal splits an area in up to 4)
  static constexpr uint32_t SYNC_MAX = 2 * LCD_FLUSH_DIRTY_MAX;
  lv_area_t sync_areas[2][SYNC_MAX];

  bool portrait() const { return rotation >= LCD_ROTATE_90; }
  pixel_t* nextTile();
  void copyArea(const pixel_t* src, const rect_t& area);
  void copyRotate180(const pixel_t* src, const rect_t& area);
  coord_t firstLine(coord_t x1, coord_t x2) const;
  void transposeStrip(pixel_t* tile, coord_t ts, const pixel_t* src,
                      coord_t w, coord_t h, coord_t n) const;
  void copyTranspose(const pixel_t* src, const rect_t& area);
  void addDirty(const rect_t& area);
  void syncCopy(const lv_area_t& area);
  void syncBackBuffer(const lv_area_t* redrawn, uint32_t redrawn_n);
};
