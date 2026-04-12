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
  CHKSIZE(CurveRef, 6);
  CHKSIZE(VarioData, 5);
  CHKSIZE(MixData, 35);
  CHKSIZE(ExpoData, 33);
  CHKSIZE(SwashRingData, 17);
  CHKSIZE(CurveHeader, 4);
  CHKSIZE(LogicalSwitchData, 18);
  CHKSIZE(TelemetrySensor, 15);
  CHKSIZE(ModuleData, 29);
  CHKSIZE(GVarData, 7);
  CHKSIZE(RFAlarmData, 2);
  CHKSIZE(TrainerData, 16);
  CHKSIZE(FlightModeData, 6 + 2 * MAX_TRIMS + LEN_FLIGHT_MODE_NAME);
  CHKSIZE(CustomFunctionData, 16);

#if defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
  CHKSIZE(LimitData, 13);
  CHKSIZE(TimerData, 15);
  CHKSIZE(FrSkyBarData, 8);
  CHKSIZE(FrSkyLineData, 8);
  CHKTYPE(TelemetryScreenData, 32);
  CHKSIZE(ModelHeader, 12);
#elif defined(PCBTARANIS)
  CHKSIZE(LimitData, 15);
  CHKSIZE(TimerData, 20);
  CHKSIZE(FrSkyBarData, 8);
  CHKSIZE(FrSkyLineData, 12);
  CHKTYPE(TelemetryScreenData, 48);
  CHKSIZE(ModelHeader, 24);
#elif defined(COLORLCD)
  CHKSIZE(LimitData, 15);
  CHKSIZE(TimerData, 20);
  CHKSIZE(ModelHeader, 131);
#else
  #error CHKSIZE not set up
#endif

#if defined(PCBXLITES)
  CHKSIZE(RadioData, 252);
#elif defined(RADIO_ST16) || defined(PCBPA01) || defined(RADIO_TX15) || defined(RADIO_T15PRO) || defined(RADIO_TX16SMK3)
  CHKSIZE(RadioData, 483);
#elif defined(COLORLCD)
  CHKSIZE(RadioData, 363);
#elif defined(RADIO_GX12)
  CHKSIZE(RadioData, 370);
#else
  CHKSIZE(RadioData, 250);
#endif

#if defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_BUMBLEBEE)
  CHKSIZE(ModelData, 1757);
#elif defined(RADIO_FAMILY_T20)
  CHKSIZE(ModelData, 1757);
#elif defined(RADIO_GX12)
  CHKSIZE(ModelData, 1821);
#elif defined(PCBX9E)
  CHKSIZE(ModelData, 2189);
#elif defined(PCBX9D) || defined(PCBX9DP)
  CHKSIZE(ModelData, 2188);
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_T14) || defined(RADIO_T12MAX)
  CHKSIZE(ModelData, 1731);
#elif defined(PCBPL18)
  #if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
    CHKSIZE(ModelData, 2185);
  #else
    CHKSIZE(ModelData, 2187);
  #endif
#elif defined(PCBST16) || defined(RADIO_T15PRO) || defined(RADIO_TX15)
  CHKSIZE(ModelData, 2840);
#elif defined(PCBPA01)
  CHKSIZE(ModelData, 2817);
#elif defined(RADIO_T15)
  CHKSIZE(ModelData, 2213);
#elif defined(RADIO_TX16SMK3)
  CHKSIZE(ModelData, 2841);
#elif defined(RADIO_H7RS)
  // CHKSIZE()
#elif defined(PCBHORUS)
  CHKSIZE(ModelData, 2187);
#else
  #error CHKSIZE not set up
#endif

#undef CHKSIZE
}
#endif /* BACKUP */
#endif /* !BOOT */
