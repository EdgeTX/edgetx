/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _MENUS_H_
#define _MENUS_H_

#include "keys.h"

#define MIXES_2ND_COLUMN       140

extern int menuVerticalPosition;
extern int menuHorizontalPosition;
extern int menuVerticalOffset;
extern uint8_t menuCalibrationState;

// Temporary no highlight

typedef bool (*MenuHandlerFunc)(event_t event);

extern MenuHandlerFunc menuHandlers[5];
extern uint8_t menuVerticalPositions[4];
extern uint8_t menuLevel;
extern event_t menuEvent;

void chainMenu(MenuHandlerFunc newMenu);
void pushMenu(MenuHandlerFunc newMenu);
void popMenu();
void abortPopMenu();

inline bool isModelMenuDisplayed()
{
  return menuVerticalPositions[0] == 0;
}

enum MenuIcons {
  ICON_OPENTX,
#if defined(HARDWARE_TOUCH)
  ICON_NEXT,
  ICON_BACK,
#endif
  ICON_RADIO,
  ICON_RADIO_SETUP,
  ICON_RADIO_SD_MANAGER,
  ICON_RADIO_TOOLS,
  ICON_RADIO_SPECTRUM_ANALYSER,
  ICON_RADIO_GLOBAL_FUNCTIONS,
  ICON_RADIO_TRAINER,
  ICON_RADIO_HARDWARE,
  ICON_RADIO_CALIBRATION,
  ICON_RADIO_VERSION,
  ICON_MODEL,
  ICON_MODEL_SETUP,
  ICON_MODEL_HELI,
  ICON_MODEL_FLIGHT_MODES,
  ICON_MODEL_INPUTS,
  ICON_MODEL_MIXER,
  ICON_MODEL_OUTPUTS,
  ICON_MODEL_CURVES,
  ICON_MODEL_GVARS,
  ICON_MODEL_LOGICAL_SWITCHES,
  ICON_MODEL_SPECIAL_FUNCTIONS,
  ICON_MODEL_LUA_SCRIPTS,
  ICON_MODEL_TELEMETRY,
  ICON_THEME,
  ICON_THEME_SETUP,
  ICON_THEME_VIEW1,
  ICON_THEME_VIEW2,
  ICON_THEME_VIEW3,
  ICON_THEME_VIEW4,
  ICON_THEME_VIEW5,
  ICON_THEME_ADD_VIEW,
  ICON_STATS,
  ICON_STATS_THROTTLE_GRAPH,
  ICON_STATS_TIMERS,
  ICON_STATS_ANALOGS,
  ICON_STATS_DEBUG,
  ICON_MONITOR,
  ICON_MONITOR_CHANNELS1,
  ICON_MONITOR_CHANNELS2,
  ICON_MONITOR_CHANNELS3,
  ICON_MONITOR_CHANNELS4,
  ICON_MONITOR_LOGICAL_SWITCHES,
  MENUS_ICONS_COUNT
};

enum EnumTabModel {
  MENU_MODEL_SETUP,
  CASE_HELI(MENU_MODEL_HELI)
  CASE_FLIGHT_MODES(MENU_MODEL_FLIGHT_MODES)
  MENU_MODEL_INPUTS,
  MENU_MODEL_MIXES,
  MENU_MODEL_OUTPUTS,
  MENU_MODEL_CURVES,
  CASE_GVARS(MENU_MODEL_GVARS)
  MENU_MODEL_LOGICAL_SWITCHES,
  MENU_MODEL_SPECIAL_FUNCTIONS,
#if defined(LUA_MODEL_SCRIPTS)
  MENU_MODEL_CUSTOM_SCRIPTS,
#endif
  MENU_MODEL_TELEMETRY,
  MENU_MODEL_PAGES_COUNT
};

const uint8_t MODEL_ICONS[MENU_MODEL_PAGES_COUNT + 1] = {
  ICON_MODEL,
  ICON_MODEL_SETUP,
  CASE_HELI(ICON_MODEL_HELI)
  CASE_FLIGHT_MODES(ICON_MODEL_FLIGHT_MODES)
  ICON_MODEL_INPUTS,
  ICON_MODEL_MIXER,
  ICON_MODEL_OUTPUTS,
  ICON_MODEL_CURVES,
  CASE_GVARS(ICON_MODEL_GVARS)
  ICON_MODEL_LOGICAL_SWITCHES,
  ICON_MODEL_SPECIAL_FUNCTIONS,
#if defined(LUA_MODEL_SCRIPTS)
  ICON_MODEL_LUA_SCRIPTS,
#endif
  ICON_MODEL_TELEMETRY
};

const uint8_t STATS_ICONS[] = {
  ICON_STATS,
  ICON_STATS_THROTTLE_GRAPH,
  ICON_STATS_DEBUG,
  ICON_STATS_ANALOGS,
#if defined(DEBUG_TRACE_BUFFER)
  ICON_STATS_TIMERS
#endif
};

const uint8_t MONITOR_ICONS[] = {
  ICON_MONITOR,
  ICON_MONITOR_CHANNELS1,
  ICON_MONITOR_CHANNELS2,
  ICON_MONITOR_CHANNELS3,
  ICON_MONITOR_CHANNELS4,
  ICON_MONITOR_LOGICAL_SWITCHES
};

bool menuModelSetup(event_t event);
bool menuModelFailsafe(event_t event);
bool menuModelHeli(event_t event);
bool menuModelFlightModesAll(event_t event);
bool menuModelExposAll(event_t event);
bool menuModelMixAll(event_t event);
bool menuModelLimits(event_t event);
bool menuModelCurvesAll(event_t event);
bool menuModelCurveOne(event_t event);
bool menuModelGVars(event_t event);
bool menuModelLogicalSwitches(event_t event);
bool menuModelSpecialFunctions(event_t event);
bool menuModelCustomScripts(event_t event);
bool menuModelTelemetryFrsky(event_t event);
bool menuModelSensor(event_t event);
bool menuModelExpoOne(event_t event);

extern const MenuHandlerFunc menuTabModel[MENU_MODEL_PAGES_COUNT];

enum EnumTabRadio {
  MENU_RADIO_SETUP,
  MENU_RADIO_SD_MANAGER,
  MENU_RADIO_TOOLS,
  MENU_RADIO_SPECIAL_FUNCTIONS,
  MENU_RADIO_TRAINER,
  MENU_RADIO_HARDWARE,
  MENU_RADIO_VERSION,
  MENU_RADIO_PAGES_COUNT
};

const uint8_t RADIO_ICONS[MENU_RADIO_PAGES_COUNT + 1] = {
  ICON_RADIO,
  ICON_RADIO_SETUP,
  ICON_RADIO_SD_MANAGER,
  ICON_RADIO_TOOLS,
  ICON_RADIO_GLOBAL_FUNCTIONS,
  ICON_RADIO_TRAINER,
  ICON_RADIO_HARDWARE,
  ICON_RADIO_VERSION
};

bool menuRadioSetup(event_t event);
bool menuRadioSdManager(event_t event);
bool menuRadioTools(event_t event);
bool menuRadioSpecialFunctions(event_t event);
bool menuRadioTrainer(event_t event);
bool menuRadioVersion(event_t event);
bool menuRadioHardware(event_t event);
bool menuRadioCalibration(event_t event);

extern const MenuHandlerFunc menuTabGeneral[MENU_RADIO_PAGES_COUNT];

enum MenuRadioIndexes
{
  e_StatsGraph,
  e_StatsDebug,
  e_StatsAnalogs,
#if defined(DEBUG_TRACE_BUFFER)
  e_StatsTraces,
#endif
};

bool menuStatsGraph(event_t event);
bool menuStatsDebug(event_t event);
bool menuStatsAnalogs(event_t event);
bool menuStatsTraces(event_t event);

enum EnumTabMonitors
{
  e_MonChannelsFirst,
  e_MonChannels1 = e_MonChannelsFirst,
  e_MonChannels2,
  e_MonChannels3,
  e_MonChannels4,
  e_MonLogicalSwitches,
  e_MonTabChannelsPagesCount
};

extern const MenuHandlerFunc menuTabMonitors[e_MonTabChannelsPagesCount];
extern uint8_t lastMonitorPage;
extern void drawSingleMixerBar(coord_t, coord_t, coord_t, coord_t, uint8_t);
extern void drawSingleOutputBar(coord_t, coord_t, coord_t, coord_t, uint8_t);

extern const MenuHandlerFunc menuTabScreensSetup[1+MAX_CUSTOM_SCREENS] ;

bool menuFirstCalib(event_t event);
bool menuMainView(event_t event);
bool menuSpecialFunctions(event_t event, CustomFunctionData * functions, CustomFunctionsContext & functionsContext);
bool menuModelSelect(event_t event);
bool menuAboutView(event_t event);
bool menuMainViewChannelsMonitor(event_t event);
bool menuTextView(event_t event);
bool menuScreensTheme(event_t event);
bool menuRadioGhostModuleConfig(event_t event);
bool menuRadioPowerMeter(event_t event);

extern int8_t checkIncDec_Ret;  // global helper vars

#define EDIT_SELECT_FIELD  0
#define EDIT_MODIFY_FIELD  1
#define EDIT_MODIFY_STRING 2
extern int8_t s_editMode;       // global editmode

// checkIncDec flags
#define EE_GENERAL                     0x01
#define EE_MODEL                       0x02
#define NO_INCDEC_MARKS                0x04
#define INCDEC_SWITCH                  0x08
#define INCDEC_SOURCE                  0x10
#define INCDEC_REP10                   0x40
#define NO_DBLKEYS                     0x80

// mawrow special values
//#define READONLY_ROW                   ((uint8_t)-1)
//#define TITLE_ROW                      READONLY_ROW
//#define ORPHAN_ROW                     ((uint8_t)-2)
//#define HIDDEN_ROW                     ((uint8_t)-3)
//#define NAVIGATION_LINE_BY_LINE        0x40
//#define CURSOR_ON_LINE()               (menuHorizontalPosition<0)

extern uint8_t s_currIdxSubMenu;

extern const char * warningText;
extern const char * warningInfoText;
extern uint8_t         warningInfoLength;
extern uint8_t         warningResult;
extern uint8_t         warningType;

#define COPY_MODE 1
#define MOVE_MODE 2
extern uint8_t s_copyMode;
extern int8_t s_copySrcRow;
extern int8_t s_copyTgtOfs;
extern uint8_t s_currIdx;
extern int8_t s_currCh;
extern uint8_t s_copySrcIdx;
extern uint8_t s_copySrcCh;

uint8_t getExposCount();
void deleteExpo(uint8_t idx);
void insertExpo(uint8_t idx, uint8_t input);

uint8_t getMixesCount();
void deleteMix(uint8_t idx);
void insertMix(uint8_t idx);
void copyMix(uint8_t source, uint8_t dest, int8_t ch);

#define MENU_X                 80
#define MENU_W                 LCD_W-(2*MENU_X)
#define POPUP_X                120
#define POPUP_Y                70
#define WARNING_LINE_LEN       32
#define WARNING_LINE_X         (POPUP_X+66)
#define WARNING_LINE_Y         (POPUP_Y+9)

void copySelection(char * dst, const char * src, uint8_t size);

void showMessageBox(const char * title);
void runPopupWarning(event_t event);

extern void (* popupFunc)(event_t event);
extern uint8_t warningInfoFlags;

#define DISPLAY_WARNING(evt)                (*popupFunc)(evt)
#define POPUP_INPUT(s, func)           (warningText = s, popupFunc = func)
#define POPUP_MENU_MAX_LINES           12
#define MENU_MAX_DISPLAY_LINES         9
#define MENU_LINE_LENGTH               (LEN_MODEL_NAME+12)

extern const char * popupMenuItems[POPUP_MENU_MAX_LINES];
extern uint16_t popupMenuItemsCount;
extern uint16_t popupMenuOffset;
enum {
  MENU_OFFSET_INTERNAL,
  MENU_OFFSET_EXTERNAL
};
extern uint8_t popupMenuOffsetType;
extern uint8_t popupMenuSelectedItem;
const char * runPopupMenu(event_t event);
extern void (*popupMenuHandler)(const char * result);
void pushMenuTextView(const char * filename);
void pushModelNotes();
void readModelNotes();

typedef int (*FnFuncP) (int x);
void drawFunction(FnFuncP fn, int x, int y, int width);

void onSourceLongEnterPress(const char *result);

uint8_t switchToMix(uint8_t source);

#endif // _MENUS_H_
