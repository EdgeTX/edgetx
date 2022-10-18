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

#include "opentx.h"

extern bool displayTelemetryScreen();
extern void displayRssiLine();

enum NavigationDirection {
  NAVIGATION_DIRECTION_NONE,
  NAVIGATION_DIRECTION_UP,
  NAVIGATION_DIRECTION_DOWN
};
#define decrTelemetryScreen() direction = NAVIGATION_DIRECTION_UP
#define incrTelemetryScreen() direction = NAVIGATION_DIRECTION_DOWN

#if defined(NAVIGATION_XLITE)
  #define EVT_KEY_PREVIOUS_VIEW(evt)         (evt == EVT_KEY_LONG(KEY_LEFT) && keysGetState(KEY_SHIFT))
  #define EVT_KEY_NEXT_VIEW(evt)             (evt == EVT_KEY_LONG(KEY_RIGHT) && keysGetState(KEY_SHIFT))
#elif defined(KEYS_GPIO_REG_PAGEDN)
  #define EVT_KEY_PREVIOUS_VIEW(evt)         (evt == EVT_KEY_FIRST(KEY_PAGEUP))
  #define EVT_KEY_NEXT_VIEW(evt)             (evt == EVT_KEY_FIRST(KEY_PAGEDN))
#elif defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
  #define EVT_KEY_PREVIOUS_VIEW(evt)         (evt == EVT_KEY_LONG(KEY_PAGE))
  #define EVT_KEY_NEXT_VIEW(evt)             (evt == EVT_KEY_BREAK(KEY_PAGE))
#elif defined(NAVIGATION_9X)
  #define EVT_KEY_PREVIOUS_VIEW(evt)         (evt == EVT_KEY_LONG(KEY_UP))
  #define EVT_KEY_NEXT_VIEW(evt)             (evt == EVT_KEY_LONG(KEY_DOWN))
#else
  #define EVT_KEY_PREVIOUS_VIEW(evt)         (evt == EVT_KEY_FIRST(KEY_UP))
  #define EVT_KEY_NEXT_VIEW(evt)             (evt == EVT_KEY_FIRST(KEY_DOWN))
#endif

void menuViewTelemetry(event_t event)
{
  enum NavigationDirection direction = NAVIGATION_DIRECTION_NONE;

  if (event == EVT_KEY_FIRST(KEY_EXIT) && TELEMETRY_SCREEN_TYPE(s_frsky_view) != TELEMETRY_SCREEN_TYPE_SCRIPT) {
    killEvents(event);
    chainMenu(menuMainView);
  }
#if defined(LUA)
  else if (event == EVT_KEY_LONG(KEY_EXIT)) {
    killEvents(event);
    chainMenu(menuMainView);
  }
#endif
  else if (EVT_KEY_PREVIOUS_VIEW(event)) {
    killEvents(event);
    decrTelemetryScreen();
  }
  else if (EVT_KEY_NEXT_VIEW(event)) {
    killEvents(event);
    incrTelemetryScreen();
  }
  else if (event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    POPUP_MENU_ADD_ITEM(STR_RESET_TELEMETRY);
    POPUP_MENU_ADD_ITEM(STR_RESET_FLIGHT);
    POPUP_MENU_START(onMainViewMenu);
  }

  for (int i=0; i<=TELEMETRY_SCREEN_TYPE_MAX; i++) {
    if (direction == NAVIGATION_DIRECTION_UP) {
      if (s_frsky_view-- == 0)
        s_frsky_view = TELEMETRY_VIEW_MAX;
    }
    else if (direction == NAVIGATION_DIRECTION_DOWN) {
      if (s_frsky_view++ == TELEMETRY_VIEW_MAX)
        s_frsky_view = 0;
    }
    else {
      direction = NAVIGATION_DIRECTION_DOWN;
    }
    if (displayTelemetryScreen()) {
      return;
    }
  }

  drawTelemetryTopBar();
  lcdDrawText(LCD_W / 2, 3 * FH, STR_NO_TELEMETRY_SCREENS, CENTERED);
  displayRssiLine();
}

#undef EVT_KEY_PREVIOUS_VIEW
#undef EVT_KEY_NEXT_VIEW
