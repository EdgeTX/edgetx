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

#ifndef _GUI_COMMON_H_
#define _GUI_COMMON_H_

#include <functional>
#include "lcd.h"
#include "keys.h"
#include "telemetry/telemetry_sensors.h"

#define READONLY_ROW                   ((uint8_t)-1)
#define TITLE_ROW                      READONLY_ROW
#define LABEL(...)                     READONLY_ROW
#define HIDDEN_ROW                     ((uint8_t)-2)

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define CASE_ROTARY_ENCODER(x) x,
  #define CASE_EVT_ROTARY_LEFT         case EVT_ROTARY_LEFT:
  #define CASE_EVT_ROTARY_RIGHT        case EVT_ROTARY_RIGHT:
#else
  #define CASE_ROTARY_ENCODER(x)
  #define CASE_EVT_ROTARY_LEFT
  #define CASE_EVT_ROTARY_RIGHT
#endif

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
inline uint8_t MENU_FIRST_LINE_EDIT(const uint8_t * horTab, uint8_t horTabMax)
{
  if (horTab) {
    uint8_t result = 0;
    while (result < horTabMax && horTab[result] >= HIDDEN_ROW)
      ++result;
    return result;
  }
  else {
    return 0;
  }
}
#endif

#if defined(LIBOPENUI)
typedef std::function<bool(int)> IsValueAvailable;
#else
typedef bool (*IsValueAvailable)(int);
#endif

enum SwitchContext
{
  LogicalSwitchesContext,
  ModelCustomFunctionsContext,
  GeneralCustomFunctionsContext,
  TimersContext,
  MixesContext
};

int circularIncDec(int current, int inc, int min, int max, IsValueAvailable isValueAvailable=nullptr);
int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable);

bool isTrimModeAvailable(int mode);
bool isInputAvailable(int input);
bool isSourceAvailableInInputs(int source);
bool isThrottleSourceAvailable(int source);
bool isLogicalSwitchFunctionAvailable(int function);
bool isLogicalSwitchAvailable(int index);
bool isAssignableFunctionAvailable(int function);
bool isSourceAvailable(int source);
bool isBacklightSourceAvailable(int source);
bool isSourceAvailableInGlobalFunctions(int source);
bool isSourceAvailableInCustomSwitches(int source);
bool isSourceAvailableInResetSpecialFunction(int index);
bool isSourceAvailableInGlobalResetSpecialFunction(int index);
bool isSwitchAvailable(int swtch, SwitchContext context);
bool isSerialModeAvailable(uint8_t port_nr, int mode);
int  hasSerialMode(int mode);
bool isSwitchAvailableInLogicalSwitches(int swtch);
bool isSwitchAvailableInCustomFunctions(int swtch);
bool isSwitchAvailableInMixes(int swtch);
bool isSwitchAvailableInTimers(int swtch);
bool isPxx2IsrmChannelsCountAllowed(int channels);
bool isModuleUsingSport(uint8_t moduleBay, uint8_t moduleType);
bool isTrainerUsingModuleBay();
bool isExternalModuleAvailable(int moduleType);
bool isInternalModuleAvailable(int moduleType);
bool isInternalModuleSupported(int moduleType);
bool isRfProtocolAvailable(int protocol);
bool isTelemetryProtocolAvailable(int protocol);
bool isTrainerModeAvailable(int mode);
bool isAssignableFunctionAvailable(int function, CustomFunctionData * functions);

bool isSensorUnit(int sensor, uint8_t unit);
bool isCellsSensor(int sensor);
bool isGPSSensor(int sensor);
bool isAltSensor(int sensor);
bool isVoltsSensor(int sensor);
bool isCurrentSensor(int sensor);
bool isTelemetryFieldAvailable(int index);
uint8_t getTelemetrySensorsCount();
bool isTelemetryFieldComparisonAvailable(int index);
bool isSensorAvailable(int sensor);
bool isRssiSensorAvailable(int sensor);

bool modelHasNotes();

bool confirmModelChange();

#if defined(COLORLCD)
bool isSwitch2POSWarningStateAvailable(int state);
#endif

