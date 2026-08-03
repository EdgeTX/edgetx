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

#include "warning_checks_view.h"

#include "edgetx.h"
#include "warning_checks.h"
#include "menus_common.h"
#include "strhelpers.h"

#if LCD_W >= 212
  #define SWITCH_WARNING_LIST_X        60
  #define SWITCH_WARNING_LIST_Y        4*FH+4
#else
  #define SWITCH_WARNING_LIST_X        4
  #define SWITCH_WARNING_LIST_Y        4*FH+4
#endif

// Draw the full-screen switch/pot warning (one frame). Called once per main-loop
// tick by warningChecksViewSync(); refresh, audio and key handling are the
// view's job (see below).
static void drawSwitchWarningScreen()
{
  drawAlertBox(STR_SWITCHWARN, nullptr, STR_PRESS_ANY_KEY_TO_SKIP);

  int x = SWITCH_WARNING_LIST_X;
  int y = SWITCH_WARNING_LIST_Y;
  int numWarnings = 0;
  for (int i = 0; i < switchGetMaxAllSwitches(); ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      uint8_t warnState = g_model.getSwitchWarning(i);
      if (warnState) {
        swarnstate_t swState = g_model.getSwitchStateForWarning(i);
        if (warnState != swState) {
          if (++numWarnings < 6) {
            const char* s = getSwitchWarnSymbol(warnState);
            drawSource(x, y, MIXSRC_FIRST_SWITCH + i, INVERS);
            lcdDrawText(lcdNextPos, y, s, INVERS);
            x = lcdNextPos + 3;
          }
        }
      }
    }
  }

  if (g_model.potsWarnMode) {
    for (int i = 0; i < MAX_POTS; i++) {
      if (!IS_POT_SLIDER_AVAILABLE(i)) continue;
      if (g_model.potsWarnEnabled & (1 << i)) {
        if (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1) {
          if (++numWarnings < 6) {
            drawSource(x, y, MIXSRC_FIRST_POT + i, INVERS);
            const char* symbol;
            auto warn_pos = g_model.potsWarnPosition[i];
            if (IS_SLIDER(i)) {
              symbol =  warn_pos > GET_LOWRES_POT_POSITION(i)
                ? CHAR_UP
                : CHAR_DOWN;
            } else {
              symbol =  warn_pos > GET_LOWRES_POT_POSITION(i)
                ? CHAR_RIGHT
                : CHAR_LEFT;
            }
            lcdDrawText(lcdNextPos, y, symbol, INVERS);
            x = lcdNextPos + 3;
          }
        }
      }
    }
  }

  if (numWarnings >= 6) {
    lcdDrawText(x, y, "...", 0);
  }
}

// Plays the per-warning alert sound on the entering edge (the state just became
// active). The state machine reads fresh inputs every tick, so the warning may
// auto-clear without a key press; we only want one sound per warning.
static void playEnterSound(WarningCheckState st)
{
  switch (st) {
    case WCS_CHECK_THROTTLE: AUDIO_ERROR_MESSAGE(AU_THROTTLE_ALERT); break;
    case WCS_CHECK_SWITCHES: AUDIO_ERROR_MESSAGE(AU_SWITCH_ALERT);   break;
    default:                 AUDIO_ERROR_MESSAGE(AU_ERROR);          break;
  }
}

void warningChecksViewSync(event_t evt)
{
  static WarningCheckState s_shownState = WCS_IDLE;

  WarningCheckState st = activeWarningCheck();

  if (st == WCS_IDLE) {
    // sequence finished (or no warning needed): clear the error indication once
    if (s_shownState != WCS_IDLE) {
      LED_ERROR_END();
      s_shownState = WCS_IDLE;
    }
    return;
  }

  bool entering = (st != s_shownState);
  if (entering) {
    cancelSplash();
    LED_ERROR_BEGIN();
    s_shownState = st;
    if (st == WCS_CHECK_CHECKLIST) setModelChecklistFilename();
  }

  // The checklist is interactive: drive the text viewer with the tick's event
  // and report completion (rather than consuming the event as a plain "skip").
  if (st == WCS_CHECK_CHECKLIST) {
    reusableBuffer.viewText.pushMenu = false;
    lcdRefreshWait();
    lcdClear();
    menuTextView(entering ? EVT_ENTRY : evt);
    lcdRefresh();
    if (reusableBuffer.viewText.checklistComplete) acknowledgeWarningCheck();
    return;
  }

  if (entering) playEnterSound(st);

  lcdRefreshWait();
  switch (st) {
    case WCS_CHECK_THROTTLE:
      drawAlertBox(STR_THROTTLE_UPPERCASE, warningCheckText(),
                   STR_PRESS_ANY_KEY_TO_SKIP);
      break;
    case WCS_CHECK_SWITCHES:
      drawSwitchWarningScreen();
      break;
    case WCS_CHECK_SD:
      drawAlertBox(STR_SD_CARD, STR_SDCARD_FULL, STR_PRESS_ANY_KEY_TO_SKIP);
      break;
    case WCS_CHECK_FAILSAFE:
      drawAlertBox(STR_FAILSAFEWARN, STR_NO_FAILSAFE, STR_PRESS_ANY_KEY_TO_SKIP);
      break;
#if defined(MULTIMODULE)
    case WCS_CHECK_MULTI:
      drawAlertBox("MULTI", STR_WARN_MULTI_LOWPOWER, STR_PRESS_ANY_KEY_TO_SKIP);
      break;
#endif
    default:
      break;
  }
  lcdRefresh();

  // Any key acknowledges the warning ("press any key to skip"). The machine
  // consumes the acknowledge on the next tick's warningChecksRun().
  if (evt) acknowledgeWarningCheck();
}
