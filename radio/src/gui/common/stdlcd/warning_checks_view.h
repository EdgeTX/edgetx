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

#include "edgetx_types.h"

// B&W view over the model-load state machine (model_load_sm.{h,cpp}). The
// COLORLCD counterpart lives in gui/colorlcd/warning_checks_view.cpp; both render the
// machine's active warning and feed key presses back as an acknowledge, so the
// warning sequence is shared by both GUIs and every entry point (model switch,
// boot, flight reset, stick-mode change).
//
// Called once per main-loop iteration from perMain(), after warningChecksRun().
// When a warning is active it draws it full-screen (so guiMain must be skipped for
// that tick) and turns the tick's key event into acknowledgeWarningCheck().
void warningChecksViewSync(event_t evt);