#if defined(LIBOPENUI)
#define IS_INSTANT_TRIM_ALLOWED()     true
#elif defined(GUI)
#define IS_INSTANT_TRIM_ALLOWED()      (IS_MAIN_VIEW_DISPLAYED() || IS_TELEMETRY_VIEW_DISPLAYED() || IS_OTHER_VIEW_DISPLAYED())
#else
#define IS_INSTANT_TRIM_ALLOWED()      true
#endif

swsrc_t checkIncDecMovedSwitch(swsrc_t val);

// TODO move this to stdlcd/draw_functions.h ?
void drawCurveRef(coord_t x, coord_t y, CurveRef & curve, LcdFlags flags=0);
void drawDate(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawTelemScreenDate(coord_t x, coord_t y, source_t sensor, LcdFlags flags=0);
void drawGPSPosition(coord_t x, coord_t y, int32_t longitude, int32_t latitude, LcdFlags flags=0);
void drawGPSSensorValue(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor, int32_t value, LcdFlags flags=0);
void drawSourceCustomValue(coord_t x, coord_t y, source_t channel, int32_t val, LcdFlags flags=0);
void drawSourceValue(coord_t x, coord_t y, source_t channel, LcdFlags flags=0);

int convertMultiToOtx(int type);

// model_setup Defines that are used in all uis in the same way
#define IF_INTERNAL_MODULE_ON(x)                  (IS_INTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)
#define IF_MODULE_ON(moduleIndex, x)              (IS_MODULE_ENABLED(moduleIndex) ? (uint8_t)(x) : HIDDEN_ROW)

inline uint8_t MODULE_BIND_ROWS(int moduleIdx)
{
  if (isModuleCrossfire(moduleIdx))
    return 0;

  if (isModuleMultimodule(moduleIdx)) {
    if (IS_RX_MULTI(moduleIdx))
      return 1;
    else
      return 2;
  }
  else if (isModuleXJTD8(moduleIdx) || isModuleSBUS(moduleIdx) || isModuleAFHDS3(moduleIdx) || isModuleDSMP(moduleIdx)) {
    return 1;
  }
  else if (isModulePPM(moduleIdx) || isModulePXX1(moduleIdx) || isModulePXX2(moduleIdx) || isModuleDSM2(moduleIdx)) {
    return 2;
  }
  else {
    return HIDDEN_ROW;
  }
}

inline uint8_t MODULE_CHANNELS_ROWS(int moduleIdx)
{
  if (!IS_MODULE_ENABLED(moduleIdx)) {
    return HIDDEN_ROW;
  }
  else if (isModuleMultimodule(moduleIdx)) {
    if (IS_RX_MULTI(moduleIdx))
      return HIDDEN_ROW;
    else if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
      return 1;
    else
      return 0;
  } else if (isModuleDSM2(moduleIdx) || isModuleCrossfire(moduleIdx) ||
             isModuleGhost(moduleIdx) || isModuleSBUS(moduleIdx) ||
             isModuleDSMP(moduleIdx)) {
    // fixed number of channels
    return 0;
  } else {
    return 1;
  }
}

#if defined(PXX2)
inline bool isRacingModeAllowed()
{
  return isModulePXX2(INTERNAL_MODULE) && g_model.moduleData[INTERNAL_MODULE].getChannelsCount() == 8;
}

inline bool isRacingModeEnabled()
{
  return isRacingModeAllowed() && g_model.moduleData[INTERNAL_MODULE].pxx2.racingMode;
}

inline uint8_t IF_ALLOW_RACING_MODE(int moduleIdx)
{
  if (!IS_MODULE_ENABLED(moduleIdx)) {
    return HIDDEN_ROW;
  }
  else if (isRacingModeAllowed()) {
    return 0;
  }
  return HIDDEN_ROW;
}
#else
inline uint8_t IF_ALLOW_RACING_MODE(int)
{
  return HIDDEN_ROW;
}
#endif

#if defined(MULTIMODULE)
inline uint8_t MULTI_DISABLE_CHAN_MAP_ROW_STATIC(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return HIDDEN_ROW;

  uint8_t protocol = g_model.moduleData[moduleIdx].multi.rfProtocol;
  if (protocol < MODULE_SUBTYPE_MULTI_LAST) {
    const mm_protocol_definition * pdef = getMultiProtocolDefinition(protocol);
    if (pdef->disable_ch_mapping)
      return 0;
  }

  return HIDDEN_ROW;
}

inline uint8_t MULTI_DISABLE_CHAN_MAP_ROW(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return HIDDEN_ROW;

  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  if (status.isValid()) {
    return status.supportsDisableMapping() == true ? 0 : HIDDEN_ROW;
  }

  return MULTI_DISABLE_CHAN_MAP_ROW_STATIC(moduleIdx);
}

inline bool isMultiProtocolSelectable(int protocol)
{
  return protocol != MODULE_SUBTYPE_MULTI_SCANNER;
}

inline bool MULTIMODULE_PROTOCOL_KNOWN(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx)) {
    return false;
  }

  if (g_model.moduleData[moduleIdx].multi.rfProtocol < MODULE_SUBTYPE_MULTI_LAST) {
    return true;
  }

  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  if (status.isValid()) {
    return status.protocolValid();
  }

  return false;
}

