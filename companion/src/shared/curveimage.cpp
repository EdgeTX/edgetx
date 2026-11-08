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

constexpr int IMAGE_SIZE = { 200 }; // image side length in pixels
constexpr int CURVE_MAX = 100;      // maximum absolute value of a coordinate on the curve
constexpr int CURVE_RANGE = 2 * CURVE_MAX; // full range from -CURVE_MAX to +CURVE_MAX

// see radio/src/curves.cpp: CUSTOM_POINT_X
static int customPointX(const CurveData & curve, int i)
{
  return i == 0 ? -CURVE_MAX : (i == curve.count - 1 ? CURVE_MAX : curve.points[i].x);
}

// Match the radio's smooth curves using monotone cubic tangents and Hermite interpolation.
// Companion uses floating-point arithmetic instead of the radio's fixed-point calculations.
// see radio/src/curves.cpp: compute_tangent()
static double compute_tangent(const CurveData & crv, int i)
{
  double m = 0;
  int num_points = crv.count;
  if (i == 0) {
    if (crv.type == CurveData::CURVE_TYPE_CUSTOM) {
      int x0 = customPointX(crv, 0);
      int x1 = customPointX(crv, 1);
      if (x1 > x0) m = double(crv.points[1].y - crv.points[0].y) / (x1 - x0);
    }
    else {
      double delta = double(CURVE_RANGE) / (num_points - 1);
      m = (crv.points[1].y - crv.points[0].y) / delta;
    }
  }
  else if (i == num_points - 1) {
    if (crv.type == CurveData::CURVE_TYPE_CUSTOM) {
      int x0 = customPointX(crv, num_points - 2);
      int x1 = customPointX(crv, num_points - 1);
      if (x1 > x0)
        m = double(crv.points[num_points - 1].y - crv.points[num_points - 2].y) / (x1 - x0);
    }
    else {
      double delta = double(CURVE_RANGE) / (num_points - 1);
      m = (crv.points[num_points - 1].y - crv.points[num_points - 2].y) / delta;
    }
  }
  else {
    double d0 = 0, d1 = 0;
    if (crv.type == CurveData::CURVE_TYPE_CUSTOM) {
      int x0 = customPointX(crv, i - 1);
      int x1 = customPointX(crv, i);
      int x2 = customPointX(crv, i + 1);
      if (x1 > x0) d0 = double(crv.points[i].y - crv.points[i - 1].y) / (x1 - x0);
      if (x2 > x1) d1 = double(crv.points[i + 1].y - crv.points[i].y) / (x2 - x1);
    }
    else {
      double delta = double(CURVE_RANGE) / (num_points - 1);
      d0 = (crv.points[i].y - crv.points[i - 1].y) / delta;
      d1 = (crv.points[i + 1].y - crv.points[i].y) / delta;
    }
    m = (d0 + d1) / 2;
    if (d0 == 0 || d1 == 0 || (d0 > 0 && d1 < 0) || (d0 < 0 && d1 > 0)) {
      m = 0;
    }
    else if (m / d0 > 3) {
      m = 3 * d0;
    }
    else if (m / d1 > 3) {
      m = 3 * d1;
    }
  }
  return m;
}

// see radio/src/curves.cpp: hermite_spline()
static double hermite_spline(double x, const CurveData & crv)
{
  int count = crv.count;
  bool custom = (crv.type == CurveData::CURVE_TYPE_CUSTOM);

  if (x < -CURVE_MAX)
    x = -CURVE_MAX;
  else if (x > CURVE_MAX)
    x = CURVE_MAX;

  for (int i = 0; i < count - 1; i++) {
    double p0x, p3x;
    if (custom) {
      p0x = customPointX(crv, i);
      p3x = customPointX(crv, i + 1);
    }
    else {
      p0x = -CURVE_MAX + i * double(CURVE_RANGE) / (count - 1);
      p3x = -CURVE_MAX + (i + 1) * double(CURVE_RANGE) / (count - 1);
    }

    if (x >= p0x && x <= p3x) {
      double p0y = crv.points[i].y;
      double p3y = crv.points[i + 1].y;
      double m0 = compute_tangent(crv, i);
      double m3 = compute_tangent(crv, i + 1);
      double y;
      double h = p3x - p0x;
      double t = h > 0 ? (x - p0x) / h : 0;
      double t2 = t * t;
      double t3 = t2 * t;
      double h00 = 2 * t3 - 3 * t2 + 1;
      double h10 = t3 - 2 * t2 + t;
      double h01 = -2 * t3 + 3 * t2;
      double h11 = t3 - t2;
      y = p0y * h00 + h * m0 * h10 + p3y * h01 + h * m3 * h11;
      return y;
    }
  }
  return 0;
}

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

  if (curve.smooth && curve.count >= 2) {
    // "smooth" curve
    QPolygonF line;
    for (int pixel = 0; pixel <= size; ++pixel) {
      double x = -CURVE_MAX + pixel * double(CURVE_RANGE) / size;
      line << QPointF(pixel, size / 2.0 - hermite_spline(x, curve) * size / CURVE_RANGE);
    }
    painter.drawPolyline(line);
    return;
  }

  // polygonal curve
  for (int j = 1; j < curve.count; j++) {
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM)
      painter.drawLine(size / 2 + (size * curve.points[j - 1].x) / CURVE_RANGE,
                       size / 2 - (size * curve.points[j - 1].y) / CURVE_RANGE,
                       size / 2 + (size * curve.points[j].x) / CURVE_RANGE,
                       size / 2 - (size * curve.points[j].y) / CURVE_RANGE);
    else
      painter.drawLine(size * (j-1) / (curve.count - 1),
                       size / 2 - (size * curve.points[j - 1].y) / CURVE_RANGE,
                       size * (j) / (curve.count - 1),
                       size / 2 - (size * curve.points[j].y) / CURVE_RANGE);
  }
}
