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

#include "curveimage.h"
#include "curvedata.h"

constexpr int IMAGE_SIZE = { 200 };

CurveImage::CurveImage(QColor gridColor, qreal gridWidth):
  size(IMAGE_SIZE),
  image(size + 1, size + 1, QImage::Format_RGB32),
  painter(&image)
{
  painter.setBrush(QBrush(Qt::white));
  painter.setPen(Qt::black);
  painter.drawRect(0, 0, size, size);

  painter.setPen(QPen(gridColor, gridWidth));
  const int step = size / 4;
  painter.drawLine(0, step * 1, size, step * 1);
  painter.drawLine(0, step * 2, size, step * 2);
  painter.drawLine(0, step * 3, size, step * 3);
  painter.drawLine(step * 1, 0, step * 1, size);
  painter.drawLine(step * 2, 0, step * 2, size);
  painter.drawLine(step * 3, 0, step * 3, size);
}

void CurveImage::drawCurve(const CurveData & curve, QColor color, qreal width)
{
  painter.setPen(QPen(color, width, Qt::SolidLine));

  for (int j = 1; j < curve.count; j++) {
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM)
      painter.drawLine(size / 2 + (size * curve.points[j - 1].x) / IMAGE_SIZE,
                       size / 2 - (size * curve.points[j - 1].y) / IMAGE_SIZE,
                       size / 2 + (size * curve.points[j].x) / IMAGE_SIZE,
                       size / 2 - (size * curve.points[j].y) / IMAGE_SIZE);
    else
      painter.drawLine(size * (j-1) / (curve.count - 1),
                       size / 2 - (size * curve.points[j - 1].y) / IMAGE_SIZE,
                       size * (j) / (curve.count - 1),
                       size / 2 - (size * curve.points[j].y) / IMAGE_SIZE);
  }
}
