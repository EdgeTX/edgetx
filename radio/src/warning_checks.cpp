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

#include "warning_checks.h"

#include "edgetx.h"
#include "switches.h"
#include "sdcard.h"
#include "audio.h"
#include "pulses/pulses.h"
#include "tasks/mixer_task.h"
#include "storage/storage.h"

#if defined(GUI)
#include "gui/gui_common.h"
#endif

#include <cstdio>
#include <cstring>

static WarningCheckState s_state = WCS_IDLE;
static WarningCheckContext s_ctx = WCC_MODEL_SWITCH;  // what armed the machine
static bool s_entered = false;     // entry evaluation done for the current state
static bool s_warnActive = false;  // a warning dialog should be shown for the current state
static bool s_ack = false;         // view reported a "press any key to skip"

static char s_warnText[64];

bool warningChecksIdle() { return s_state == WCS_IDLE; }

WarningCheckState activeWarningCheck()
{
  return s_warnActive ? s_state : WCS_IDLE;
}

const char* warningCheckText() { return s_warnText; }

void acknowledgeWarningCheck() { s_ack = true; }

void warningChecksStart(WarningCheckContext ctx)
{
  s_ctx = ctx;
  // The stick-mode change only needs the throttle check; every other context
  // runs the full sequence from the top.
  s_state = (ctx == WCC_STICK_MODE) ? WCS_CHECK_THROTTLE : WCS_CHECK_SD;
  s_entered = false;
  s_warnActive = false;
  s_ack = false;
}

static void buildThrottleText()
{
  if (g_model.enableCustomThrottleWarning) {
    snprintf(s_warnText, sizeof(s_warnText), "%s (%d%%)", STR_THROTTLE_NOT_IDLE,
             g_model.customThrottleWarningPosition);
  } else {
    strncpy(s_warnText, STR_THROTTLE_NOT_IDLE, sizeof(s_warnText) - 1);
    s_warnText[sizeof(s_warnText) - 1] = '\0';
  }
}

static void gotoNextState()
{
  s_entered = false;
  s_warnActive = false;
  // The stick-mode context only runs the throttle check, then terminates.
  if (s_ctx == WCC_STICK_MODE) {
    s_state = WCS_COMPLETE;
    return;
  }
  switch (s_state) {
    case WCS_CHECK_SD:        s_state = WCS_CHECK_THROTTLE;  break;
    case WCS_CHECK_THROTTLE:  s_state = WCS_CHECK_SWITCHES;  break;
    case WCS_CHECK_SWITCHES:  s_state = WCS_CHECK_FAILSAFE;  break;
    case WCS_CHECK_FAILSAFE:  s_state = WCS_CHECK_MULTI;     break;
    case WCS_CHECK_MULTI:     s_state = WCS_CHECK_CHECKLIST; break;
    case WCS_CHECK_CHECKLIST: s_state = WCS_COMPLETE;    break;
    default:                  s_state = WCS_COMPLETE;    break;
  }
}

void warningChecksRun()
{
  if (s_state == WCS_IDLE) return;

  // Refresh the input readings once per tick, then query the pure predicates
  // below; the warning views read the same fresh state.
  refreshInputsForWarnings();

  // Loop so that any run of states that don't need a warning collapses into a
  // single tick (the no-warning case reaches WCS_COMPLETE immediately).
  for (;;) {
    switch (s_state) {
      case WCS_IDLE:
        return;

      case WCS_CHECK_SD:
        if (!s_entered) { s_entered = true; s_warnActive = sdIsFull(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;

      case WCS_CHECK_THROTTLE:
        if (!s_entered) {
          s_entered = true;
          // Don't check the throttle stick if the radio is not calibrated; the
          // stick-mode change is exempt (its old direct call never gated on it).
          s_warnActive = (s_ctx == WCC_STICK_MODE ||
                          g_eeGeneral.chkSum == evalChkSum()) &&
                         isThrottleWarningAlertNeeded();
          if (s_warnActive) buildThrottleText();
        }
        if (!s_warnActive) { gotoNextState(); continue; }
        // Auto-advance when the stick is lowered (pure predicate, evaluated on
        // this tick's fresh inputs), or skip on key press (view acknowledge).
        if (s_ack || !isThrottleWarningAlertNeeded()) {
          s_ack = false; gotoNextState(); continue;
        }
        return;

      case WCS_CHECK_SWITCHES: {
        uint16_t bad_pots = 0;
        if (!s_entered) {
          s_entered = true;
          s_warnActive = isSwitchWarningRequired(bad_pots);
        }
        if (!s_warnActive) { gotoNextState(); continue; }
        // Auto-advance when the switches/pots are corrected, or skip on key
        // press (view acknowledge).
        if (s_ack || !isSwitchWarningRequired(bad_pots)) {
          s_ack = false; gotoNextState(); continue;
        }
        return;
      }

      case WCS_CHECK_FAILSAFE:
        if (!s_entered) { s_entered = true; s_warnActive = isFailsafeWarningRequired(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;

      case WCS_CHECK_MULTI:
#if defined(MULTIMODULE)
        if (!s_entered) { s_entered = true; s_warnActive = isMultiLowPowerWarningRequired(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;
#else
        gotoNextState();
        continue;
#endif

      case WCS_CHECK_CHECKLIST:
#if defined(GUI)
        if (!s_entered) {
          s_entered = true;
          s_warnActive = (g_model.displayChecklist && modelHasNotes());
        }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;
#else
        gotoNextState();
        continue;
#endif

      case WCS_COMPLETE:
        // All checks cleared. The terminal action depends on how the machine was
        // armed (see WarningCheckContext): announce the model, restart pulses/mixer
        // and/or run the model-load tail, then settle into the silence period.
        if (s_ctx == WCC_MODEL_SWITCH || s_ctx == WCC_BOOT) PLAY_MODEL_NAME();
        switch (s_ctx) {
          case WCC_MODEL_SWITCH:
            // runtime switch: pulses were stopped in preModelLoad; restart them
            // and run the rest of the model-load tail (audio refs, LUA, failsafe)
            postModelLoadFinish();
            break;
          case WCC_BOOT:
            // boot: postModelLoadFinish() already ran at load time (with the
            // mixer not yet started, so it skipped pulses); just start pulses now
            pulsesStart();
            break;
          case WCC_STICK_MODE:
            // stick-mode change: the caller stopped the mixer; restart it
            mixerTaskStart();
            break;
          case WCC_FLIGHT_RESET:
            // flight reset never stopped pulses and must not reload the model
            break;
        }
        if (s_ctx != WCC_STICK_MODE) START_SILENCE_PERIOD();
        s_state = WCS_IDLE;
        s_warnActive = false;
        s_entered = false;
        return;
    }
  }
}
