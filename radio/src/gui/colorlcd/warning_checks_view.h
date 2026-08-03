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

// COLORLCD view for the model-load state machine. Polled once per perMain()
// iteration (after MainWindow::run): it mirrors the machine's active warning
// into the matching full-screen dialog and reports user "skip" key presses back
// to the machine. The machine owns all the logic; this is display only.
void warningChecksViewSync();

// Suspend/restore main-view widget refresh for the whole warning sequence. Must
// be called *before* MainWindow::run() in perMain() so widgets are never
// repainted for a tick while a sequence is active (edge-triggered).
void warningChecksViewUpdateRefresh();
