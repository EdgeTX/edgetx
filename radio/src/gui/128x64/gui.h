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
#include "menus.h"
#include "popups.h"
#include "common/stdlcd/draw_functions.h"

#define MENUS_SCROLLBAR_WIDTH          0

#if defined(NAVIGATION_X7)
  #define HEADER_LINE                  0
  #define HEADER_LINE_COLUMNS
#else
  #define HEADER_LINE                  1
  #define HEADER_LINE_COLUMNS          0,
#endif

#define drawFieldLabel(x, y, str)      lcdDrawTextAlignedLeft(y, str)

#define NUM_BODY_LINES                 (LCD_LINES-1)
#define TEXT_VIEWER_LINES              NUM_BODY_LINES
#define MENU_HEADER_HEIGHT             FH

#define CURVE_SIDE_WIDTH               (LCD_H/2)
#define CURVE_CENTER_X                 (LCD_W-CURVE_SIDE_WIDTH-2)
#define CURVE_CENTER_Y                 (LCD_H/2)

#define MIXES_2ND_COLUMN               (12*FW)

// Temporary no highlight
extern uint8_t noHighlightCounter;
#define NO_HIGHLIGHT()        (noHighlightCounter > 0)
#define START_NO_HIGHLIGHT()  do { noHighlightCounter = 25; } while(0)

void drawSlider(coord_t x, coord_t y, uint8_t width, uint8_t value, uint8_t max, uint8_t attr);
void drawSlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr);

extern int8_t checkIncDec_Ret;  // global helper vars

#define EDIT_SELECT_FIELD              0
#define EDIT_MODIFY_FIELD              1
#define EDIT_MODIFY_STRING             2
extern int8_t s_editMode;       // global editmode

// checkIncDec flags

// we leave room for EE_MODEL and EE_GENERAL
#define NO_INCDEC_MARKS                0x04
#define INCDEC_SWITCH                  0x08
#define INCDEC_SOURCE                  0x10
#define INCDEC_REP10                   0x40
#define NO_DBLKEYS                     0x80

#define INCDEC_DECLARE_VARS(f)       uint8_t incdecFlag = (f); IsValueAvailable isValueAvailable = nullptr
#define INCDEC_SET_FLAG(f)           incdecFlag = (f)
#define INCDEC_ENABLE_CHECK(fn)      isValueAvailable = fn
#define CHECK_INCDEC_PARAM(event, var, min, max) checkIncDec(event, var, min, max, incdecFlag, isValueAvailable)

struct CheckIncDecStops {
  const int count;
  const int stops[];
  int min() const
  {
    return stops[0];
  }
  int max() const
  {
    return stops[count-1];
  }
  bool contains(int value) const
  {
    for (int i=0; i<count; ++i) {
      int stop = stops[i];
      if (value == stop)
        return true;
      else if (value < stop)
        return false;
    }
    return false;
  }
};
extern const CheckIncDecStops &stops100;
extern const CheckIncDecStops &stops1000;
extern const CheckIncDecStops &stopsSwitch;
#define INIT_STOPS(var, ...)                                        \
  const int _ ## var[] = { __VA_ARGS__ };                           \
  const CheckIncDecStops &var  = (const CheckIncDecStops&)_ ## var;
#define CATEGORY_END(val)                                          \
  (val), (val+1)
int checkIncDec(event_t event, int val, int i_min, int i_max, unsigned int i_flags=0, IsValueAvailable isValueAvailable=nullptr, const CheckIncDecStops &stops=stops100);

#define checkIncDecModel(event, i_val, i_min, i_max) checkIncDec(event, i_val, i_min, i_max, EE_MODEL)
#define checkIncDecModelZero(event, i_val, i_max) checkIncDec(event, i_val, 0, i_max, EE_MODEL)
#define checkIncDecGen(event, i_val, i_min, i_max) checkIncDec(event, i_val, i_min, i_max, EE_GENERAL)

#define CHECK_INCDEC_MODELVAR(event, var, min, max) \
  var = checkIncDecModel(event, var, min, max)

