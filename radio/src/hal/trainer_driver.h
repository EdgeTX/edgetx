/*
 * Copyright (C) EdgeTx
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

// Startup init
void board_trainer_init();

// DSC port available?
bool trainer_dsc_available();
    
// Output mode
void trainer_init_dsc_out();

// Input mode
void trainer_init_dsc_in();

// Stop input/output
void trainer_stop_dsc();

// Cable inserted?
bool is_trainer_dsc_connected();

// Start/stop CPPM from module bay
void trainer_init_module_cppm();
void trainer_stop_module_cppm();
