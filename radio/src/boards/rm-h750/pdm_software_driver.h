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

// Decimation factor: output rate = PDM_CLOCK_FREQ / PDM_PCM_DECIMATION.
// R=100, PDM_CLOCK=1.6 MHz → SRC_RATE=16000 Hz exactly (no resampler needed).
#define PDM_PCM_DECIMATION 100

// Post-CIC gain (left shift before int16 saturation; +6 dB per step).
// Default calibrated at 4. Override per target in hal.h before this header
// is reached. trimSilence() in pdm_wav_recorder.cpp scales its threshold
// from this same value, so changing it keeps trim in sync automatically.
#ifndef PDM_POST_GAIN_SHIFT
#define PDM_POST_GAIN_SHIFT 5
#endif

// Bring up SAI clock + TIM15 + DMA capture and reset filter state.
// Idempotent. Call when entering a context that needs the mic.
void pdmStart();
// Stop the DMA pacer, disable SAI block (the PDM clock to the mic stops).
void pdmStop();
bool pdmUpdateSoundLevel();
uint8_t pdmGetSoundLevel();
bool pdmCapture();
uint32_t pdmConvertToPCM(int16_t* pcm, uint32_t max);
