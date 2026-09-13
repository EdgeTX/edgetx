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

#include "autoimage.h"

AutoImage::AutoImage(QWidget * parent, const QString & filename, int width, int height, int depth) :
  QLabel(parent),
  AutoWidget(parent)
{
  setup(filename, width, height, depth);
}

AutoImage::~AutoImage()
{
}

void AutoImage::setup(const QString & filename, int width, int height, int depth)
{
  m_filename = filename;
  setDimensions(width, height, depth);
}

void AutoImage::setDimensions(int width, int height, int depth)
{
  m_width = width;
  m_height = height;
  m_depth = depth;
  setFixedSize(m_width, m_height);
}

void AutoImage::updateValue()
{
  setLock(true);
  clear();

  if (!m_filename.isEmpty()) {
    QImage image(m_filename);

    if (!image.isNull()) {
      QImage result = image.scaled(m_width, m_height);

      if (m_depth == 4) {
        result = result.convertToFormat(QImage::Format_RGB32);
        for (int i = 0; i < result.width(); ++i) {
          for (int j = 0; j < result.height(); ++j) {
            QRgb col = result.pixel(i, j);
            int gray = qGray(col);
            result.setPixel(i, j, qRgb(gray, gray, gray));
          }
        }
      } else {
        result = result.convertToFormat(QImage::Format_Mono);
      }

      setPixmap(QPixmap::fromImage(result));
    }
  }

  setLock(false);
}
