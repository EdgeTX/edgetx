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

#include "output_data.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"

void LimitData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(LimitData));
  min = -1000;
  max = +1000;
}

bool LimitData::isEmpty() const
{
  LimitData tmp;
  return !memcmp(this, &tmp, sizeof(LimitData));
}

QString LimitData::minToString() const
{
  return QString::number((qreal)min / 10);
}

QString LimitData::maxToString() const
{
  return QString::number((qreal)max / 10);
}

QString LimitData::revertToString() const
{
  return revert ? tr("INV") : tr("NOR");
}

QString LimitData::nameToString(int index) const
{
  return RadioData::getElementName(tr("CH"), index + 1, name);
}

QString LimitData::offsetToString() const
{
  return QString::number((qreal)offset / 10, 'f', 1);
}
