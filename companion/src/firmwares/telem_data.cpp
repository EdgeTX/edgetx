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

#include "telem_data.h"

#include "eeprominterface.h"  // for getCurrentBoard()

float FrSkyChannelData::getRatio() const
{
  if (type==0 || type==1 || type==2)
    return float(ratio << multiplier) / 10.0;
  else
    return ratio << multiplier;
}

RawSourceRange FrSkyChannelData::getRange() const
{
  RawSourceRange result;
  float ratio = getRatio();
  if (type==0 || type==1 || type==2)
    result.decimals = 2;
  else
    result.decimals = 0;
  result.step = ratio / 255;
  result.min = offset * result.step;
  result.max = ratio + result.min;
  result.unit = tr("V");
  return result;
}

void FrSkyScreenData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(FrSkyScreenData));
}

void FrSkyData::clear()
{
  usrProto = 0;
  voltsSource = 0;
  altitudeSource = 0;
  currentSource = 0;
  varioMin = 0;
  varioCenterMin = 0;    // if increment in 0.2m/s = 3.0m/s max
  varioCenterMax = 0;
  varioMax = 0;
  mAhPersistent = 0;
  storedMah = 0;
  fasOffset = 0;
  for (int i=0; i<4; i++)
    screens[i].clear();
  varioSource = 0;
  blades = 2;
}

