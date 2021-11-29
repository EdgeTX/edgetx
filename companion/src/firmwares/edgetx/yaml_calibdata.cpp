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

#include "yaml_calibdata.h"

// TODO: we need a function that
// does the opposite of Boards::getAnalogInputName()
//
// This is only valid for X10, T16, TX16S (not X12S)
//
const YamlLookupTable calibIdxLut = {
  {  0, "Rud"  },
  {  1, "Ele"  },
  {  2, "Thr"  },
  {  3, "Ail"  },
  {  4, "S1"  },
  {  5, "6POS"  },
  {  6, "S2"  },
  {  7, "EXT1"  },
  {  8, "EXT2"  },
  // not yet supported in Companion:
  // {  9, "EXT3"  },
  // {  10, "EXT4"  },
  {  9, "LS"  },
  {  10, "RS"  },
  {  11, "MOUSE1"  },
  {  12, "MOUSE2"  },
};

YamlCalibData::YamlCalibData() { memset(calib, 0, sizeof(calib)); }

void YamlCalibData::copy(int* calibMid, int* calibSpanNeg,
                         int* calibSpanPos) const
{
  for (int i = 0; i < CPN_MAX_ANALOGS; i++) {
    calibMid[i] = calib[i].mid;
    calibSpanNeg[i] = calib[i].spanNeg;
    calibSpanPos[i] = calib[i].spanPos;
  }
}
