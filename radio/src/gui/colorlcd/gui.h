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

#ifndef _GUI_H_
#define _GUI_H_

#include "gui_common.h"
#include "lcd.h"
#include "menus.h"
#include "popups.h"
#include "draw_functions.h"
#include "bitmaps.h"
#include "theme.h"
#include "libopenui_config.h"

#define LOAD_MODEL_BITMAP()

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value, uint8_t attr);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif

// Curve functions
coord_t getCurveYCoord(FnFuncP fn, int x, int width);
void drawFunction(FnFuncP fn, int offset);
void drawCurveVerticalScale(int x);
void drawCurveHorizontalScale();
void drawCurveCoord(int x, int y, const char * text, bool active=false);
void drawCurvePoint(int x, int y, LcdFlags color);

extern WidgetsContainer * customScreens[MAX_CUSTOM_SCREENS];

void drawAlertBox(const char * title, const char * text, const char * action);
void showAlertBox(const char * title, const char * text, const char * action, uint8_t sound);

#define IS_MAIN_VIEW_DISPLAYED()       menuHandlers[0] == menuMainView
#define IS_TELEMETRY_VIEW_DISPLAYED()  false
#define IS_OTHER_VIEW_DISPLAYED()      false

#endif // _GUI_H_
