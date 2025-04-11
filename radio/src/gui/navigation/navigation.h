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

// Define navigation type based on available keys
#if LCD_W == 212
  #define NAVIGATION_X9D
#elif defined(KEYS_GPIO_REG_SHIFT)
  #define NAVIGATION_XLITE
#elif defined(KEYS_GPIO_REG_LEFT)
  #define NAVIGATION_9X
#elif defined(KEYS_GPIO_REG_PAGEUP) && defined(KEYS_GPIO_REG_TELE)
  #define NAVIGATION_X7
  #define NAVIGATION_X7_TX12
#else
  #define NAVIGATION_X7
#endif

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
  #define HEADER_LINE                  0
  #define HEADER_LINE_COLUMNS
#else
  #define HEADER_LINE                  1
  #define HEADER_LINE_COLUMNS          0,
#endif

// Main View and Channel Monitor view navigation mapping

#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)

#if defined(RADIO_T8) || defined(RADIO_COMMANDO8)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_PLUS)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_MINUS)
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_BREAK(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_PAGEUP)
#elif defined(NAVIGATION_X7_TX12)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_BREAK(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_BREAK(KEY_TELE)
#elif defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MENU)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_MENU)
#define EVT_KEY_TELEMETRY              EVT_KEY_BREAK(KEY_PAGEUP)
#else
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_UP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_DOWN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_RIGHT)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_LEFT)
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_RIGHT)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_LEFT)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_DOWN)
#define EVT_KEY_STATISTICS             EVT_KEY_LONG(KEY_UP)
#endif

// Telemtry view navigation mapping

#if defined(NAVIGATION_XLITE)
  #define EVT_KEY_PREVIOUS_TELEM_VIEW(evt)  (evt == EVT_KEY_LONG(KEY_LEFT) && keysGetState(KEY_SHIFT))
  #define EVT_KEY_NEXT_TELEM_VIEW(evt)      (evt == EVT_KEY_LONG(KEY_RIGHT) && keysGetState(KEY_SHIFT))
#elif defined(KEYS_GPIO_REG_PAGEUP)
  #define EVT_KEY_PREVIOUS_TELEM_VIEW(evt)  (evt == EVT_KEY_FIRST(KEY_PAGEUP))
  #define EVT_KEY_NEXT_TELEM_VIEW(evt)      (evt == EVT_KEY_FIRST(KEY_PAGEDN))
#elif defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
  #define EVT_KEY_PREVIOUS_TELEM_VIEW(evt)  (evt == EVT_KEY_BREAK(KEY_PAGEUP))
  #define EVT_KEY_NEXT_TELEM_VIEW(evt)      (evt == EVT_KEY_BREAK(KEY_PAGEDN))
#elif defined(NAVIGATION_9X)
  #define EVT_KEY_PREVIOUS_TELEM_VIEW(evt)  (evt == EVT_KEY_LONG(KEY_UP))
  #define EVT_KEY_NEXT_TELEM_VIEW(evt)      (evt == EVT_KEY_LONG(KEY_DOWN))
#else
  #define EVT_KEY_PREVIOUS_TELEM_VIEW(evt)  (evt == EVT_KEY_FIRST(KEY_UP))
  #define EVT_KEY_NEXT_TELEM_VIEW(evt)      (evt == EVT_KEY_FIRST(KEY_DOWN))
#endif

// Open Channel view
#if defined(NAVIGATION_XLITE)
  #define EVT_KEY_OPEN_CHAN_VIEW(evt)       (evt == EVT_KEY_FIRST(KEY_ENTER) && keysGetState(KEY_SHIFT))
#else
  #define EVT_KEY_OPEN_CHAN_VIEW(evt)       (evt == EVT_KEY_BREAK(KEY_MODEL) || evt == EVT_KEY_BREAK(KEY_MENU))
#endif

#if defined(NAVIGATION_XLITE) || defined(NAVIGATION_9X)
  #define HAS_LEFT_RIGHT_NAV_KEYS
#endif

