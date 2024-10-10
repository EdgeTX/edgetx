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
#include "lcd.h"
#include "menus.h"
#include "popups.h"

#include "navigation/navigation.h"
#include "common/stdlcd/draw_functions.h"

#define HEADER_LINE                    0
#define HEADER_LINE_COLUMNS

#define NUM_BODY_LINES                 (LCD_LINES-1)
#define TEXT_VIEWER_LINES              NUM_BODY_LINES
#define MENU_HEADER_HEIGHT             FH

#define MODEL_BITMAP_WIDTH             64
#define MODEL_BITMAP_HEIGHT            32
#define MODEL_BITMAP_SIZE              BITMAP_BUFFER_SIZE(MODEL_BITMAP_WIDTH, MODEL_BITMAP_HEIGHT)
#define LOAD_MODEL_BITMAP()            loadModelBitmap(g_model.header.bitmap, modelBitmap)

#define CURVE_SIDE_WIDTH               (LCD_H/2-2)
#define CURVE_CENTER_X                 (LCD_W-CURVE_SIDE_WIDTH-3)
#define CURVE_CENTER_Y                 (LCD_H/2)

#define MIXES_2ND_COLUMN               (18*FW)

#define MENUS_SCROLLBAR_WIDTH          2

extern uint8_t modelBitmap[MODEL_BITMAP_SIZE];
bool loadModelBitmap(char * name, uint8_t * bitmap);

// Temporary no highlight
extern uint8_t noHighlightCounter;
#define NO_HIGHLIGHT()        (noHighlightCounter > 0)
#define START_NO_HIGHLIGHT()  do { noHighlightCounter = 25; } while(0)

void drawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr);
void drawSlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr);
void drawSplash();
void drawScreenIndex(uint8_t index, uint8_t count, uint8_t attr);
void drawVerticalScrollbar(coord_t x, coord_t y, coord_t h, uint16_t offset, uint16_t count, uint8_t visible);
void drawGauge(coord_t x, coord_t y, coord_t w, coord_t h, int32_t val, int32_t max);
void drawColumnHeader(const char * const * headers, uint8_t index);
void drawStick(coord_t centrex, int16_t xval, int16_t yval);

void drawAlertBox(const char * title, const char * text, const char * action);
void showAlertBox(const char * title, const char * text, const char * action, uint8_t sound);

void doMainScreenGraphics();

void title(const char * s);

typedef int choice_t;

choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX = 0);
choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX,
                    IsValueAvailable isValueAvailable);

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label,
                     LcdFlags attr, event_t event, coord_t lblX = 0);

swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr,
                   event_t event);

uint16_t editSrcVarFieldValue(coord_t x, coord_t y, const char* title, uint16_t value,
                              int16_t min, int16_t max, LcdFlags attr, event_t event,
                              IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

#if defined(GVARS)
void drawGVarValue(coord_t x, coord_t y, uint8_t gvar, gvar_t value,
                   LcdFlags flags = 0);

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min,
                           int16_t max, LcdFlags attr, uint8_t editflags,
                           event_t event);

#define GVAR_MENU_ITEM(x, y, v, min, max, lcdattr, editflags, event)    \
  editGVarFieldValue(x, y, v, min, max, lcdattr, editflags, event)

#define displayGVar(x, y, v, min, max)          \
  GVAR_MENU_ITEM(x, y, v, min, max, 0, 0, 0)

#else // GVARS

#define GVAR_MENU_ITEM(x, y, v, min, max, lcdattr, editflags, event)    \
  editGVarFieldValue(x, y, v, min, max, lcdattr, event)

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min,
                           int16_t max, LcdFlags attr, event_t event);

#define displayGVar(x, y, v, min, max) lcdDrawNumber(x, y, v)

#endif

void editCurveRef(coord_t x, coord_t y, CurveRef & curve, event_t event, LcdFlags flags,
                  IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

extern uint8_t editNameCursorPos;

void editName(coord_t x, coord_t y, char *name, uint8_t size, event_t event,
              uint8_t active, LcdFlags attr, uint8_t old_editMode);

void editSingleName(coord_t x, coord_t y, const char *label, char *name,
                    uint8_t size, event_t event, uint8_t active,
                    uint8_t old_editMode, coord_t lblX = 0);

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay, uint8_t prec);

#define EDIT_DELAY(y, event, attr, str, delay, prec) editDelay(y, event, attr, str, delay, prec)

void copySelection(char * dst, const char * src, uint8_t size);

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

uint8_t getExposCount();
void deleteExpo(uint8_t idx);
void insertExpo(uint8_t idx);

#define STATUS_LINE_LENGTH             32
extern char statusLineMsg[STATUS_LINE_LENGTH];
void showStatusLine();
void drawStatusLine();

void menuTextView(event_t event);
void pushMenuTextView(const char *filename);
void pushModelNotes();
void readModelNotes();

void menuChannelsView(event_t event);

#define EDIT_MODE_INIT                 0

void onSwitchLongEnterPress(const char *result);
void onSourceLongEnterPress(const char *result);

uint8_t switchToMix(uint8_t source);

extern coord_t scrollbar_X;
#define SET_SCROLLBAR_X(x) scrollbar_X = (x);

extern const unsigned char sticks[] ;

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif

#define IS_MAIN_VIEW_DISPLAYED()       menuHandlers[0] == menuMainView
#define IS_TELEMETRY_VIEW_DISPLAYED()  menuHandlers[0] == menuViewTelemetry
#define IS_OTHER_VIEW_DISPLAYED()      (menuHandlers[0] == menuMainViewChannelsMonitor || menuHandlers[0] == menuChannelsView)
