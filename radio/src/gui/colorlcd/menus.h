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

#ifndef _MENUS_H_
#define _MENUS_H_

#include "keys.h"

extern uint8_t menuCalibrationState;

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
  ICON_RADIO_GLOBAL_FUNCTIONS,
  ICON_RADIO_TRAINER,
  ICON_RADIO_HARDWARE,
  ICON_RADIO_CALIBRATION,
  ICON_RADIO_EDIT_THEME,
  ICON_RADIO_VERSION,
  ICON_MODEL,
  ICON_MODEL_SETUP,
  ICON_MODEL_HELI,
  ICON_MODEL_FLIGHT_MODES,
  ICON_MODEL_INPUTS,
  ICON_MODEL_MIXER,
  ICON_MODEL_NOTES,
  ICON_MODEL_OUTPUTS,
  ICON_MODEL_CURVES,
  ICON_MODEL_GVARS,
  ICON_MODEL_LOGICAL_SWITCHES,
  ICON_MODEL_SPECIAL_FUNCTIONS,
  ICON_MODEL_LUA_SCRIPTS,
  ICON_MODEL_TELEMETRY,
  ICON_MODEL_SELECT,
  ICON_MODEL_SELECT_CATEGORY,
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

typedef int (*FnFuncP) (int x);
void drawFunction(FnFuncP fn, int x, int y, int width);

#endif // _MENUS_H_
