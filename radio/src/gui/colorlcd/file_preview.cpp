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

#include "file_preview.h"
#include "sdcard.h"

FilePreview::FilePreview(Window *parent, const rect_t &rect,
                         bool drawCentered) :
    Window(parent, rect, NO_SCROLLBAR), _drawCentered(drawCentered)
{
}

FilePreview::~FilePreview()
{
  if (bitmap != nullptr) delete bitmap;
}

void FilePreview::setFile(const char *filename)
{
  if (bitmap != nullptr) delete bitmap;
  bitmap = nullptr;

  if (filename) {
    const char *ext = vIRTUALfs..getFileExtension(filename);
    if (ext && isExtensionMatching(ext, BITMAPS_EXT)) {
      bitmap = BitmapBuffer::loadBitmap(filename);
    } else {
      bitmap = nullptr;
    }
  }
  invalidate();
}

coord_t FilePreview::getBitmapWidth() const
{
  if (bitmap) return bitmap->width();
  return 0;
}

coord_t FilePreview::getBitmapHeight() const
{
  if (bitmap) return bitmap->height();
  return 0;
}

void FilePreview::paint(BitmapBuffer *dc)
{
  if (!bitmap) return;

  coord_t w = lv_obj_get_content_width(lvobj);
  coord_t h = lv_obj_get_content_height(lvobj);

  coord_t bm_w = min<coord_t>(w, bitmap->width());
  coord_t bm_h = min<coord_t>(h, bitmap->height());

  coord_t border_w = lv_obj_get_style_border_width(lvobj, 0);
  coord_t x = border_w + lv_obj_get_style_pad_left(lvobj, 0);
  coord_t y = border_w + lv_obj_get_style_pad_top(lvobj, 0);

  dc->setFormat(BMP_RGB565);
  dc->drawScaledBitmap(bitmap, x + (w - bm_w) / 2, y + (h - bm_h) / 2, bm_w, bm_h);
}