inline bool MULTIMODULE_HAS_SUBTYPE(uint8_t moduleIdx)
{
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  int proto = g_model.moduleData[moduleIdx].multi.rfProtocol;

  if (proto == MODULE_SUBTYPE_MULTI_FRSKY) {
    return true;
  }

  if (status.isValid()) {
    TRACE("(%d) status.protocolSubNbr = %d", proto, status.protocolSubNbr);
    return status.protocolSubNbr > 0;
  }
  else
  {
    if (proto > MODULE_SUBTYPE_MULTI_LAST) {
      return true;
    }
    else {
      auto subProto = getMultiProtocolDefinition(proto);
      return subProto->subTypeString != nullptr;
    }
  }
}

inline uint8_t MULTIMODULE_RFPROTO_COLUMNS(uint8_t moduleIdx)
{
#if LCD_W < 212
  return (MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW);
#else
  return (MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t) 1 : 0);
#endif
}

inline uint8_t MULTIMODULE_HASOPTIONS(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return false;

  uint8_t protocol = g_model.moduleData[moduleIdx].multi.rfProtocol;
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);

  if (status.isValid())
    return status.optionDisp;

  if (protocol < MODULE_SUBTYPE_MULTI_LAST)
    return getMultiProtocolDefinition(protocol)->optionsstr != nullptr;

  return false;
}

#define MULTIMODULE_MODULE_ROWS(moduleIdx)      (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) && !IS_RX_MULTI(moduleIdx)) ? (uint8_t) 0 : HIDDEN_ROW, (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) && !IS_RX_MULTI(moduleIdx)) ? (uint8_t) 0 : HIDDEN_ROW, MULTI_DISABLE_CHAN_MAP_ROW(moduleIdx), // AUTOBIND, DISABLE TELEM, DISABLE CN.MAP
#define MULTIMODULE_TYPE_ROW(moduleIdx)         isModuleMultimodule(moduleIdx) ? MULTIMODULE_RFPROTO_COLUMNS(moduleIdx) : HIDDEN_ROW,
#define MULTIMODULE_STATUS_ROWS(moduleIdx)      isModuleMultimodule(moduleIdx) ? TITLE_ROW : HIDDEN_ROW, (isModuleMultimodule(moduleIdx) && getModuleSyncStatus(moduleIdx).isValid()) ? TITLE_ROW : HIDDEN_ROW,
#define MULTIMODULE_MODE_ROWS(moduleIdx)        (g_model.moduleData[moduleIdx].multi.customProto) ? (uint8_t) 3 : MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t)2 : (uint8_t)1
#define MULTIMODULE_TYPE_ROWS(moduleIdx)        isModuleMultimodule(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW,
#define MULTIMODULE_SUBTYPE_ROWS(moduleIdx)     isModuleMultimodule(moduleIdx) ? MULTIMODULE_RFPROTO_COLUMNS(moduleIdx) : HIDDEN_ROW,
#define MULTIMODULE_OPTIONS_ROW(moduleIdx)      (isModuleMultimodule(moduleIdx) && MULTIMODULE_HASOPTIONS(moduleIdx)) ? (uint8_t) 0: HIDDEN_ROW
#define MODULE_POWER_ROW(moduleIdx)            (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) || isModuleR9MNonAccess(moduleIdx) || isModuleAFHDS3(moduleIdx)) ? (isModuleR9MLiteNonPro(moduleIdx) ? (isModuleR9M_FCC_VARIANT(moduleIdx) ? READONLY_ROW : (uint8_t)0) : (uint8_t)0) : HIDDEN_ROW

