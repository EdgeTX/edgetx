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
#include "compounditemmodels.h"

void LimitData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(LimitData));
  offset = RawSource(SOURCE_TYPE_NUMBER, 0);
  min = RawSource(SOURCE_TYPE_NUMBER, -1000);
  max = RawSource(SOURCE_TYPE_NUMBER, 1000);
  curve = RawSource();
  ppmCenter = 1500;
}

bool LimitData::isEmpty() const
{
  LimitData tmp;
  return !memcmp(this, &tmp, sizeof(LimitData));
}

QString LimitData::revertToString() const
{
  return revert ? tr("INV") : tr("NOR");
}

QString LimitData::nameToString(int index) const
{
  return RadioData::getElementName(tr("CH"), index + 1, name);
}

QString LimitData::symetricalToString() const
{
  return symetricalToString(symetrical);
}

QString LimitData::symetricalToString(bool value)
{
  return value ? tr("Symetrical") : tr("Center only");
}

void LimitData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("CH"), 9);
  cstate.setSubComp(nameToString(cstate.subCompIdx));
  min = min.convert(cstate.withComponentField("MIN"));
  max = max.convert(cstate.withComponentField("MAX"));
  offset = offset.convert(cstate.withComponentField("SUB-TRIM"));
  curve = curve.convert(cstate.withComponentField("CURVE"));
}

AbstractStaticItemModel * LimitData::symetricalModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("limitdata.symetrical");

  mdl->appendToItemList(symetricalToString(false), 0);
  mdl->appendToItemList(symetricalToString(true), 1);

  mdl->loadItemList();
  return mdl;
}
