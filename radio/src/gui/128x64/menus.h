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

#include "keys.h"
#include "common/stdlcd/menus.h"
#include "common/stdlcd/features.h"

#if defined(PCBTARANIS)
#define NAVIGATION_LINE_BY_LINE        0x40
#define IS_LINE_SELECTED(sub, k)       ((sub)==(k) && menuHorizontalPosition < 0)
#else
#define NAVIGATION_LINE_BY_LINE        0
#define IS_LINE_SELECTED(sub, k)       (false)
#endif

inline bool isRadioMenuDisplayed()
{
  return menuVerticalPositions[0] == 1;
}

inline bool isModelMenuDisplayed()
{
  return menuVerticalPositions[0] == 0;
}

void onMainViewMenu(const char * result);
void menuFirstCalib(event_t event);
void menuMainView(event_t event);
void menuViewTelemetry(event_t event);
void menuSpecialFunctions(event_t event, CustomFunctionData * functions, CustomFunctionsContext * functionsContext);

enum MenuRadioIndexes
{
#if defined(RADIO_TOOLS)
  MENU_RADIO_TOOLS,
#endif
  MENU_RADIO_SD_MANAGER,
  MENU_RADIO_SETUP,
  MENU_RADIO_SPECIAL_FUNCTIONS,
  MENU_RADIO_TRAINER,
  MENU_RADIO_HARDWARE,
  MENU_RADIO_VERSION,
  MENU_RADIO_PAGES_COUNT
};

void menuRadioSetup(event_t event);
void menuRadioSdManager(event_t event);
void menuRadioSpecialFunctions(event_t event);
void menuRadioTrainer(event_t event);
void menuRadioVersion(event_t event);
void menuRadioDiagKeys(event_t event);
void menuRadioDiagAnalogs(event_t event);
void menuRadioDiagFS(event_t event);
void menuRadioHardware(event_t event);
void menuRadioTools(event_t event);
void menuRadioSpectrumAnalyser(event_t event);
void menuRadioPowerMeter(event_t event);
void menuRadioCalibration(event_t event);
void menuGhostModuleConfig(event_t event);

extern bool radioGFEnabled();
extern bool radioTrainerEnabled();

extern const MenuHandler menuTabGeneral[MENU_RADIO_PAGES_COUNT];

enum MenuModelIndexes {
  MENU_MODEL_SELECT,
  MENU_MODEL_SETUP,
  CASE_HELI(MENU_MODEL_HELI)
  CASE_FLIGHT_MODES(MENU_MODEL_FLIGHT_MODES)
  MENU_MODEL_INPUTS,
  MENU_MODEL_MIXES,
  MENU_MODEL_OUTPUTS,
  MENU_MODEL_CURVES,
  MENU_MODEL_LOGICAL_SWITCHES,
  MENU_MODEL_SPECIAL_FUNCTIONS,
#if defined(LUA_MODEL_SCRIPTS)
  MENU_MODEL_CUSTOM_SCRIPTS,
#endif
  MENU_MODEL_TELEMETRY,
  MENU_MODEL_DISPLAY,
  MENU_MODEL_PAGES_COUNT
};

void menuModelSelect(event_t event);
void menuModelSetup(event_t event);
void menuModelFailsafe(event_t event);
void menuModelModuleOptions(event_t event);
void menuModelReceiverOptions(event_t event);
void menuModelHeli(event_t event);
void menuModelFlightModesAll(event_t event);
void menuModelExpoOne(event_t event);
void menuModelExposAll(event_t event);
void menuModelMixOne(event_t event);
void menuModelMixAll(event_t event);
void menuModelLimits(event_t event);
void menuModelCurvesAll(event_t event);
void menuModelCurveOne(event_t event);
void menuModelGVars(event_t event);
void menuModelLogicalSwitches(event_t event);
void menuModelSpecialFunctions(event_t event);
void menuModelCustomScripts(event_t event);
void menuModelTelemetry(event_t event);
void menuModelSensor(event_t event);
void menuModelDisplay(event_t event);
void menuModelTemplates(event_t event);
void menuModelGVarOne(event_t event);
#if defined(USBJ_EX)
void menuModelUSBJoystick(event_t event);
#endif

extern bool modelHeliEnabled();
extern bool modelFMEnabled();
extern bool modelCurvesEnabled();
extern bool modelLSEnabled();
extern bool modelSFEnabled();
extern bool modelCustomScriptsEnabled();
extern bool modelTelemetryEnabled();

extern const MenuHandler menuTabModel[MENU_MODEL_PAGES_COUNT];

void menuStatisticsView(event_t event);
void menuStatisticsDebug(event_t event);
void menuStatisticsDebug2(event_t event);
void menuAboutView(event_t event);
