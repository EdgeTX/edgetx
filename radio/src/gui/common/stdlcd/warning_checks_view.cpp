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
