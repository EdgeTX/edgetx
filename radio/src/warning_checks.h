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

// Startup/safety warning check sequence, modelled as a GUI-agnostic state
// machine. Shared by every entry point that has to run the throttle/switch/
// failsafe/etc. warnings: a runtime model switch, boot, a flight reset, and the
// radio-setup stick-mode change (see WarningCheckContext).
//
// For the contexts that stop the mixer/pulses first (a model switch stops them in
// preModelLoad, the stick-mode change stops the mixer), the RF module is silent
// and the receiver holds failsafe meanwhile. The machine progresses through the
// checks one at a time, and only when it reaches WCS_COMPLETE (all cleared) does
// it run the context-specific terminal action (restart pulses/mixer, model-load
// tail, ...). It is polled once per main loop iteration, so no blocking/nested UI
// loop is needed.
//
// This module owns the truth (which check is active, when to advance, what the
// terminal does). The GUI is only a *view*: it renders activeWarningCheck() and
// feeds key presses back via acknowledgeWarningCheck(), so the sequence is
// independent of any particular GUI (COLORLCD and B&W each have a thin view).

enum WarningCheckState : uint8_t {
  WCS_IDLE = 0,         // NORMAL: nothing pending, mixer running
  WCS_CHECK_SD,         // ack-alert:  SD card full
  WCS_CHECK_THROTTLE,   // position warning: throttle not idle
  WCS_CHECK_SWITCHES,   // position warning: switches/pots not in warning position
  WCS_CHECK_FAILSAFE,   // ack-alert:  failsafe not set
  WCS_CHECK_MULTI,      // ack-alert:  MULTI module low power (skipped if !MULTIMODULE)
  WCS_CHECK_CHECKLIST,  // model checklist / notes
  WCS_COMPLETE,     // terminal: run the context-specific tail, then -> WCS_IDLE
};

// What armed the machine. The set of checks and the terminal action both depend
// on this, so every warning entry point (model switch, boot, flight reset, the
// radio-setup stick-mode change) routes through the one machine instead of its
// own blocking loop.
enum WarningCheckContext : uint8_t {
  WCC_MODEL_SWITCH,   // full sequence; terminal: postModelLoadFinish() (restart pulses + model init)
  WCC_BOOT,           // full sequence; terminal: pulsesStart() (postModelLoadFinish already ran at load)
  WCC_FLIGHT_RESET,   // full sequence; terminal: nothing (pulses were never stopped)
  WCC_STICK_MODE,     // throttle-only; terminal: mixerTaskStart() (caller did mixerTaskStop())
};

// Arm the machine for the given context. Resets any in-progress sequence.
// WCC_MODEL_SWITCH / WCC_BOOT / WCC_FLIGHT_RESET run the full check sequence;
// WCC_STICK_MODE runs only the throttle check.
void warningChecksStart(WarningCheckContext ctx = WCC_MODEL_SWITCH);

// Advance the machine. Polled once per perMain() iteration, before the UI loop.
void warningChecksRun();

// View contract -------------------------------------------------------------

// The check the view should currently display a warning dialog for, or WCS_IDLE
// if no dialog should be shown.
WarningCheckState activeWarningCheck();

// Message text for the currently active warning (currently only the throttle
// warning uses a dynamic string; valid while activeWarningCheck() returns
// WCS_CHECK_THROTTLE).
const char* warningCheckText();

// Report a user "press any key to skip" acknowledge to the machine.
void acknowledgeWarningCheck();

// True when the machine is idle (no model-load sequence pending). Used to gate a
// deferred flight-reset request so it can't restart a sequence already running,
// and at boot to decide whether pulses are started directly or by the terminal.
bool warningChecksIdle();

// Warning predicates (pure reads) and the input refresh they rely on. The state
// machine calls refreshInputsForWarnings() once per tick, then queries the
// predicates; the warning views read the same fresh state.
void refreshInputsForWarnings();
bool isThrottleWarningAlertNeeded();
bool isFailsafeWarningRequired();
#if defined(MULTIMODULE)
bool isMultiLowPowerWarningRequired();
#endif

// Sound-disabled alarm, shown once at boot.
void checkAlarm();

#if defined(GUI)
// Boot-only warning checks that are not part of the runtime warning sequence
// (stuck keys, low RTC battery, external-antenna configuration). Run once,
// blocking, in edgeTxInit() before the main loop and before arming the state
// machine for the post-load checks.
void checkBootSpecificWarnings();
#endif
