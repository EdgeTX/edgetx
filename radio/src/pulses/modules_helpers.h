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

#ifndef _MODULES_HELPERS_H_
#define _MODULES_HELPERS_H_

#include "libopenui/src/bitfield.h"
#include "definitions.h"
#include "opentx_helpers.h"
#include "storage/storage.h"
#include "globals.h"

#if defined(MULTIMODULE)
#include "telemetry/multi.h"
#endif

#if defined(PCBNV14)
extern uint32_t NV14internalModuleFwVersion;
#endif

#if defined(AFHDS3)
#include "pulses/afhds3_module.h"
#endif

#define CROSSFIRE_CHANNELS_COUNT        16
#define GHOST_CHANNELS_COUNT            16

#if defined (MULTIMODULE)
#define IS_D16_MULTI(module)                                                   \
  (((g_model.moduleData[module].multi.rfProtocol ==                          \
     MODULE_SUBTYPE_MULTI_FRSKY) &&                                            \
    (g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16 ||          \
     g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_8CH ||      \
     g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_LBT ||      \
     g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH ||  \
     g_model.moduleData[module].subType == MM_RF_FRSKY_SUBTYPE_D16_CLONED)) || \
   (g_model.moduleData[module].multi.rfProtocol ==                           \
    MODULE_SUBTYPE_MULTI_FRSKYX2))

#define IS_R9_MULTI(module)                         \
  (g_model.moduleData[module].multi.rfProtocol == \
   MODULE_SUBTYPE_MULTI_FRSKY_R9)

#define IS_HOTT_MULTI(module)                                           \
  (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_HOTT)

#define IS_CONFIG_MULTI(module)                                         \
  (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_CONFIG)

#define IS_DSM_MULTI(module)                                            \
  (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)

#define IS_RX_MULTI(module)                          \
  ((g_model.moduleData[module].multi.rfProtocol == \
    MODULE_SUBTYPE_MULTI_AFHDS2A_RX) ||              \
   (g_model.moduleData[module].multi.rfProtocol == \
    MODULE_SUBTYPE_MULTI_FRSKYX_RX) ||               \
   (g_model.moduleData[module].multi.rfProtocol == \
    MODULE_SUBTYPE_MULTI_BAYANG_RX) ||               \
   (g_model.moduleData[module].multi.rfProtocol == \
    MODULE_SUBTYPE_MULTI_DSM_RX))

#if defined(HARDWARE_INTERNAL_MODULE)
#define IS_FRSKY_SPORT_PROTOCOL()                                      \
  (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT ||              \
   (telemetryProtocol == PROTOCOL_TELEMETRY_MULTIMODULE &&             \
    (IS_D16_MULTI(INTERNAL_MODULE) || IS_D16_MULTI(EXTERNAL_MODULE) || \
     IS_R9_MULTI(INTERNAL_MODULE) || IS_R9_MULTI(EXTERNAL_MODULE))))
#else
#define IS_FRSKY_SPORT_PROTOCOL()                          \
  (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT ||  \
   (telemetryProtocol == PROTOCOL_TELEMETRY_MULTIMODULE && \
    (IS_D16_MULTI(EXTERNAL_MODULE) || IS_R9_MULTI(EXTERNAL_MODULE))))
#endif

#else
  #define IS_D16_MULTI(module)           false
  #define IS_R9_MULTI(module)            false
  #define IS_HOTT_MULTI(module)          false
  #define IS_CONFIG_MULTI(module)        false
  #define IS_DSM_MULTI(module)           false
  #define IS_FRSKY_SPORT_PROTOCOL()      (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT)
  #define IS_RX_MULTI(module)            false
#endif

#define IS_SPEKTRUM_PROTOCOL()           (telemetryProtocol == PROTOCOL_TELEMETRY_SPEKTRUM)


#if defined(MULTIMODULE)
// When using packed, the pointer in here end up not being aligned, which clang and gcc complain about
// Keep the order of the fields that the so that the size stays small
struct mm_options_strings {
  static const char* options[];
};

