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

#pragma once

#if !defined(BOOT)
#include "datastructs_private.h"

#if !defined(BACKUP)
/* Compile time check to test structure size has not changed *
   Changing the size of one of the eeprom structs may cause wrong data to
   be loaded. Error out if the struct size changes.
   This function tries not avoid checking or using the defines
   other than the CPU arch and board type so changes in other
   defines also trigger the struct size changes */

#include "chksize.h"

#define CHKSIZE(x, y) check_size<struct x, y>()
#define CHKTYPE(x, y) check_size<x, y>()

static inline void check_struct()
{
  CHKSIZE(CurveRef, 2);
  CHKSIZE(VarioData, 5);
  CHKSIZE(MixData, 20);
  CHKSIZE(ExpoData, 18);
  CHKSIZE(SwashRingData, 8);
  CHKSIZE(CurveHeader, 4);
  CHKSIZE(LogicalSwitchData, 9);
  CHKSIZE(TelemetrySensor, 14);
  CHKSIZE(ModuleData, 29);
  CHKSIZE(GVarData, 7);
  CHKSIZE(RFAlarmData, 2);
  CHKSIZE(TrainerData, 16);
  CHKSIZE(FlightModeData, 22 + 2 * MAX_TRIMS + LEN_FLIGHT_MODE_NAME);
  CHKSIZE(CustomFunctionData, 11);

#if defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
  CHKSIZE(LimitData, 11);
  CHKSIZE(TimerData, 12);
  CHKSIZE(FrSkyBarData, 6);
  CHKSIZE(FrSkyLineData, 4);
  CHKTYPE(TelemetryScreenData, 24);
  CHKSIZE(ModelHeader, 12);
#elif defined(PCBTARANIS)
  CHKSIZE(LimitData, 13);
  CHKSIZE(TimerData, 17);
  CHKSIZE(FrSkyBarData, 6);
  CHKSIZE(FrSkyLineData, 6);
  CHKTYPE(TelemetryScreenData, 24);
  CHKSIZE(ModelHeader, 24);
#elif defined(COLORLCD)
  CHKSIZE(LimitData, 13);
  CHKSIZE(TimerData, 17);
  CHKSIZE(ModelHeader, 131);
  CHKSIZE(CustomScreenData, 1892);
  #if defined(PCBNV14)
    CHKTYPE(TopBarPersistentData, 704);
  #else
    CHKTYPE(TopBarPersistentData, 1048);
  #endif
#else
  #error CHKSIZE not set up
#endif

#if defined(PCBXLITES)
  CHKSIZE(RadioData, 872);
#elif defined(COLORLCD)
  CHKSIZE(RadioData, 966);
#else
  CHKSIZE(RadioData, 870);
#endif

#if defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_BUMBLEBEE)
  CHKSIZE(ModelData, 6354);
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  CHKSIZE(ModelData, 6329);
#elif defined(RADIO_FAMILY_T20)
  CHKSIZE(ModelData, 6390);
#elif defined(RADIO_GX12)
  CHKSIZE(ModelData, 6390);
#elif defined(PCBX9E)
  CHKSIZE(ModelData, 6771);
#elif defined(PCBX9D) || defined(PCBX9DP)
  CHKSIZE(ModelData, 6770);
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
  CHKSIZE(ModelData, 6329);
#elif defined(PCBNV14)
  CHKSIZE(ModelData, 26463);
#elif defined(PCBPL18)
  CHKSIZE(ModelData, 26845);
#elif defined(RADIO_T15)
  CHKSIZE(ModelData, 26834);
#elif defined(PCBHORUS)
  CHKSIZE(ModelData, 26809);
#else
  #error CHKSIZE not set up
#endif

#undef CHKSIZE
}
#endif /* BACKUP */
#endif /* !BOOT */
