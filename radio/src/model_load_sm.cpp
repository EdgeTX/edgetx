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

#include "model_load_sm.h"

#include "edgetx.h"
#include "switches.h"
#include "sdcard.h"
#include "audio.h"
#include "storage/storage.h"

#if defined(GUI)
#include "gui/gui_common.h"
#endif

#include <cstdio>
#include <cstring>

static ModelLoadState s_state = MLS_IDLE;
static bool s_entered = false;     // entry evaluation done for the current state
static bool s_warnActive = false;  // a warning dialog should be shown for the current state
static bool s_ack = false;         // view reported a "press any key to skip"

static char s_warnText[64];

bool modelLoadIdle() { return s_state == MLS_IDLE; }

ModelLoadState modelLoadActiveWarning()
{
  return s_warnActive ? s_state : MLS_IDLE;
}

const char* modelLoadWarningText() { return s_warnText; }

void modelLoadAcknowledge() { s_ack = true; }

void modelLoadStart()
{
  s_state = MLS_CHECK_SD;
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
  switch (s_state) {
    case MLS_CHECK_SD:        s_state = MLS_CHECK_THROTTLE;  break;
    case MLS_CHECK_THROTTLE:  s_state = MLS_CHECK_SWITCHES;  break;
    case MLS_CHECK_SWITCHES:  s_state = MLS_CHECK_FAILSAFE;  break;
    case MLS_CHECK_FAILSAFE:  s_state = MLS_CHECK_MULTI;     break;
    case MLS_CHECK_MULTI:     s_state = MLS_CHECK_CHECKLIST; break;
    case MLS_CHECK_CHECKLIST: s_state = MLS_START_PULSES;    break;
    default:                  s_state = MLS_START_PULSES;    break;
  }
}

void modelLoadStateMachineRun()
{
  if (s_state == MLS_IDLE) return;

  // Refresh the input readings once per tick, then query the pure predicates
  // below; the warning views read the same fresh state.
  refreshInputsForWarnings();

  // Loop so that any run of states that don't need a warning collapses into a
  // single tick (the no-warning case reaches MLS_START_PULSES immediately).
  for (;;) {
    switch (s_state) {
      case MLS_IDLE:
        return;

      case MLS_CHECK_SD:
        if (!s_entered) { s_entered = true; s_warnActive = sdIsFull(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;

      case MLS_CHECK_THROTTLE:
        if (!s_entered) {
          s_entered = true;
          // don't check the throttle stick if the radio is not calibrated
          s_warnActive = (g_eeGeneral.chkSum == evalChkSum()) &&
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

      case MLS_CHECK_SWITCHES: {
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

      case MLS_CHECK_FAILSAFE:
        if (!s_entered) { s_entered = true; s_warnActive = isFailsafeWarningRequired(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;

      case MLS_CHECK_MULTI:
#if defined(MULTIMODULE)
        if (!s_entered) { s_entered = true; s_warnActive = isMultiLowPowerWarningRequired(); }
        if (!s_warnActive) { gotoNextState(); continue; }
        if (s_ack) { s_ack = false; gotoNextState(); continue; }
        return;
#else
        gotoNextState();
        continue;
#endif

      case MLS_CHECK_CHECKLIST:
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

      case MLS_START_PULSES:
        // all checks cleared: announce the model, restart pulses/mixer and run
        // the rest of the model-load tail, then settle into the silence period
        PLAY_MODEL_NAME();
        postModelLoadFinish();
        START_SILENCE_PERIOD();
        s_state = MLS_IDLE;
        s_warnActive = false;
        s_entered = false;
        return;
    }
  }
}
