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

#include "curvereference.h"
#include "modeldata.h"
#include "generalsettings.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"

CurveReference::CurveReference()
{
  clear();
}

void CurveReference::clear()
{
  type = CURVE_REF_DIFF;
  source = RawSource(SOURCE_TYPE_NUMBER, 0);
}

const QString CurveReference::toString(const ModelData * model, bool verbose, const GeneralSettings * const generalSettings,
                                       Board::Type board, bool prefixCustomName) const
{
  Q_UNUSED(verbose);
  return source.toString(model, generalSettings, board, prefixCustomName);
}

//  static
CurveReference CurveReference::getDefaultValue(const CurveRefType type)
{
  CurveReference cr;
  cr.type = type;
  if (type == CURVE_REF_FUNC)
    cr.source = RawSource(SOURCE_TYPE_CURVE_FUNC, 1);
  else if (type == CURVE_REF_CUSTOM)
    cr.source = RawSource(SOURCE_TYPE_CURVE, 1);
  else
    cr.source = RawSource();

  return cr;
}

//  static
QString CurveReference::typeToString(const int type)
{
  const QStringList strl = { tr("Diff"), tr("Expo") , tr("Func"), tr("Custom") };
  int idx = (int)type;

  if (idx < 0 || idx > MAX_CURVE_REF_COUNT)
    return CPN_STR_UNKNOWN_ITEM;

  return strl.at(idx);
}

//  static
AbstractItemModel *CurveReference::typeItemModel()
{
   AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
   mdl->setName("curvereference.type");

   for (int i = 0; i < MAX_CURVE_REF_COUNT; i++) {
     mdl->appendToItemList(typeToString(i), i);
   }

   mdl->loadItemList();
   return mdl;
}

CurveReference CurveReference::convert(RadioDataConversionState & cstate)
{
  source = source.convert(cstate);
  return *this;
}
