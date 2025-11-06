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

#include "mixdata.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"

void MixData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("MIX"), 4);
  cstate.setSubComp(RawSource(SOURCE_TYPE_CH, destCh).toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType) % tr(" (@%1)").arg(cstate.subCompIdx));
  srcRaw.convert(cstate);
  swtch.convert(cstate);
  weight.convert(cstate);
  offset.convert(cstate);
  curve.convert(cstate);
}

bool MixData::isEmpty() const
{
  return (destCh == 0);
}

QString MixData::mltpxToString() const
{
  return mltpxToString(mltpx);
}

// static
QString MixData::mltpxToString(int val)
{
  switch (val) {
    case MLTPX_ADD:
      return tr("ADD");
    case MLTPX_MUL:
      return tr("MULTIPLY");
    case MLTPX_REP:
      return tr("REPLACE");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  };
}

// static
AbstractStaticItemModel * MixData::mltpxItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("mixdata.mltpx");

  for (int i = 0; i < MLTPX_COUNT; i++)
    mdl->appendToItemList(mltpxToString(i), i);

  mdl->loadItemList();
  return mdl;
}

QString MixData::mixWarnToString() const
{
  return mixWarnToString(mixWarn);
}

// static
QString MixData::mixWarnToString(int val)
{
  switch(val) {
    case 0:
      return tr("OFF");
    case 1:
      return tr("1 Beep");
    case 2:
      return tr("2 Beeps");
    case 3:
      return tr("3 Beeps");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  };
}

// static
AbstractStaticItemModel * MixData::mixWarnItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("expodata.mixwarn");

  for (int i = 0; i < 4; i++)
    mdl->appendToItemList(mixWarnToString(i), i);

  mdl->loadItemList();
  return mdl;
}

// static
AbstractStaticItemModel * MixData::precisionItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("expodata.precision");

  mdl->appendToItemList("0.0", 0);
  mdl->appendToItemList("0.00", 1);

  mdl->loadItemList();
  return mdl;
}