struct CheckIncDecStops
{
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

#define CATEGORY_END(val)                       \
  (val), (val+1)

extern int8_t checkIncDec_Ret;  // global helper vars

#define CURSOR_ON_LINE() (menuHorizontalPosition < 0)

#define EDIT_SELECT_FIELD              0
#define EDIT_MODIFY_FIELD              1
#define EDIT_MODIFY_STRING             2
extern int8_t s_editMode; // global editmode

// checkIncDec flags
// we leave room for EE_MODEL and EE_GENERAL
#define NO_INCDEC_MARKS                0x04
#define INCDEC_SWITCH                  0x08
#define INCDEC_SOURCE                  0x10
#define INCDEC_REP10                   0x40
#define NO_DBLKEYS                     0x80
#define INCDEC_SOURCE_INVERT           0x100
#define INCDEC_SOURCE_VALUE            0x200  // Field can be source or value
#define INCDEC_SKIP_VAL_CHECK_FUNC     0x400  // Skip isValueAvailable function when changing value (only used for popup)

int checkIncDec(event_t event, int val, int i_min, int i_max,
                unsigned int i_flags = 0, IsValueAvailable isValueAvailable = nullptr,
                const CheckIncDecStops &stops = stops100);

int checkIncDec(event_t event, int val, int i_min, int i_max, int srcMin, int srcMax,
                unsigned int i_flags = 0, IsValueAvailable isValueAvailable = nullptr,
                const CheckIncDecStops &stops = stops100);

#define checkIncDecModel(event, i_val, i_min, i_max) \
  checkIncDec(event, i_val, i_min, i_max, EE_MODEL)

#define checkIncDecModelZero(event, i_val, i_max)       \
  checkIncDec(event, i_val, 0, i_max, EE_MODEL)

#define checkIncDecGen(event, i_val, i_min, i_max)      \
  checkIncDec(event, i_val, i_min, i_max, EE_GENERAL)

#define NAVIGATION_LINE_BY_LINE  0x40

#define CHECK_FLAG_NO_SCREEN_INDEX   1

void check(event_t event, uint8_t curr, const MenuHandler *menuTab,
           uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax,
           vertpos_t rowcount, uint8_t flags = 0);

#define INCDEC_DECLARE_VARS(f) \
  uint16_t incdecFlag = (f);    \
  IsValueAvailable isValueAvailable = nullptr

#define INCDEC_SET_FLAG(f) incdecFlag = (f)

#define INCDEC_ENABLE_CHECK(fn) isValueAvailable = fn

#define CHECK_INCDEC_PARAM(event, var, min, max) \
  checkIncDec(event, var, min, max, incdecFlag, isValueAvailable)

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
  var = checkIncDec(event, var, min, max, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable)

#define CHECK_INCDEC_GENVAR(event, var, min, max) \
  var = checkIncDecGen(event, var, min, max)

#if defined(AUTOSWITCH)
swsrc_t checkIncDecMovedSwitch(swsrc_t val);
#endif

void repeatLastCursorMove(event_t event);
#if defined(NAVIGATION_9X) || defined(NAVIGATION_XLITE)
void repeatLastCursorHorMove(event_t event);
#else
#define repeatLastCursorHorMove(event) repeatLastCursorMove(event)
#endif

void onSwitchLongEnterPress(const char * result);
void onSourceLongEnterPress(const char * result);

void check_submenu_simple(event_t event, uint8_t rowcount);

void check_simple(event_t event, uint8_t curr, const MenuHandler *menuTab,
                  uint8_t menuTabSize, vertpos_t rowcount);


#define MENU_TAB_ARRAY_NAME mstate_tab
#define MENU_TAB_ARRAY_TYPE const uint8_t
#define MENU_TAB_ARRAY_DEF  MENU_TAB_ARRAY_TYPE MENU_TAB_ARRAY_NAME
#define MENU_TAB(...)       MENU_TAB_ARRAY_DEF[] = __VA_ARGS__

#define MENU_CHECK(tab, menu, lines_count) \
  check(event, menu, tab, DIM(tab), mstate_tab, DIM(mstate_tab)-1, lines_count)

#define MENU_CHECK_FLAGS(tab, menu, flags, lines_count) \
  check(event, menu, tab, DIM(tab), mstate_tab, DIM(mstate_tab)-1, lines_count, flags)

#define MENU(name, tab, menu, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  MENU_CHECK(tab, menu, lines_count); \
  title(name)

#define MENU_FLAGS(name, tab, menu, flags, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  MENU_CHECK_FLAGS(tab, menu, flags, lines_count); \
  title(name)

#define SIMPLE_MENU(name, tab, menu, lines_count) \
  check_simple(event, menu, tab, DIM(tab), lines_count); \
  title(name)

#define SUBMENU_NOTITLE(lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  check(event, 0, nullptr, 0, mstate_tab, DIM(mstate_tab)-1, lines_count);

#define SUBMENU(name, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  check(event, 0, nullptr, 0, mstate_tab, DIM(mstate_tab)-1, lines_count); \
  title(name)

#define SIMPLE_SUBMENU_NOTITLE(lines_count) \
  check_submenu_simple(event, lines_count)

#define SIMPLE_SUBMENU(name, lines_count) \
  SIMPLE_SUBMENU_NOTITLE(lines_count); \
  title(name)
