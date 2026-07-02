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

#include "gtests.h"

#include <cstring>

#include "firmwares/telem_data.h"

// FrSkyData::clear() is a hand-written field-by-field reset (not a memset),
// so any field it forgets is left uninitialised. varioCenterSilent and
// ignoreSensorIds were both omitted yet are serialised to YAML, which made a
// freshly-cleared model emit non-deterministic values (e.g. centerSilent
// emitted as a stray byte, then masked to 0/1 on reload -> non-idempotent
// round-trip).
//
// Poison the storage first so the test fails deterministically if clear()
// ever stops initialising one of these fields.
TEST(FrSkyDataClear, InitialisesAllSerialisedFields)
{
  FrSkyData fs;
  memset(reinterpret_cast<void*>(&fs), 0x2D, sizeof(fs));  // 0x2D == 45
  fs.clear();

  EXPECT_FALSE(fs.varioCenterSilent)
      << "varioCenterSilent left uninitialised by FrSkyData::clear()";
  EXPECT_FALSE(fs.ignoreSensorIds)
      << "ignoreSensorIds left uninitialised by FrSkyData::clear()";
}