const uint8_t getMaxMultiOptions();

struct mm_protocol_definition {
  uint8_t protocol;
  uint8_t maxSubtype;
  bool failsafe;
  bool disable_ch_mapping;
  const char** subTypeString;
  const char* optionsstr;
};

const mm_protocol_definition *getMultiProtocolDefinition (uint8_t protocol);

inline uint8_t getMaxMultiSubtype(uint8_t moduleIdx)
{
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  auto proto = g_model.moduleData[moduleIdx].multi.rfProtocol;
  if (proto == MODULE_SUBTYPE_MULTI_FRSKY) {
    return 7;
  }

  uint8_t max_pdef = 0;
  const mm_protocol_definition *pdef = getMultiProtocolDefinition(proto);
  if (pdef) {
    max_pdef = pdef->maxSubtype;
  }
  uint8_t max_status = 0;
  if (status.isValid()) {
    max_status = (status.protocolSubNbr == 0 ? 0 : status.protocolSubNbr - 1);
  }
  return max(max_status, max_pdef);
}

inline bool isModuleMultimodule(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_MULTIMODULE;
}

inline bool isModuleMultimoduleDSM2(uint8_t idx)
{
  return isModuleMultimodule(idx) &&
         g_model.moduleData[idx].multi.rfProtocol ==
             MODULE_SUBTYPE_MULTI_DSM2;
}
#else
inline bool isModuleMultimodule(uint8_t)
{
  return false;
}

inline bool isModuleMultimoduleDSM2(uint8_t)
{
  return false;
}
#endif

inline bool isModuleTypeXJT(uint8_t type)
{
  return type == MODULE_TYPE_XJT_PXX1 || type == MODULE_TYPE_XJT_LITE_PXX2;
}

inline bool isModuleNone(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_NONE;
}

inline bool isModuleXJT(uint8_t idx)
{
  return isModuleTypeXJT(g_model.moduleData[idx].type);
}

