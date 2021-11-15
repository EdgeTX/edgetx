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
#include "libopenui.h"

class FilePreview : public Window
{
  public:
    FilePreview(Window *parent, const rect_t &rect, bool drawCentered = true) :
            Window(parent, rect, NO_SCROLLBAR),
            _drawCentered(drawCentered)
    {
    }
    
    ~FilePreview()
    {
      if (bitmap != nullptr)
        delete bitmap;
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FilePreview";
    }
#endif

    void setFile(const char *filename)
    {
      if (bitmap != nullptr)
        delete bitmap;
      bitmap = nullptr;

      const char *ext = getFileExtension(filename);
      if (ext && isExtensionMatching(ext, BITMAPS_EXT)) {
        bitmap = BitmapBuffer::loadBitmap(filename);
      } else {
        bitmap = nullptr;
      }
      invalidate();
    }

    void paint(BitmapBuffer *dc) override
    {
      dc->setFormat(BMP_RGB565);
      coord_t y = _drawCentered ? parent->getScrollPositionY() + 2 : 2;
      coord_t h = _drawCentered ? MENU_BODY_HEIGHT - 4 : rect.h;
      if (bitmap != nullptr) {
        coord_t bitmapHeight = min<coord_t>(h, bitmap->height());
        coord_t bitmapWidth = min<coord_t>(width(), bitmap->width());
        dc->drawScaledBitmap(bitmap, (width() - bitmapWidth) / 2, 
                             _drawCentered ? y + (h - bitmapHeight) / 2 : 0, 
                             bitmapWidth, bitmapHeight);
      }
    }

  protected:
    BitmapBuffer *bitmap = nullptr;
    bool _drawCentered = true;
};
