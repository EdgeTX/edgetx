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

#include "fullscreen_dialog.h"
#include "mainwindow.h"
#include "edgetx.h"

// Dumb display-only dialogs. Their driver (the boot/flightReset blocking loops
// or the model-load state machine) owns the input refresh, the warning
// predicate and the close decision. Closing on a key press self-deletes as
// usual (default FullScreenDialog behaviour); for the state machine the
// model-load view reports that close as an acknowledge.
class SwitchWarnDialog : public FullScreenDialog
{
 public:
  SwitchWarnDialog();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SwitchWarnDialog"; }
#endif

 protected:
  // rebuilds the bad-switch/pot list text each frame (display only)
  void checkEvents() override;
};

class ThrottleWarnDialog : public FullScreenDialog
{
 public:
  ThrottleWarnDialog(const char* msg);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ThrottleWarnDialog"; }
#endif
};
