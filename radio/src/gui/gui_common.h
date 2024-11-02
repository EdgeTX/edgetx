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
#else
  #define CASE_ROTARY_ENCODER(x)
#endif

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
extern uint8_t MENU_FIRST_LINE_EDIT(const uint8_t * horTab, uint8_t horTabMax);
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
  MixesContext
};

int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable);

bool isInputAvailable(int input);
bool isSourceAvailableInInputs(int source);
bool isThrottleSourceAvailable(int source);
bool isLogicalSwitchAvailable(int index);
bool isAssignableFunctionAvailable(int function);
bool isSourceAvailable(int source);
bool isTimerSourceAvailable(int source);
bool isSourceAvailableInGlobalFunctions(int source);
bool isSourceAvailableInCustomSwitches(int source);
bool isSourceAvailableInResetSpecialFunction(int index);
bool isSourceAvailableInGlobalResetSpecialFunction(int index);
bool isSwitchAvailable(int swtch, SwitchContext context);
bool isSerialModeAvailable(uint8_t port_nr, int mode);
bool isSwitchAvailableInLogicalSwitches(int swtch);
bool isSwitchAvailableInCustomFunctions(int swtch);
bool isSwitchAvailableForArming(int swtch);
bool isSwitchAvailableInMixes(int swtch);
bool isPxx2IsrmChannelsCountAllowed(int channels);
bool isModuleUsingSport(uint8_t moduleBay, uint8_t moduleType);
bool isTrainerUsingModuleBay();
bool isExternalModuleAvailable(int moduleType);
bool isInternalModuleAvailable(int moduleType);
bool isInternalModuleSupported(int moduleType);
bool isRfProtocolAvailable(int protocol);
bool isTrainerModeAvailable(int mode);
bool isAssignableFunctionAvailable(int function, bool modelFunctions);
bool isPotTypeAvailable(uint8_t type);
bool isFlexSwitchSourceValid(int source);
bool getPotInversion(int index);
void setPotInversion(int index, bool value);
uint8_t getPotType(int index);
void setPotType(int index, int value);

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
bool isVarioSensorAvailable(int sensor);
bool hasSportPower();

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
void drawSourceCustomValue(coord_t x, coord_t y, mixsrc_t channel, int32_t val, LcdFlags flags=0);
void drawSourceValue(coord_t x, coord_t y, source_t channel, LcdFlags flags=0);

// model_setup Defines that are used in all uis in the same way
#define IF_INTERNAL_MODULE_ON(x)                  (IS_INTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)
#define IF_MODULE_ON(moduleIndex, x)              (IS_MODULE_ENABLED(moduleIndex) ? (uint8_t)(x) : HIDDEN_ROW)

extern uint8_t MODULE_BIND_ROWS(int moduleIdx);
extern uint8_t MODULE_CHANNELS_ROWS(int moduleIdx);

#if defined(EXTERNAL_ANTENNA) && defined(INTERNAL_MODULE_PXX1)
void onAntennaSwitchConfirm(const char * result);
void checkExternalAntenna();
void onAntennaSelection(const char* result);
#endif

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
extern uint8_t MULTI_DISABLE_CHAN_MAP_ROW_STATIC(uint8_t moduleIdx);
extern uint8_t MULTI_DISABLE_CHAN_MAP_ROW(uint8_t moduleIdx);
extern bool MULTIMODULE_PROTOCOL_KNOWN(uint8_t moduleIdx);
extern bool MULTIMODULE_HAS_SUBTYPE(uint8_t moduleIdx);
extern uint8_t MULTIMODULE_RFPROTO_COLUMNS(uint8_t moduleIdx);
extern uint8_t MULTIMODULE_HASOPTIONS(uint8_t moduleIdx);

#if defined(MANUFACTURER_FRSKY)
  #define MULTIMODULE_MODULE_ROWS(moduleIdx)      (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) && !IS_RX_MULTI(moduleIdx)) ? (uint8_t) 0 : HIDDEN_ROW, (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) && !IS_RX_MULTI(moduleIdx)) ? (uint8_t) 0 : HIDDEN_ROW, MULTI_DISABLE_CHAN_MAP_ROW(moduleIdx), // AUTOBIND, DISABLE TELEM, DISABLE CN.MAP
#else
  #define MULTIMODULE_MODULE_ROWS(moduleIdx)      (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) && !IS_RX_MULTI(moduleIdx)) ? (uint8_t) 0 : HIDDEN_ROW, MULTI_DISABLE_CHAN_MAP_ROW(moduleIdx), // AUTOBIND, DISABLE CN.MAP
#endif
#define MULTIMODULE_DSM_CLONED_RAW(moduleIdx)   isMultiProtocolDSMCloneAvailable(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW
#define MULTIMODULE_TYPE_ROW(moduleIdx)         isModuleMultimodule(moduleIdx) ? MULTIMODULE_RFPROTO_COLUMNS(moduleIdx) : HIDDEN_ROW,
#define MULTIMODULE_STATUS_ROWS(moduleIdx)      isModuleMultimodule(moduleIdx) ? TITLE_ROW : HIDDEN_ROW, (isModuleMultimodule(moduleIdx) && getModuleSyncStatus(moduleIdx).isValid()) ? TITLE_ROW : HIDDEN_ROW,
#define MULTIMODULE_MODE_ROWS(moduleIdx)        (g_model.moduleData[moduleIdx].multi.customProto) ? (uint8_t) 3 : MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t)2 : (uint8_t)1
#define MULTIMODULE_TYPE_ROWS(moduleIdx)        isModuleMultimodule(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW,
#define MULTIMODULE_SUBTYPE_ROWS(moduleIdx)     isModuleMultimodule(moduleIdx) ? MULTIMODULE_RFPROTO_COLUMNS(moduleIdx) : HIDDEN_ROW,
#define MULTIMODULE_OPTIONS_ROW(moduleIdx)      (isModuleMultimodule(moduleIdx) && MULTIMODULE_HASOPTIONS(moduleIdx)) ? (uint8_t) 0: HIDDEN_ROW
#define MODULE_POWER_ROW(moduleIdx)            (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx) || isModuleR9MNonAccess(moduleIdx) || isModuleAFHDS3(moduleIdx)) ? (isModuleR9MLiteNonPro(moduleIdx) ? (isModuleR9M_FCC_VARIANT(moduleIdx) ? READONLY_ROW : (uint8_t)0) : (uint8_t)0) : HIDDEN_ROW

#else
#define MULTIMODULE_DSM_CLONED_RAW(moduleIdx)
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

#define FAILSAFE_ROW(moduleIdx)               isModuleFailsafeAvailable(moduleIdx) ? (g_model.moduleData[moduleIdx].failsafeMode==FAILSAFE_CUSTOM ? (uint8_t)1 : (uint8_t)0) : HIDDEN_ROW

extern uint8_t MODULE_OPTION_ROW(uint8_t moduleIdx);

void editStickHardwareSettings(coord_t x, coord_t y, int idx, event_t event,
                               LcdFlags flags, uint8_t old_editMode);

const char * getMultiOptionTitleStatic(uint8_t moduleIdx);
const char *getMultiOptionTitle(uint8_t moduleIdx);

const char * writeScreenshot();

uint8_t expandableSection(coord_t y, const char* title, uint8_t value, uint8_t attr, event_t event);
