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

#include <stdint.h>

// Post-filter gain (left shift before int16 saturation; +6 dB per step).
// Same meaning and default as the software PDM driver, so trimSilence() in
// pdm_wav_recorder.cpp stays calibrated across both back-ends.
#ifndef PDM_POST_GAIN_SHIFT
#define PDM_POST_GAIN_SHIFT 5
#endif

// Bring up DFSDM (CKOUT starts clocking the mic) and reset the ring.
// Idempotent. Call when entering a context that needs the mic.
void pdmStart();
// Stop the filter and disable DFSDM — CKOUT stops, the mic goes low-power.
void pdmStop();
bool pdmUpdateSoundLevel();
uint8_t pdmGetSoundLevel();
bool pdmCapture();
uint32_t pdmConvertToPCM(int16_t* pcm, uint32_t max);
