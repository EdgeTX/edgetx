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
void init_trainer();

// Output mode
void init_trainer_ppm();
void stop_trainer_ppm();

// Input mode
void init_trainer_capture();
void stop_trainer_capture();

// Cable inserted?
bool is_trainer_connected();

#if defined(TRAINER_MODULE_CPPM)
void init_trainer_module_cppm();
void stop_trainer_module_cppm();
#endif

#if defined(TRAINER_MODULE_SBUS)
void init_trainer_module_sbus();
void stop_trainer_module_sbus();
int trainerModuleSbusGetByte(uint8_t* byte);
#endif
