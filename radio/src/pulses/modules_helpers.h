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

#include "bitfield.h"
#include "definitions.h"
#include "edgetx_helpers.h"
#include "storage/storage.h"
#include "globals.h"
#include "MultiProtoDefs.h"
#include "hal/module_port.h"
#include "telemetry/crossfire.h"

#if defined(MULTIMODULE)
#include "telemetry/multi.h"
#endif

#if defined(PCBNV14) && defined(AFHDS2)
extern uint32_t NV14internalModuleFwVersion;
#endif

#if defined(AFHDS3)
#include "pulses/afhds3_module.h"
#endif

#define CROSSFIRE_CHANNELS_COUNT        16
#define GHOST_CHANNELS_COUNT            16

#define IS_NATIVE_FRSKY_PROTOCOL(module)                                \
  ((moduleState[module].protocol == PROTOCOL_CHANNELS_PXX1) ||          \
   (moduleState[module].protocol == PROTOCOL_CHANNELS_PXX2))

#if defined (MULTIMODULE)
  #define IS_D16_MULTI(module)                                            \
    ((g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_FRSKYX) || \
    (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_FRSKYX2))

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

  // When using packed, the pointer in here end up not being aligned, which clang and gcc complain about
  // Keep the order of the fields that the so that the size stays small
  struct mm_options_strings {
    static const char* const options[];
  };

  const uint8_t getMaxMultiOptions();

  PACK_NOT_SIMU(struct mm_protocol_definition {
    uint8_t protocol;
    uint8_t maxSubtype:6;
    bool failsafe:1;
    bool disable_ch_mapping:1;
    const char* const* subTypeString;
    const char* optionsstr;
  });

  const mm_protocol_definition *getMultiProtocolDefinition (uint8_t protocol);

  inline uint8_t getMaxMultiSubtype(uint8_t moduleIdx)
  {
    MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);

    uint8_t max_pdef = 0;
    auto proto = g_model.moduleData[moduleIdx].multi.rfProtocol;
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
#else
  #define IS_D16_MULTI(module)           false
  #define IS_R9_MULTI(module)            false
  #define IS_HOTT_MULTI(module)          false
  #define IS_CONFIG_MULTI(module)        false
  #define IS_DSM_MULTI(module)           false
  #define IS_RX_MULTI(module)            false

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

inline bool isModuleELRS(uint8_t idx)
{
  return crossfireModuleStatus[idx].isELRS;
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

inline bool isModuleELRS(uint8_t idx)
{
  return false;
}

inline bool isInternalModuleCrossfire()
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

inline bool isModuleAFHDS2A(uint8_t idx)
{
  return (g_model.moduleData[idx].type == MODULE_TYPE_FLYSKY_AFHDS2A);
}

inline bool isModuleAFHDS3(uint8_t idx)
{
  return (g_model.moduleData[idx].type == MODULE_TYPE_FLYSKY_AFHDS3);
}

inline bool isModuleFlySky(uint8_t idx)
{
  return (isModuleAFHDS2A(idx) || isModuleAFHDS3(idx));
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
  6, // MODULE_TYPE_FLYSKY_AFHDS2A: 14 channels
  10,// MODULE_TYPE_FLYSKY_AFHDS3: 18 channels
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

extern int8_t maxModuleChannels_M8(uint8_t moduleIdx);

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

extern int8_t sentModuleChannels(uint8_t idx);

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

#if defined(MULTIMODULE)
constexpr int32_t MULTI_DSM_CLONE_VERSION = (1 << 24) | (3 << 16) | (3 << 8) | 30;

inline bool isMultiProtocolDSMCloneAvailable(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return false;

  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  if (status.isValid() && ((status.major << 24) | (status.minor << 16) | (status.revision << 8) | status.patch) < MULTI_DSM_CLONE_VERSION) {
    return false;
  }

  return g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2;
}
#endif

inline bool isModuleBindRangeAvailable(uint8_t moduleIdx)
{
  return isModulePXX2(moduleIdx) || isModulePXX1(moduleIdx) ||
         isModuleDSM2(moduleIdx) || isModuleMultimodule(moduleIdx) ||
         isModuleFlySky(moduleIdx) || isModuleDSMP(moduleIdx) ||
         (isModuleELRS(moduleIdx) && (crossfireModuleStatus[moduleIdx].major >= 4 || (crossfireModuleStatus[moduleIdx].major == 3 && crossfireModuleStatus[moduleIdx].minor >= 4)));
}

inline uint32_t getNV14RfFwVersion()
{
#if defined(PCBNV14) && defined(AFHDS2)
  return  NV14internalModuleFwVersion;
#else
  return 0;
#endif
}

inline bool isModuleRangeAvailable(uint8_t moduleIdx)
{
  bool ret = isModuleBindRangeAvailable(moduleIdx) && !IS_RX_MULTI(moduleIdx) && !isModuleCrossfire(moduleIdx);
#if defined(PCBNV14) && defined(AFHDS2)
  ret = ret &&
        (!isModuleAFHDS2A(moduleIdx) || NV14internalModuleFwVersion >= 0x1000E);
#elif defined(AFHDS3)
  ret = ret && (!isModuleAFHDS3(moduleIdx));
#endif
  return ret;
}

constexpr uint8_t MAX_RXNUM = 63;

extern uint8_t getMaxRxNum(uint8_t idx);

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

inline bool isTelemAllowedOnBind(uint8_t moduleIndex)
{
#if defined(HARDWARE_INTERNAL_MODULE)
  if (moduleIndex == INTERNAL_MODULE)
    return true;

  if (modulePortIsPortUsedByModule(INTERNAL_MODULE, ETX_MOD_PORT_SPORT))
    return false;
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
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
#endif

  return true;
}

inline bool isPXX2ReceiverUsed(uint8_t moduleIdx, uint8_t receiverIdx)
{
  return g_model.moduleData[moduleIdx].pxx2.receivers & (1 << receiverIdx);
}

inline void setPXX2ReceiverUsed(uint8_t moduleIdx, uint8_t receiverIdx)
{
  g_model.moduleData[moduleIdx].pxx2.receivers |= (1 << receiverIdx);
  storageDirty(EE_MODEL);
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

inline void setDefaultPpmFrameLengthTrainer()
{
  g_model.trainerData.frameLength =
      4 * max<int>(0, g_model.trainerData.channelsCount);
}

inline void resetAccessAuthenticationCount()
{
  // the module will reset on mode switch, we need to reset the authentication counter
  globalData.authenticationCount = 0;
}

inline void resetAfhds3Options(uint8_t moduleIdx)
{
#if defined(AFHDS3)
  auto & data = g_model.moduleData[moduleIdx];
  data.subType = 0;
  data.afhds3.emi = 2; // FCC
  data.afhds3.telemetry = 1;
  data.afhds3.phyMode = 0;
#endif
}

inline void resetAfhds2AOptions(uint8_t moduleIdx)
{
#if defined(AFHDS2)
  auto & data = g_model.moduleData[moduleIdx];
  data.subType = 0;
  data.flysky.setDefault();
#endif
}

extern void setModuleType(uint8_t moduleIdx, uint8_t moduleType);

extern bool isExternalAntennaEnabled();
