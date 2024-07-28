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

void MixData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("MIX"), 4);
  cstate.setSubComp(RawSource(SOURCE_TYPE_CH, destCh-1).toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType) % tr(" (@%1)").arg(cstate.subCompIdx));
  srcRaw.convert(cstate);
  swtch.convert(cstate);
}

bool MixData::isEmpty() const
{
  return (destCh == 0);
}