#define CHECK_INCDEC_MODELVAR_ZERO(event, var, max) \
  var = checkIncDecModelZero(event, var, max)

#define CHECK_INCDEC_MODELVAR_CHECK(event, var, min, max, check) \
    var = checkIncDec(event, var, min, max, EE_MODEL, check)
  #define CHECK_INCDEC_MODELVAR_ZERO_CHECK(event, var, max, check) \
    var = checkIncDec(event, var, 0, max, EE_MODEL, check)

#define AUTOSWITCH_ENTER_LONG() (attr && event==EVT_KEY_LONG(KEY_ENTER))
  #define CHECK_INCDEC_SWITCH(event, var, min, max, flags, available) \
    var = checkIncDec(event, var, min, max, (flags)|INCDEC_SWITCH, available)
  #define CHECK_INCDEC_MODELSWITCH(event, var, min, max, available) \
    CHECK_INCDEC_SWITCH(event, var, min, max, EE_MODEL, available)

#define CHECK_INCDEC_MODELSOURCE(event, var, min, max) \
    var = checkIncDec(event,var,min,max,EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable)

#define CHECK_INCDEC_GENVAR(event, var, min, max) \
  var = checkIncDecGen(event, var, min, max)

#if defined(PCBTARANIS)
#define CURSOR_ON_LINE()               (menuHorizontalPosition < 0)
#else
#define CURSOR_ON_LINE()               (0)
#endif

void check(event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax, vertpos_t maxrow);
void check_simple(event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, vertpos_t maxrow);
void check_submenu_simple(event_t event, uint8_t maxrow);

void title(const char * s);

#define MENU_TAB(...) const uint8_t mstate_tab[] = __VA_ARGS__

#define MENU_CHECK(tab, menu, lines_count) \
  check(event, menu, tab, DIM(tab), mstate_tab, DIM(mstate_tab)-1, (lines_count)-HEADER_LINE)

#define MENU(name, tab, menu, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  MENU_CHECK(tab, menu, lines_count); \
  title(name)

#define SUBMENU_NOTITLE(lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  check(event, 0, nullptr, 0, mstate_tab, DIM(mstate_tab)-1, (lines_count)-HEADER_LINE)

#define SIMPLE_MENU_NOTITLE(tab, menu, lines_count) \
  check_simple(event, menu, tab, DIM(tab), (lines_count)-HEADER_LINE)

#define SIMPLE_SUBMENU_NOTITLE(lines_count) \
  check_submenu_simple(event, (lines_count)-HEADER_LINE)

#define SUBMENU(name, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  check(event, 0, nullptr, 0, mstate_tab, DIM(mstate_tab)-1, (lines_count)-HEADER_LINE); \
  title(name)

#define SIMPLE_MENU(name, tab, menu, lines_count) \
  SIMPLE_MENU_NOTITLE(tab, menu, lines_count); \
  title(name)

#define SIMPLE_SUBMENU(name, lines_count) \
  SIMPLE_SUBMENU_NOTITLE(lines_count); \
  title(name)

typedef int choice_t;
choice_t editChoice(coord_t x, coord_t y, const char * label, const char** values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event, IsValueAvailable isValueAvailable = nullptr);
uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char * label, LcdFlags attr, event_t event);
swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr, event_t event);

#if defined(GVARS)
#define GVAR_MENU_ITEM(x, y, v, min, max, attr, editflags, event) editGVarFieldValue(x, y, v, min, max, attr, editflags, event)
int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t editflags, event_t event);
void drawGVarValue(coord_t x, coord_t y, uint8_t gvar, gvar_t value, LcdFlags flags=0);
void editGVarValue(coord_t x, coord_t y, event_t event, uint8_t gvar, uint8_t flightMode, LcdFlags flags);
#define displayGVar(x, y, v, min, max) GVAR_MENU_ITEM(x, y, v, min, max, 0, 0, 0)
#else
int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, event_t event);
#define GVAR_MENU_ITEM(x, y, v, min, max, attr, editflags, event) editGVarFieldValue(x, y, v, min, max, attr, event)
#define displayGVar(x, y, v, min, max) lcdDraw8bitsNumber(x, y, v)
#endif

