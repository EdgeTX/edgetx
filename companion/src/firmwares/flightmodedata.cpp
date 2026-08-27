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

#include "flightmodedata.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"
#include "eeprominterface.h"

void FlightModeData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent("FMD", 2);
  cstate.setSubComp(nameToString(cstate.subCompIdx));
  swtch.convert(cstate);
}

void FlightModeData::clear(const int phaseIdx)
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(FlightModeData));

  for (int i = 0; i < CPN_MAX_GVARS; i++) {
    gvars[i] = linkedGVarFlightModeZero(phaseIdx);
  }
}

QString FlightModeData::nameToString(int phaseIdx) const
{
  return RadioData::getElementName(tr("%1M").arg(modeAbbrev()), phaseIdx, name);  // names are zero-based, FM0, FM1, etc
}

// static
QString FlightModeData::modeAbbrev()
{
  return Boards::isAir() ? tr("F") : tr("D");
}

// static
QString FlightModeData::modeName()
{
  return Boards::isAir() ? tr("Flight") : tr("Drive");
}

bool FlightModeData::isEmpty(int phaseIdx) const
{
  if (name[0] != '\0' || swtch.isSet() || fadeIn != 0 || fadeOut != 0)
    return false;

  for (int i = 0; i < CPN_MAX_TRIMS; i++) {
    if (trim[i] != 0 || trimRef[i] != 0 || trimMode[i] != 0)
      return false;
  }

  for (int i = 0; i < CPN_MAX_GVARS; i++) {
    if (!isGVarEmpty(phaseIdx, i))
      return false;
  }

  return true;
}

bool FlightModeData::isGVarEmpty(int phaseIdx, int gvIdx) const
{
  if ((phaseIdx == 0 && gvars[gvIdx] == 0) || (phaseIdx != 0 && gvars[gvIdx] == linkedGVarFlightModeZero(phaseIdx)))
    return true;

  return false;
}

int FlightModeData::linkedFlightModeZero(int phaseIdx, int maxOwnValue) const
{
  if (phaseIdx == 0)
    return 0;

  return maxOwnValue + 1;
}

int FlightModeData::linkedGVarFlightModeZero(int phaseIdx) const
{
  return linkedFlightModeZero(phaseIdx, GVAR_MAX_VALUE);
}

// static
AbstractStaticItemModel * FlightModeData::gvarModesItemModel(int excludeMode)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_FLIGHTMODEDATA_GVARFM);
  int modes = getCurrentFirmware()->getCapability(FlightModes);

  for (int i = 0; i < modes; i++) {
    if (i != excludeMode)
      mdl->appendToItemList(tr("%1M%2").arg(modeAbbrev()).arg(i),
          GVAR_MAX_VALUE + 1 + i - (i > excludeMode ? 1 : 0));
  }

  mdl->loadItemList();
  return mdl;
}
