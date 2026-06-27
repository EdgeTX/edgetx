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
#include "controls/switch_warn_dialog.h"
#include "libui/fullscreen_dialog.h"
#include "libui/view_text.h"

// The single warning dialog currently displayed for the active machine state,
// and the state it corresponds to (so we only (re)build on state changes).
static Window* s_shown = nullptr;
static WarningCheckState s_shownState = WCS_IDLE;
// set while we tear a dialog down ourselves, to distinguish a view-initiated
// close (no acknowledge) from a user/condition close (acknowledge).
static bool s_tearingDown = false;

// Invoked from Window::deleteLater() (before the window is trashed/freed) when
// the dialog we are showing closes — either because its warning condition
// cleared, or the user pressed a key. We must react here rather than polling
// s_shown->deleted() afterwards, since perMain's MainWindow::run() empties the
// trash (freeing the window) before the next warningChecksViewSync().
static void onShownClosed()
{
  s_shown = nullptr;
  if (!s_tearingDown) acknowledgeWarningCheck();
}

static FullScreenDialog* makeAckAlert(const char* title, const char* msg)
{
  AUDIO_ERROR_MESSAGE(AU_ERROR);
  return new FullScreenDialog(WARNING_TYPE_ALERT, title, msg,
                              STR_PRESS_ANY_KEY_TO_SKIP);
}

void warningChecksViewSync()
{
  // Suspend main-view widget refresh for the whole model-load sequence: the
  // model is being swapped and the warning view covers the main view anyway.
  // Edge-triggered so we don't fight other users of the flag (standalone Lua).
  static bool s_seqActive = false;
  bool active = !warningChecksIdle();
  if (active != s_seqActive) {
    s_seqActive = active;
    MainWindow::instance()->enableWidgetRefresh(!active);
  }

  WarningCheckState want = activeWarningCheck();
  if (want == s_shownState) return;

  // Tear down a dialog that is still up (e.g. the sequence was restarted by a
  // new model switch). Suppress the acknowledge for this view-initiated close.
  if (s_shown) {
    s_tearingDown = true;
    s_shown->deleteLater();   // fires onShownClosed -> s_shown = nullptr
    s_tearingDown = false;
  }
  s_shownState = want;

  Window* dlg = nullptr;
  switch (want) {
    case WCS_CHECK_THROTTLE:
      LED_ERROR_BEGIN();
      dlg = new ThrottleWarnDialog(warningCheckText());
      break;
    case WCS_CHECK_SWITCHES:
      LED_ERROR_BEGIN();
      dlg = new SwitchWarnDialog();
      break;
    case WCS_CHECK_SD:
      LED_ERROR_BEGIN();
      dlg = makeAckAlert(STR_SD_CARD, STR_SDCARD_FULL);
      break;
    case WCS_CHECK_FAILSAFE:
      LED_ERROR_BEGIN();
      dlg = makeAckAlert(STR_FAILSAFEWARN, STR_NO_FAILSAFE);
      break;
#if defined(MULTIMODULE)
    case WCS_CHECK_MULTI:
      LED_ERROR_BEGIN();
      dlg = makeAckAlert("MULTI", STR_WARN_MULTI_LOWPOWER);
      break;
#endif
    case WCS_CHECK_CHECKLIST:
      cancelSplash();
      dlg = showModelChecklist();
      // no notes window could be opened: don't stall the sequence
      if (!dlg) acknowledgeWarningCheck();
      break;
    case WCS_IDLE:
    default:
      // sequence finished: clear the error indication
      LED_ERROR_END();
      break;
  }

  if (dlg) {
    dlg->setCloseHandler(onShownClosed);
    s_shown = dlg;
  }
}
