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

#include "edgetx.h"

#if defined(EXTERNAL_ANTENNA)
bool isExternalAntennaEnabled()
{
  switch (g_eeGeneral.antennaMode) {
    case ANTENNA_MODE_INTERNAL:
      return false;
    case ANTENNA_MODE_EXTERNAL:
      return true;
    case ANTENNA_MODE_PER_MODEL:
      switch (g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode) {
        case ANTENNA_MODE_EXTERNAL:
        case ANTENNA_MODE_ASK:
          return globalData.externalAntennaEnabled;
        default:
          return false;
      }
    default:
      return globalData.externalAntennaEnabled;
  }
}
#endif

int8_t maxModuleChannels_M8(uint8_t moduleIdx)
{
  if (isExtraModule(moduleIdx)) {
    return MAX_EXTRA_MODULE_CHANNELS_M8;
  } else if (isModuleXJT(moduleIdx)) {
    return maxChannelsXJT_M8[1 + g_model.moduleData[moduleIdx].subType];
  } else if (isModuleISRMD16(moduleIdx)) {
    return maxChannelsXJT_M8[MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16];
  } else if (isModuleR9M(moduleIdx)) {
    if (isModuleR9M_LBT(moduleIdx)) {
      if (isModuleR9MLite(moduleIdx))
        return g_model.moduleData[moduleIdx].pxx.power ==
                       R9M_LITE_LBT_POWER_25_8CH
                   ? 0
                   : 8;
      else
        return g_model.moduleData[moduleIdx].pxx.power == R9M_LBT_POWER_25_8CH
                   ? 0
                   : 8;
    } else {
      return 8;  // always 16 channels in FCC / FLEX
    }
  } else if (isModuleMultimoduleDSM2(moduleIdx)) {
    return 4;  // 12 channels
  } else if (isModuleDSMP(moduleIdx) &&
             (g_model.moduleData[moduleIdx].dsmp.flags != 0)) {
    return g_model.moduleData[moduleIdx].channelsCount;
  } else {
    return maxChannelsModules_M8[g_model.moduleData[moduleIdx].type];
  }
}

int8_t sentModuleChannels(uint8_t idx)
{
  if (isModuleCrossfire(idx))
    return CROSSFIRE_CHANNELS_COUNT;
  else if (isModuleGhost(idx))
    return GHOST_CHANNELS_COUNT;
  else if (isModuleMultimodule(idx) && !isModuleMultimoduleDSM2(idx))
    return 16;
  else if (isModuleSBUS(idx))
    return 16;
  else
    return sentModulePXXChannels(idx);
}

uint8_t getMaxRxNum(uint8_t idx)
{
  if (isModuleDSM2(idx))
    return 20;

#if defined(MULTIMODULE)
  if (isModuleMultimodule(idx)) {
    switch (g_model.moduleData[idx].multi.rfProtocol) {
      case MODULE_SUBTYPE_MULTI_OLRS:
        return MODULE_SUBTYPE_MULTI_OLRS_RXNUM;
      case MODULE_SUBTYPE_MULTI_BUGS:
        return MODULE_SUBTYPE_MULTI_BUGS_RXNUM;
      case MODULE_SUBTYPE_MULTI_BUGS_MINI:
        return MODULE_SUBTYPE_MULTI_BUGS_MINI_RXNUM;
    }
  }
#endif

#if defined(AFHDS3)
  if (isModuleAFHDS3(idx)) return AFHDS3_MAX_MODEL_ID;
#endif
  
  return MAX_RXNUM;
}

void setModuleType(uint8_t moduleIdx, uint8_t moduleType)
{
  ModuleData & moduleData = g_model.moduleData[moduleIdx];
  memclear(&moduleData, sizeof(ModuleData));
  moduleData.type = moduleType;
  moduleData.channelsCount = defaultModuleChannels_M8(moduleIdx);
  if (moduleData.type == MODULE_TYPE_SBUS)
    moduleData.sbus.refreshRate = -31;
  else if (moduleData.type == MODULE_TYPE_PPM)
    setDefaultPpmFrameLength(moduleIdx);
  else if (moduleData.type == MODULE_TYPE_FLYSKY_AFHDS2A) {
    resetAfhds2AOptions(moduleIdx);
  }
  else if (moduleData.type == MODULE_TYPE_FLYSKY_AFHDS3) {
    resetAfhds3Options(moduleIdx);
  }
  else
    resetAccessAuthenticationCount();
}
