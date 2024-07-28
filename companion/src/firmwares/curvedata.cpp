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

#include "curvedata.h"
#include "datahelpers.h"
#include "compounditemmodels.h"
#include "eeprominterface.h"

CurveData::CurveData()
{
  clear();
}

void CurveData::clear(int count)
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(CurveData));
  this->count = count;
}

void CurveData::init()
{
  memset(points, 0, sizeof(CurvePoint) * CPN_MAX_POINTS);

  float incr = 200.0 / (float)(count - 1);

  for (int i = 1; i < (count - 1); i++) {
    points[i].x = -100 + (int)(incr * (float)i);
    points[i].y = points[i].x;
  }

  points[0].x = -100;
  points[0].y = -100;
  points[count - 1].x = 100;
  points[count - 1].y = 100;
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

QString CurveData::typeToString() const
{
  return typeToString(type);
}

// static
QString CurveData::typeToString(int value)
{
  switch (value) {
    case CURVE_TYPE_STANDARD:
      return tr("Standard");
    case CURVE_TYPE_CUSTOM:
      return tr("Custom");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString CurveData::pointsToString() const
{
  QString result = "[";

  if (type == CURVE_TYPE_CUSTOM) {
    for (int j = 0; j < count; j++) {
      if (j != 0)
        result.append(", ");
      result.append(QString("(%1, %2)").arg(points[j].x).arg(points[j].y));
    }
  }
  else {
    for (int j = 0; j < count; j++) {
      if (j != 0)
        result.append(", ");
      result.append(QString("%1").arg(points[j].y));
    }
  }

  result.append("]");
  return result;
}

//  static
AbstractStaticItemModel * CurveData::typeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("curvedata.type");

  for (int i = 0; i <= CURVE_TYPE_LAST; i++) {
    mdl->appendToItemList(typeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * CurveData::pointsItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("curvedata.points");

  for (int i = 2; i <= CPN_MAX_POINTS; i++) {
    mdl->appendToItemList(tr("%1 points").arg(i), i);
  }

  mdl->loadItemList();
  return mdl;
}