inline bool isModuleXJTD8(uint8_t idx)
{
  return isModuleXJT(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_PXX1_ACCST_D8;
}

inline bool isModuleXJTLR12(uint8_t idx)
{
  return isModuleXJT(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_PXX1_ACCST_LR12;
}

inline bool isModuleXJTD16(uint8_t idx)
{
  return isModuleXJT(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_PXX1_ACCST_D16;
}

inline bool isModuleTypeISRM(uint8_t type)
{
  return type == MODULE_TYPE_ISRM_PXX2;
}

inline bool isModuleISRM(uint8_t idx)
{
  return isModuleTypeISRM(g_model.moduleData[idx].type);
}

inline bool isModuleISRMD16(uint8_t idx)
{
  return isModuleISRM(idx) &&
         g_model.moduleData[idx].subType == MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16;
}

inline bool isModuleD16(uint8_t idx)
{
  return isModuleXJTD16(idx) || isModuleISRMD16(idx) || IS_D16_MULTI(idx);
}

inline bool isModuleISRMAccess(uint8_t idx)
{
  return isModuleISRM(idx) &&
         g_model.moduleData[idx].subType == MODULE_SUBTYPE_ISRM_PXX2_ACCESS;
}

#if defined(CROSSFIRE)
inline bool isModuleCrossfire(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_CROSSFIRE;
}

inline bool isInternalModuleCrossfire()
{
  return g_eeGeneral.internalModule == MODULE_TYPE_CROSSFIRE;
}
#else
inline bool isModuleCrossfire(uint8_t idx)
{
  return false;
}
#endif

#if defined(GHOST)
inline bool isModuleGhost(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_GHOST;
}
#else
inline bool isModuleGhost(uint8_t idx)
{
  return false;
}
#endif

inline bool isExtraModule(uint8_t)
{
  return false;
}

inline bool isModuleTypePPM(uint8_t type)
{
  return type == MODULE_TYPE_PPM;
}

inline bool isModulePPM(uint8_t moduleIdx)
{
  return isModuleTypePPM(g_model.moduleData[moduleIdx].type);
}

inline bool isModuleTypeR9MNonAccess(uint8_t type)
{
  return type == MODULE_TYPE_R9M_PXX1 || type == MODULE_TYPE_R9M_LITE_PXX1;
}

inline bool isModuleR9MNonAccess(uint8_t idx)
{
  return isModuleTypeR9MNonAccess(g_model.moduleData[idx].type);
}

inline bool isModuleTypeR9MAccess(uint8_t type)
{
  return type == MODULE_TYPE_R9M_PXX2 || type == MODULE_TYPE_R9M_LITE_PXX2 || type == MODULE_TYPE_R9M_LITE_PRO_PXX2;
}

inline bool isModuleR9MAccess(uint8_t idx)
{
  return isModuleTypeR9MAccess(g_model.moduleData[idx].type);
}

inline bool isModuleTypeR9M(uint8_t type)
{
  return isModuleTypeR9MNonAccess(type) || isModuleTypeR9MAccess(type);
}

inline bool isModuleR9M(uint8_t idx)
{
  return isModuleTypeR9M(g_model.moduleData[idx].type);
}

inline bool isModuleTypeR9MLiteNonPro(uint8_t type)
{
  return type == MODULE_TYPE_R9M_LITE_PXX1 || type == MODULE_TYPE_R9M_LITE_PXX2;
}

inline bool isModuleR9MLiteNonPro(uint8_t idx)
{
  return isModuleTypeR9MLiteNonPro(g_model.moduleData[idx].type);
}

inline bool isModuleTypeR9MLitePro(uint8_t type)
{
  return type == MODULE_TYPE_R9M_LITE_PRO_PXX2;
}

inline bool isModuleTypeR9MLite(uint8_t type)
{
  return isModuleTypeR9MLiteNonPro(type) || isModuleTypeR9MLitePro(type);
}

inline bool isModuleR9MLite(uint8_t idx)
{
  return isModuleTypeR9MLite(g_model.moduleData[idx].type);
}

inline bool isModuleR9M_FCC(uint8_t idx)
{
  return isModuleR9MNonAccess(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_R9M_FCC;
}

inline bool isModuleTypeLite(uint8_t type)
{
  return isModuleTypeR9MLite(type) || type == MODULE_TYPE_XJT_LITE_PXX2;
}

inline bool isModuleR9M_LBT(uint8_t idx)
{
  return isModuleR9MNonAccess(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_R9M_EU;
}

inline bool isModuleR9M_FCC_VARIANT(uint8_t idx)
{
  return isModuleR9MNonAccess(idx) && g_model.moduleData[idx].subType != MODULE_SUBTYPE_R9M_EU;
}

inline bool isModuleR9M_EUPLUS(uint8_t idx)
{
  return isModuleR9MNonAccess(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_R9M_EUPLUS;
}

inline bool isModuleR9M_AU_PLUS(uint8_t idx)
{
  return isModuleR9MNonAccess(idx) && g_model.moduleData[idx].subType == MODULE_SUBTYPE_R9M_AUPLUS;
}

inline bool isModuleTypePXX1(uint8_t type)
{
  return isModuleTypeXJT(type) || isModuleTypeR9MNonAccess(type);
}

inline bool isModulePXX1(uint8_t idx)
{
  return isModuleTypePXX1(g_model.moduleData[idx].type);
}

inline bool isModuleXJTLite(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_XJT_LITE_PXX2;
}

inline bool isModulePXX2(uint8_t idx)
{
  return isModuleISRM(idx) || isModuleR9MAccess(idx) || isModuleXJTLite(idx);
}

inline bool isModuleRFAccess(uint8_t idx)
{
  if (isModuleISRM(idx)) {
    return g_model.moduleData[idx].subType == MODULE_SUBTYPE_ISRM_PXX2_ACCESS;
  }
  else if (isModuleR9MAccess(idx)) {
    return true;
  }
  else {
    return false;
  }
}

inline bool isModuleDSM2(uint8_t moduleIdx)
{
  return g_model.moduleData[moduleIdx].type == MODULE_TYPE_DSM2;
}

inline bool isModuleSBUS(uint8_t moduleIdx)
{
  return g_model.moduleData[moduleIdx].type == MODULE_TYPE_SBUS;
}

inline bool isModuleFlySky(uint8_t idx)
{
  return
    (g_model.moduleData[idx].type == MODULE_TYPE_FLYSKY);
}

inline bool isModuleAFHDS2A(uint8_t idx)
{
  return isModuleFlySky(idx)
    && (g_model.moduleData[idx].subType == FLYSKY_SUBTYPE_AFHDS2A);
}

inline bool isModuleAFHDS3(uint8_t idx)
{
  return isModuleFlySky(idx)
    && (g_model.moduleData[idx].subType == FLYSKY_SUBTYPE_AFHDS3);
}

inline bool isModuleDSMP(uint8_t idx)
{
  return g_model.moduleData[idx].type == MODULE_TYPE_LEMON_DSMP;
}

// See enum ModuleType in modules_constant.h
// relative to 8!
static const int8_t maxChannelsModules_M8[] = {
  0, // MODULE_TYPE_NONE
  8, // MODULE_TYPE_PPM
  0, // MODULE_TYPE_XJT_PXX1: index NOT USED
  16,// MODULE_TYPE_ISRM_PXX2
  -2,// MODULE_TYPE_DSM2
  CROSSFIRE_CHANNELS_COUNT - 8, // MODULE_TYPE_CROSSFIRE
  8, // MODULE_TYPE_MULTIMODULE
  0, // MODULE_TYPE_R9M_PXX1: index NOT USED
  0, // MODULE_TYPE_R9M_PXX2: index NOT USED
  0, // MODULE_TYPE_R9M_LITE_PXX1: index NOT USED
  0, // MODULE_TYPE_R9M_LITE_PXX2: index NOT USED
  GHOST_CHANNELS_COUNT - 8, // MODULE_TYPE_GHOST
  0, // MODULE_TYPE_R9M_LITE_PRO_PXX2: index NOT USED
  8, // MODULE_TYPE_SBUS
  0, // MODULE_TYPE_XJT_LITE_PXX2: index NOT USED
  6, // MODULE_TYPE_FLYSKY: 14 channels for AFHDS2A, AFHDS3 special cased
  4, // MODULE_TYPE_LEMON_DSMP: 12 channels for DSMX
};

static_assert(MODULE_TYPE_COUNT == sizeof(maxChannelsModules_M8),
              "MODULE_TYPE_COUNT != sizeof(maxChannelsModules_M8)");

// See enum ModuleSubtypePXX1 in modules_constant.h
// relative to 8!
static const int8_t maxChannelsXJT_M8[] = {
  0, // MODULE_SUBTYPE_PXX1_OFF
  8, // MODULE_SUBTYPE_PXX1_ACCST_D16
  0, // MODULE_SUBTYPE_PXX1_ACCST_D8
  4  // MODULE_SUBTYPE_PXX1_ACCST_LR12
};

// MODULE_SUBTYPE_PXX1_OFF == -1
static_assert(MODULE_SUBTYPE_PXX1_LAST + 2 == sizeof(maxChannelsXJT_M8),
              "MODULE_SUBTYPE_PXX1_LAST + 2 != sizeof(maxChannelsXJT_M8)");

constexpr int8_t MAX_TRAINER_CHANNELS_M8 = MAX_TRAINER_CHANNELS - 8;
constexpr int8_t MAX_EXTRA_MODULE_CHANNELS_M8 = 8; // only 16ch PPM

inline int8_t maxModuleChannels_M8(uint8_t moduleIdx)
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
  } else if (isModuleAFHDS3(moduleIdx)) {
    return 10;
  } else if (isModuleMultimoduleDSM2(moduleIdx)) {
    return 4;  // 12 channels
  } else if (isModuleDSMP(moduleIdx) &&
             (g_model.moduleData[moduleIdx].dsmp.flags != 0)) {
    return g_model.moduleData[moduleIdx].channelsCount;
  } else {
    return maxChannelsModules_M8[g_model.moduleData[moduleIdx].type];
  }
}

inline int8_t maxModuleChannels(uint8_t moduleIdx)
{
  return maxModuleChannels_M8(moduleIdx) + 8;
}

inline int8_t minModuleChannels(uint8_t idx)
{
  if (isModuleCrossfire(idx))
    return CROSSFIRE_CHANNELS_COUNT;
  else if (isModuleGhost(idx))
    return GHOST_CHANNELS_COUNT;
  else if (isModuleSBUS(idx))
    return 16;
  else if (isModuleDSMP(idx))
    return maxModuleChannels(idx);
  else
    return 1;
}

inline int8_t defaultModuleChannels_M8(uint8_t idx)
{
  if (isModulePPM(idx))
    return 0; // 8 channels
  else
    return maxModuleChannels_M8(idx);
}

inline uint8_t sentModulePXXChannels(uint8_t idx)
{
  return 8 + g_model.moduleData[idx].channelsCount;
}

inline int8_t sentModuleChannels(uint8_t idx)
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

inline bool isDefaultModelRegistrationID()
{
  return memcmp(g_model.modelRegistrationID, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID) == 0;
}

inline bool isModuleRxNumAvailable(uint8_t moduleIdx)
{
  if (isModuleXJT(moduleIdx))
    return g_model.moduleData[moduleIdx].subType != MODULE_SUBTYPE_PXX1_ACCST_D8;

  if (isModuleR9M(moduleIdx))
    return true;

  if (isModuleDSM2(moduleIdx))
    return true;

  if (isModuleISRM(moduleIdx))
    return true;

  if (isModuleMultimodule(moduleIdx))
    return true;

  if (isModuleCrossfire(moduleIdx))
    return true;

  return false;
}

inline bool isModuleModelIndexAvailable(uint8_t idx)
{
  if (isModuleXJT(idx))
    return g_model.moduleData[idx].subType != MODULE_SUBTYPE_PXX1_ACCST_D8;

  if (isModuleR9M(idx))
    return true;

  if (isModuleDSM2(idx))
    return true;

  if (isModuleISRM(idx))
    return true;

  if (isModuleMultimodule(idx))
    return true;

  if (isModuleCrossfire(idx))
    return true;

  if (isModuleAFHDS3(idx))
    return true;
  
  return false;
}

inline bool isModuleFailsafeAvailable(uint8_t moduleIdx)
{
#if defined(PXX2)
  if (isModuleISRM(moduleIdx))
    return true;
#endif

  if (isModuleXJT(moduleIdx))
    return g_model.moduleData[moduleIdx].subType == MODULE_SUBTYPE_PXX1_ACCST_D16;

#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
    if (status.isValid()) {
      return status.supportsFailsafe();
    }
    else {
      auto proto = g_model.moduleData[moduleIdx].multi.rfProtocol;
      const mm_protocol_definition * pdef = getMultiProtocolDefinition(proto);
      if (pdef) return pdef->failsafe;
      return false;
    }
  }
#endif

#if defined(AFHDS3) || defined(AFHDS2)
  if (isModuleFlySky(moduleIdx))
    return true;
#endif

  if (isModuleR9M(moduleIdx))
    return true;

  return false;
}

inline bool isModuleBindRangeAvailable(uint8_t moduleIdx)
{
  return isModulePXX2(moduleIdx) || isModulePXX1(moduleIdx) ||
         isModuleDSM2(moduleIdx) || isModuleMultimodule(moduleIdx) ||
         isModuleFlySky(moduleIdx) || isModuleDSMP(moduleIdx);
}

inline uint32_t getNV14RfFwVersion()
{
#if defined(PCBNV14)
  return  NV14internalModuleFwVersion;
#else
  return 0;
#endif
}

inline bool isModuleRangeAvailable(uint8_t moduleIdx)
{
  bool ret = isModuleBindRangeAvailable(moduleIdx) && !IS_RX_MULTI(moduleIdx);
#if defined(PCBNV14)
  ret = ret &&
        (!isModuleFlySky(moduleIdx) || NV14internalModuleFwVersion >= 0x1000E);
#else
  ret = ret && (!isModuleFlySky(moduleIdx));
#endif
  return ret;
}

constexpr uint8_t MAX_RXNUM = 63;

inline uint8_t getMaxRxNum(uint8_t idx)
{
  if (isModuleDSM2(idx))
    return 20;

#if defined(MULTIMODULE)
  if (isModuleMultimodule(idx)) {
    switch (g_model.moduleData[idx].multi.rfProtocol) {
      case MODULE_SUBTYPE_MULTI_OLRS:
        return 4;
      case MODULE_SUBTYPE_MULTI_BUGS:
      case MODULE_SUBTYPE_MULTI_BUGS_MINI:
        return 15;
    }
  }
#endif

#if defined(AFHDS3)
  if (isModuleAFHDS3(idx)) return AFHDS3_MAX_MODEL_ID;
#endif
  
  return MAX_RXNUM;
}

inline const char * getModuleDelay(uint8_t idx)
{
  if (isModuleISRMAccess(idx))
    return sentModuleChannels(idx) > 16 ? "(21ms)" : (sentModuleChannels(idx) > 8 ? "(14ms)" : "(7ms)");

  if (isModuleXJTD16(idx) || isModuleR9MNonAccess(idx))
    return sentModuleChannels(idx) > 8 ? "(18ms)" : "(9ms)";

  return nullptr;
}

inline bool isBindCh9To16Allowed(uint8_t moduleIndex)
{
  if (g_model.moduleData[moduleIndex].channelsCount <= 0) {
    return false;
  }

  if (isModuleR9M_LBT(moduleIndex)) {
    if (isModuleR9MLite(moduleIndex))
      return g_model.moduleData[moduleIndex].pxx.power != R9M_LBT_POWER_25_8CH;
    else
      return g_model.moduleData[moduleIndex].pxx.power != R9M_LITE_LBT_POWER_25_8CH;
  }
  else {
    return true;
  }
}

#if defined(PCBTARANIS) || defined(PCBHORUS)
inline bool isSportLineUsedByInternalModule()
{
  return g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_XJT_PXX1;
}
#else
inline bool isSportLineUsedByInternalModule()
{
  return false;
}
#endif

inline bool isTelemAllowedOnBind(uint8_t moduleIndex)
{
#if defined(HARDWARE_INTERNAL_MODULE)
  if (moduleIndex == INTERNAL_MODULE)
    return true;

  if (isSportLineUsedByInternalModule())
    return false;
#endif

  if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_LITE_PXX1) {
    if (isModuleR9M_LBT(EXTERNAL_MODULE))
      return g_model.moduleData[EXTERNAL_MODULE].pxx.power < R9M_LITE_LBT_POWER_100_16CH_NOTELEM;
    else
      return true;
  }

  if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_PXX1) {
    if (isModuleR9M_LBT(EXTERNAL_MODULE))
      return g_model.moduleData[EXTERNAL_MODULE].pxx.power < R9M_LBT_POWER_200_16CH_NOTELEM;
    else
      return true;
  }

  return true;
}

inline bool isPXX2ReceiverUsed(uint8_t moduleIdx, uint8_t receiverIdx)
{
  return g_model.moduleData[moduleIdx].pxx2.receivers & (1 << receiverIdx);
}

inline void setPXX2ReceiverUsed(uint8_t moduleIdx, uint8_t receiverIdx)
{
  g_model.moduleData[moduleIdx].pxx2.receivers |= (1 << receiverIdx);
}

inline bool isPXX2ReceiverEmpty(uint8_t moduleIdx, uint8_t receiverIdx)
{
  return is_memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
}

inline void removePXX2Receiver(uint8_t moduleIdx, uint8_t receiverIdx)
{
  memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
  g_model.moduleData[moduleIdx].pxx2.receivers &= ~(1 << receiverIdx);
  storageDirty(EE_MODEL);
}

inline void removePXX2ReceiverIfEmpty(uint8_t moduleIdx, uint8_t receiverIdx)
{
  if (isPXX2ReceiverEmpty(moduleIdx, receiverIdx)) {
    removePXX2Receiver(moduleIdx, receiverIdx);
  }
}

inline void setDefaultPpmFrameLength(uint8_t moduleIdx)
{
  // channelsCount + 8 ???
  g_model.moduleData[moduleIdx].ppm.frameLength =
      4 * max<int>(0, g_model.moduleData[moduleIdx].channelsCount);
}

inline void resetAccessAuthenticationCount()
{
#if defined(ACCESS_LIB)
  // the module will reset on mode switch, we need to reset the authentication counter
  globalData.authenticationCount = 0;
#endif
}

inline void resetAfhdsOptions(uint8_t moduleIdx)
{
#if defined(AFHDS3)
  auto & data = g_model.moduleData[moduleIdx];
  data.subType = FLYSKY_SUBTYPE_AFHDS3;
  data.afhds3.emi = 2; // FCC
  data.afhds3.telemetry = 1;
  data.afhds3.phyMode = 0;
#elif defined(AFHDS2)
  auto & data = g_model.moduleData[moduleIdx];
  data.subType = FLYSKY_SUBTYPE_AFHDS2A;
  data.flysky.setDefault();
#endif
}


inline void setModuleType(uint8_t moduleIdx, uint8_t moduleType)
{
  ModuleData & moduleData = g_model.moduleData[moduleIdx];
  memclear(&moduleData, sizeof(ModuleData));
  moduleData.type = moduleType;
  moduleData.channelsCount = defaultModuleChannels_M8(moduleIdx);
  if (moduleData.type == MODULE_TYPE_SBUS)
    moduleData.sbus.refreshRate = -31;
  else if (moduleData.type == MODULE_TYPE_PPM)
    setDefaultPpmFrameLength(moduleIdx);
  else if (moduleData.type == MODULE_TYPE_FLYSKY) {
    resetAfhdsOptions(moduleIdx);
  }
  else
    resetAccessAuthenticationCount();
}

extern bool isExternalAntennaEnabled();

#if defined(MULTIMODULE)
inline void resetMultiProtocolsOptions(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return;

  // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
  if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2) {
    g_model.moduleData[moduleIdx].multi.autoBindMode = 1;
  }
  else {
    g_model.moduleData[moduleIdx].multi.autoBindMode = 0;
  }
  g_model.moduleData[moduleIdx].multi.optionValue = 0;
  g_model.moduleData[moduleIdx].multi.disableTelemetry = 0;
  g_model.moduleData[moduleIdx].multi.disableMapping = 0;
  g_model.moduleData[moduleIdx].multi.lowPowerMode = 0;
  g_model.moduleData[moduleIdx].failsafeMode = FAILSAFE_NOT_SET;
  g_model.header.modelId[moduleIdx] = 0;
}

inline void getMultiOptionValues(int8_t multi_proto, int8_t & min, int8_t & max)
{
  switch (multi_proto) {
    case MODULE_SUBTYPE_MULTI_DSM2:
      min = 0;
      max = 1;
      break;
    case MODULE_SUBTYPE_MULTI_BAYANG:
      min = 0;
      max = 3;
      break;
    case MODULE_SUBTYPE_MULTI_OLRS:
      min = -1;
      max = 7;
      break;
    case MODULE_SUBTYPE_MULTI_FS_AFHDS2A:
      min = 0;
      max = 70;
      break;
    case MODULE_SUBTYPE_MULTI_XN297DUMP:
      min = -1;
      max = 84;
      break;
    default:
      min = -128;
      max = 127;
      break;
  }
}
#endif

#endif // _MODULES_HELPERS_H_