void gvarWeightItem(coord_t x, coord_t y, MixData * md, LcdFlags attr, event_t event);

void editName(coord_t x, coord_t y, char *name, uint8_t size, event_t event,
              uint8_t active, LcdFlags attr, uint8_t old_editMode);

void editSingleName(coord_t x, coord_t y, const char *label, char *name,
                    uint8_t size, event_t event, uint8_t active,
                    uint8_t old_editMode);

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay);
#define EDIT_DELAY(x, y, event, attr, str, delay) editDelay(y, event, attr, str, delay)

#define COPY_MODE                      1
#define MOVE_MODE                      2
extern uint8_t s_copyMode;
extern int8_t s_copySrcRow;
extern int8_t s_copyTgtOfs;
extern uint8_t s_currIdx;
extern uint8_t s_currIdxSubMenu;
extern uint16_t s_currSrcRaw;
extern uint16_t s_currScale;
extern uint8_t s_copySrcIdx;
extern uint8_t s_copySrcCh;
extern int8_t s_currCh;
extern uint8_t s_maxLines;

#if defined(SDCARD)
#define STATUS_LINE_LENGTH           32
extern char statusLineMsg[STATUS_LINE_LENGTH];
void showStatusLine();
void drawStatusLine();
#else
#define drawStatusLine()
#endif

void menuTextView(event_t event);
void pushMenuTextView(const char *filename);
void pushModelNotes();
void readModelNotes();

void menuChannelsView(event_t event);
void menuChannelsViewCommon(event_t event);

#define CURSOR_MOVED_LEFT(event)       (IS_ROTARY_LEFT(event) || EVT_KEY_MASK(event) == KEY_LEFT)
#define CURSOR_MOVED_RIGHT(event)      (IS_ROTARY_RIGHT(event) || EVT_KEY_MASK(event) == KEY_RIGHT)

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define IS_ROTARY_LEFT(evt)          (evt == EVT_ROTARY_LEFT)
  #define IS_ROTARY_RIGHT(evt)         (evt == EVT_ROTARY_RIGHT)
  #define IS_ROTARY_BREAK(evt)         (evt == EVT_ROTARY_BREAK)
  #define IS_ROTARY_LONG(evt)          (evt == EVT_ROTARY_LONG)
  #define IS_ROTARY_EVENT(evt)         (EVT_KEY_MASK(evt) >= 0x0e)
  void repeatLastCursorMove(event_t event);
  #define REPEAT_LAST_CURSOR_MOVE()    { if (EVT_KEY_MASK(event) >= 0x0e) pushEvent(event); else repeatLastCursorMove(event); }
#else
  #define IS_ROTARY_LEFT(evt)          (0)
  #define IS_ROTARY_RIGHT(evt)         (0)
  #define IS_ROTARY_BREAK(evt)         (0)
  #define IS_ROTARY_LONG(evt)          (0)
  #define IS_ROTARY_EVENT(evt)         (0)
  void repeatLastCursorMove(event_t event);
  #define REPEAT_LAST_CURSOR_MOVE()    repeatLastCursorMove(event)
#endif

// TODO enum
#if defined(PCBX7) || defined(PCBX9LITE)
#define EDIT_MODE_INIT                 0
#else
#define EDIT_MODE_INIT                 -1
#endif

extern uint8_t editNameCursorPos;

uint8_t getExposCount();
void insertExpo(uint8_t idx);
void deleteExpo(uint8_t idx);
uint8_t getMixesCount();
void insertMix(uint8_t idx);
void deleteMix(uint8_t idx);

void onSourceLongEnterPress(const char *result);

uint8_t switchToMix(uint8_t source);

void drawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr);

extern const unsigned char sticks[] ;

void drawSplash();
void drawScreenIndex(uint8_t index, uint8_t count, uint8_t attr);
void drawStick(coord_t centrex, int16_t xval, int16_t yval);
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

void editCurveRef(coord_t x, coord_t y, CurveRef & curve, event_t event, LcdFlags flags);

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif

#endif // _GUI_H_