#else
#define MULTIMODULE_TYPE_ROWS(moduleIdx)
#define MULTIMODULE_STATUS_ROWS(moduleIdx)
#define MULTIMODULE_MODULE_ROWS(moduleIdx)
#define MULTIMODULE_TYPE_ROW(moduleIdx)
#define MULTIMODULE_SUBTYPE_ROWS(moduleIdx)
#define MULTIMODULE_TYPE_ROWS(moduleIdx)
#define MULTIMODULE_MODE_ROWS(moduleIdx)        (uint8_t)0
#define MULTIMODULE_OPTIONS_ROW(moduleIdx)      HIDDEN_ROW
#define MODULE_POWER_ROW(moduleIdx)            isModuleR9MNonAccess(moduleIdx) || isModuleAFHDS3(moduleIdx) ? (isModuleR9MLiteNonPro(moduleIdx) ? (isModuleR9M_FCC_VARIANT(moduleIdx) ? READONLY_ROW : (uint8_t)0) : (uint8_t)0) : HIDDEN_ROW
#endif

#if defined(AFHDS3)
#define AFHDS3_PROTOCOL_ROW(moduleIdx)          isModuleAFHDS3(moduleIdx) ? uint8_t(0) : HIDDEN_ROW,
#define AFHDS3_MODE_ROWS(moduleIdx)             isModuleAFHDS3(moduleIdx) ? TITLE_ROW : HIDDEN_ROW, isModuleAFHDS3(moduleIdx) ? TITLE_ROW : HIDDEN_ROW, isModuleAFHDS3(moduleIdx) ? TITLE_ROW : HIDDEN_ROW,
#define AFHDS3_MODULE_ROWS(moduleIdx)           isModuleAFHDS3(moduleIdx) ? uint8_t(0) : HIDDEN_ROW, isModuleAFHDS3(moduleIdx) ? TITLE_ROW : HIDDEN_ROW,
#else
#define AFHDS3_PROTOCOL_ROW(moduleIdx)
#define AFHDS3_MODE_ROWS(moduleIdx)
#define AFHDS3_MODULE_ROWS(moduleIdx)
#endif

#define FAILSAFE_ROWS(moduleIdx)               isModuleFailsafeAvailable(moduleIdx) ? (g_model.moduleData[moduleIdx].failsafeMode==FAILSAFE_CUSTOM ? (uint8_t)1 : (uint8_t)0) : HIDDEN_ROW

inline uint8_t MODULE_OPTION_ROW(uint8_t moduleIdx) {
  if(isModuleR9MNonAccess(moduleIdx) || isModuleSBUS(moduleIdx))
    return TITLE_ROW;
  if(isModuleAFHDS3(moduleIdx))
    return HIDDEN_ROW;
  if(isModuleGhost(moduleIdx))
    return 0;
  return MULTIMODULE_OPTIONS_ROW(moduleIdx);
}

void editStickHardwareSettings(coord_t x, coord_t y, int idx, event_t event,
                               LcdFlags flags, uint8_t old_editMode);

const char * getMultiOptionTitleStatic(uint8_t moduleIdx);
const char *getMultiOptionTitle(uint8_t moduleIdx);

const char * writeScreenshot();

#endif // _GUI_COMMON_H_
