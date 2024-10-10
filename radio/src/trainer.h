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

#include "dataconstants.h"

// Trainer input channels
extern int16_t trainerInput[MAX_TRAINER_CHANNELS];

extern uint8_t currentTrainerMode;

bool isTrainerConnected();
void checkTrainerSignalWarning();

bool isTrainerValid();
void trainerResetTimer();
void trainerDecTimer();
void trainerSetTimer(uint16_t t);

void checkTrainerSettings();
void forceResetTrainerSettings();
void stopTrainer();

// Allows notifications on trainer setting change (old_mode, new_mode)
void trainerSetChangeCb(void (*changeCb)(uint8_t, uint8_t));
