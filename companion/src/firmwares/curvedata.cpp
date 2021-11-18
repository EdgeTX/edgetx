/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "curvedata.h"
#include "datahelpers.h"

CurveData::CurveData()
{
  clear();
}

void CurveData::clear(int count)
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(CurveData));
  this->count = count;

  int incr = 200 / (count - 1);

  //  points[0].x must be -100
  for (int i = 0; i < count; i++) {
    if (i < count - 1)
      points[i].x = -100 + incr * i;
    else
      points[i].x = 100;    // points[last].x must be +100 do not risk a rounding issue using incr

    points[i].y = points[i].x;
  }
}

bool CurveData::isEmpty() const
{
  CurveData tmp;
  return !memcmp(this, &tmp, sizeof(CurveData));
}

QString CurveData::nameToString(const int idx) const
{
  return DataHelpers::getElementName(tr("CV"), idx + 1, name);
}
