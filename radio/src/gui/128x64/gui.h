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

#include "gui_common.h"
#include "menus.h"
#include "popups.h"

#include "navigation/navigation.h"
#include "common/stdlcd/draw_functions.h"

#define MENUS_SCROLLBAR_WIDTH          0

#if defined(NAVIGATION_X7)
  #define HEADER_LINE                  0
  #define HEADER_LINE_COLUMNS
#else
  #define HEADER_LINE                  1
  #define HEADER_LINE_COLUMNS          0,
#endif

#define NUM_BODY_LINES                 (LCD_LINES-1)
#define TEXT_VIEWER_LINES              NUM_BODY_LINES
#define MENU_HEADER_HEIGHT             FH

#define CURVE_SIDE_WIDTH               (LCD_H/2-2)
#define CURVE_CENTER_X                 (LCD_W-CURVE_SIDE_WIDTH-3)
#define CURVE_CENTER_Y                 (LCD_H/2)

#define MIXES_2ND_COLUMN               (10*FW)

// Temporary no highlight
extern uint8_t noHighlightCounter;
#define NO_HIGHLIGHT()        (noHighlightCounter > 0)
#define START_NO_HIGHLIGHT()  do { noHighlightCounter = 25; } while(0)

void drawSlider(coord_t x, coord_t y, uint8_t width, uint8_t value, uint8_t max, uint8_t attr);
void drawSlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr);

void title(const char * s);

typedef int choice_t;

choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event);
choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX);
choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX,
                    IsValueAvailable isValueAvailable);

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label,
                     LcdFlags attr, event_t event);
uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label,
                     LcdFlags attr, event_t event, coord_t lblX);

swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr,
                   event_t event);

uint16_t editSrcVarFieldValue(coord_t x, coord_t y, const char* title, uint16_t value,
                              int16_t min, int16_t max, LcdFlags attr, event_t event,
                              IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

#if defined(GVARS)

#define GVAR_MENU_ITEM(x, y, v, min, max, attr, editflags, event)       \
  editGVarFieldValue(x, y, v, min, max, attr, editflags, event)

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min,
                           int16_t max, LcdFlags attr, uint8_t editflags,
                           event_t event);

void drawGVarValue(coord_t x, coord_t y, uint8_t gvar, gvar_t value,
                   LcdFlags flags = 0);

void editGVarValue(coord_t x, coord_t y, event_t event, uint8_t gvar,
                   uint8_t flightMode, LcdFlags flags);

#define displayGVar(x, y, v, min, max)          \
  GVAR_MENU_ITEM(x, y, v, min, max, 0, 0, 0)

#else // GVARS

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min,
                           int16_t max, LcdFlags attr, event_t event);

#define GVAR_MENU_ITEM(x, y, v, min, max, attr, editflags, event)       \
  editGVarFieldValue(x, y, v, min, max, attr, event)

#define displayGVar(x, y, v, min, max) lcdDraw8bitsNumber(x, y, v)

#endif

void editName(coord_t x, coord_t y, char *name, uint8_t size, event_t event,
              uint8_t active, LcdFlags attr, uint8_t old_editMode);

void editSingleName(coord_t x, coord_t y, const char *label, char *name,
                    uint8_t size, event_t event, uint8_t active,
                    uint8_t old_editMode, coord_t lblX = 0);

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay, uint8_t prec);

#define EDIT_DELAY(x, y, event, attr, str, delay, prec) editDelay(y, event, attr, str, delay, prec)

#define COPY_MODE 1
#define MOVE_MODE 2

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

#define STATUS_LINE_LENGTH           32
extern char statusLineMsg[STATUS_LINE_LENGTH];
void showStatusLine();
void drawStatusLine();

void menuTextView(event_t event);
void pushMenuTextView(const char *filename);
void pushModelNotes();
void readModelNotes();

void menuChannelsView(event_t event);
void menuChannelsViewCommon(event_t event);

#define EDIT_MODE_INIT 0

extern uint8_t editNameCursorPos;

uint8_t getExposCount();
void insertExpo(uint8_t idx);
void deleteExpo(uint8_t idx);

void onSwitchLongEnterPress(const char *result);
void onSourceLongEnterPress(const char *result);

uint8_t switchToMix(uint8_t source);

void drawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr);

extern const unsigned char sticks[] ;

void drawSplash();
void drawScreenIndex(uint8_t index, uint8_t count, uint8_t attr);
void drawStick(coord_t centrex, int16_t xval, int16_t yval);
void drawWheel(coord_t centrex, int16_t wval);
void drawThrottle(coord_t centrex, int16_t tval);
void drawPotsBars();
void doMainScreenGraphics();

void drawProgressScreen(const char * title, const char * message, int num, int den);
void drawSleepBitmap();

void drawVerticalScrollbar(coord_t x, coord_t y, coord_t h, uint16_t offset, uint16_t count, uint8_t visible);
void drawGauge(coord_t x, coord_t y, coord_t w, coord_t h, int32_t val, int32_t max);

void drawAlertBox(const char * title, const char * text, const char * action);
void showAlertBox(const char * title, const char * text, const char * action , uint8_t sound);

#define SET_SCROLLBAR_X(x)
#define LOAD_MODEL_BITMAP()

#define IS_MAIN_VIEW_DISPLAYED()       menuHandlers[0] == menuMainView
#define IS_TELEMETRY_VIEW_DISPLAYED()  menuHandlers[0] == menuViewTelemetry
#define IS_OTHER_VIEW_DISPLAYED()      menuHandlers[0] == menuChannelsView

void editCurveRef(coord_t x, coord_t y, CurveRef & curve, event_t event, LcdFlags flags,
                  IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif
