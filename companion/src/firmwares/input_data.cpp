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

#include "input_data.h"
#include "radiodataconversionstate.h"
#include "eeprominterface.h"
#include "compounditemmodels.h"

void ExpoData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("INP"), 3);
  cstate.setSubComp(RawSource(SOURCE_TYPE_VIRTUAL_INPUT, chn).toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType) % tr(" (@%1)").arg(cstate.subCompIdx));
  srcRaw.convert(cstate);
  swtch.convert(cstate);
}

bool ExpoData::isEmpty() const
{
  return (chn == 0 && mode == INPUT_MODE_NONE);
}

QString ExpoData::carryTrimToString() const
{
  if (carryTrim == CARRYTRIM_STICK_OFF)
    return tr("OFF");
  else if (carryTrim == CARRYTRIM_DEFAULT) {
      if (srcRaw.isStick())
        return tr("ON");
      else
        return tr("OFF");
  }
  else if (carryTrim < 0 && abs(carryTrim) <= Boards::getBoardCapability(getCurrentBoard(), Board::NumTrims))
    return RawSource(SOURCE_TYPE_TRIM, abs(carryTrim) - 1).toString();
  else
    return CPN_STR_UNKNOWN_ITEM;
}

//  static
AbstractStaticItemModel * ExpoData::carryTrimItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_EXPO_CARRYTRIM);
  ExpoData tmp = ExpoData();

  tmp.srcRaw = RawSource(SOURCE_TYPE_INPUT, 0);

  for (int i = -CARRYTRIM_STICK_OFF; i <= Boards::getBoardCapability(getCurrentBoard(), Board::NumTrims); i++) {
    tmp.carryTrim = -i;
    mdl->appendToItemList(tmp.carryTrimToString(), -i, true, 0, CarryTrimSticksGroup);
  }

  tmp.srcRaw = RawSource(SOURCE_TYPE_NONE, 0);

  for (int i = CARRYTRIM_DEFAULT; i <= Boards::getBoardCapability(getCurrentBoard(), Board::NumTrims); i++) {
    tmp.carryTrim = -i;
    mdl->appendToItemList(tmp.carryTrimToString(), -i, true, 0, CarryTrimNotSticksGroup);
  }

  mdl->loadItemList();
  return mdl;
}
