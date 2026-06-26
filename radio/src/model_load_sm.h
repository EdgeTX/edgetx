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

#include <cstdint>

// Post-model-load check sequence, modelled as a GUI-agnostic state machine.
//
// On a runtime model switch the mixer/pulses are stopped (preModelLoad ->
// pulsesStop) so the RF module is silent and the receiver holds failsafe. The
// machine progresses through the warning checks one at a time, and only when it
// reaches MLS_START_PULSES (all checks cleared) does it restart pulses and run
// the rest of the model-load tail. It is polled once per main loop iteration,
// so no blocking/nested UI loop is needed.
//
// This module owns the truth (which check is active, when to advance, when to
// start pulses). The GUI is only a *view*: it renders modelLoadActiveWarning()
// and feeds key presses back via modelLoadAcknowledge(). This keeps the
// sequence independent of any particular GUI (so a B&W view can be added later).

enum ModelLoadState : uint8_t {
  MLS_IDLE = 0,         // NORMAL: nothing pending, mixer running
  MLS_CHECK_SD,         // ack-alert:  SD card full
  MLS_CHECK_THROTTLE,   // position warning: throttle not idle
  MLS_CHECK_SWITCHES,   // position warning: switches/pots not in warning position
  MLS_CHECK_FAILSAFE,   // ack-alert:  failsafe not set
  MLS_CHECK_MULTI,      // ack-alert:  MULTI module low power (skipped if !MULTIMODULE)
  MLS_CHECK_CHECKLIST,  // model checklist / notes
  MLS_START_PULSES,     // terminal: run model-load tail, then -> MLS_IDLE
};

// Arm the machine at the first check state (called from postModelLoad on a
// runtime model switch). Resets any in-progress sequence.
void modelLoadStart();

// Advance the machine. Polled once per perMain() iteration, before the UI loop.
void modelLoadStateMachineRun();

// View contract -------------------------------------------------------------

// The check the view should currently display a warning dialog for, or MLS_IDLE
// if no dialog should be shown.
ModelLoadState modelLoadActiveWarning();

// Message text for the currently active warning (currently only the throttle
// warning uses a dynamic string; valid while modelLoadActiveWarning() returns
// MLS_CHECK_THROTTLE).
const char* modelLoadWarningText();

// Report a user "press any key to skip" acknowledge to the machine.
void modelLoadAcknowledge();

// True when the machine is idle (no model-load sequence pending). Used to gate
// flightReset so its (blocking) checkAll() cannot run on top of the machine.
bool modelLoadIdle();
