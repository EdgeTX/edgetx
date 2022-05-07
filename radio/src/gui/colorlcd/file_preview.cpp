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

  const char *ext = getFileExtension(filename);
  if (ext && isExtensionMatching(ext, BITMAPS_EXT)) {
    bitmap = BitmapBuffer::loadBitmap(filename);
  } else {
    bitmap = nullptr;
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
  dc->setFormat(BMP_RGB565);
  coord_t y = /*_drawCentered ? parent->getScrollPositionY() + 2 :*/ 2;
  coord_t h = _drawCentered ? MENU_BODY_HEIGHT - 4 : rect.h;
  if (bitmap != nullptr) {
    coord_t bitmapHeight = min<coord_t>(h, bitmap->height());
    coord_t bitmapWidth = min<coord_t>(width(), bitmap->width());
    dc->drawScaledBitmap(bitmap, (width() - bitmapWidth) / 2,
                         _drawCentered ? y + (h - bitmapHeight) / 2 : 0,
                         bitmapWidth, bitmapHeight);
  }
}
