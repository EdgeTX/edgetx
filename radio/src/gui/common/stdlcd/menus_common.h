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

#include "edgetx_types.h"
#include "keys.h"
#include "feature_control.h"

#define COPY_MODE 1
#define MOVE_MODE 2
#define EDIT_MODE_INIT 0

typedef int8_t horzpos_t;
typedef uint16_t vertpos_t;

typedef void (* MenuHandlerFunc)(event_t event);
typedef bool (* MenuEnabledFunc)();

typedef struct _MenuHandler {
  MenuHandlerFunc menuFunc;
  MenuEnabledFunc enabledFunc;

  bool isEnabled() const
  {
    if (enabledFunc)
      return enabledFunc();
    return true;
  }
} MenuHandler;

extern tmr10ms_t menuEntryTime;
extern vertpos_t menuVerticalPosition;
extern horzpos_t menuHorizontalPosition;
extern vertpos_t menuVerticalOffset;
extern uint8_t menuCalibrationState;
extern MenuHandlerFunc menuHandlers[5];
extern uint8_t menuVerticalPositions[4];
extern uint8_t menuVerticalOffsets[4];
extern uint8_t menuLevel;
extern event_t menuEvent;

void chainMenu(MenuHandlerFunc newMenu);
void pushMenu(MenuHandlerFunc newMenu);
void popMenu();
void abortPopMenu();

inline MenuHandlerFunc lastPopMenu()
{
  return menuHandlers[menuLevel + 1];
}

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
void menuChannelsViewCommon(event_t event);
void menuChannelsView(event_t event);
void menuMainView(event_t event);
void menuViewTelemetry(event_t event);
void menuSpecialFunctions(event_t event, CustomFunctionData * functions, CustomFunctionsContext * functionsContext);
void menuStatisticsView(event_t event);
void menuStatisticsDebug(event_t event);
void menuStatisticsDebug2(event_t event);
void menuAboutView(event_t event);

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

enum MenuModelIndexes {
  MENU_MODEL_SELECT,
  MENU_MODEL_SETUP,
  CASE_HELI(MENU_MODEL_HELI)
  CASE_FLIGHT_MODES(MENU_MODEL_FLIGHT_MODES)
  MENU_MODEL_INPUTS,
  MENU_MODEL_MIXES,
  MENU_MODEL_OUTPUTS,
  MENU_MODEL_CURVES,
#if LCD_W >= 212
  CASE_GVARS(MENU_MODEL_GVARS)
#endif
  MENU_MODEL_LOGICAL_SWITCHES,
  MENU_MODEL_SPECIAL_FUNCTIONS,
#if defined(LUA_MODEL_SCRIPTS)
  MENU_MODEL_CUSTOM_SCRIPTS,
#endif
  MENU_MODEL_TELEMETRY,
  MENU_MODEL_DISPLAY,
  MENU_MODEL_PAGES_COUNT
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
void menuModelLogicalSwitches(event_t event);
void menuModelSpecialFunctions(event_t event);
void menuModelCustomScripts(event_t event);
void menuModelTelemetry(event_t event);
void menuModelSensor(event_t event);
void menuModelDisplay(event_t event);
#if defined(USBJ_EX)
void menuModelUSBJoystick(event_t event);
#endif

extern const MenuHandler menuTabGeneral[MENU_RADIO_PAGES_COUNT];
extern const MenuHandler menuTabModel[MENU_MODEL_PAGES_COUNT];

extern const unsigned char sticks[];

extern uint8_t s_copyMode;
extern int8_t s_copySrcRow;
extern int8_t s_copyTgtOfs;
extern uint8_t s_currIdx;
extern uint8_t s_currIdxSubMenu;
extern mixsrc_t s_currSrcRaw;
extern uint16_t s_currScale;
extern uint8_t s_copySrcIdx;
extern uint8_t s_copySrcCh;
extern int8_t s_currCh;
extern uint8_t s_maxLines;

// Temporary no highlight
extern uint8_t noHighlightCounter;
#define NO_HIGHLIGHT()        (noHighlightCounter > 0)
#define START_NO_HIGHLIGHT()  do { noHighlightCounter = 25; } while(0)

void menuTextView(event_t event);
void pushMenuTextView(const char *filename);
void pushModelNotes();
void readModelNotes();

uint8_t getExposCount();
void insertExpo(uint8_t idx);
void deleteExpo(uint8_t idx);

uint8_t switchToMix(uint8_t source);

void drawSplash();
void doMainScreenGraphics();
void drawAlertBox(const char * title, const char * text, const char * action);

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif

#define IS_MAIN_VIEW_DISPLAYED()       menuHandlers[0] == menuMainView
#define IS_TELEMETRY_VIEW_DISPLAYED()  menuHandlers[0] == menuViewTelemetry
