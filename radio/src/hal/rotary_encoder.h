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

#define ROTENC_LOWSPEED   1
#define ROTENC_MIDSPEED   5
#define ROTENC_HIGHSPEED 50

#if defined(RADIO_FAMILY_T20) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_T15) || defined(RADIO_BUMBLEBEE)
#define ROTARY_ENCODER_GRANULARITY 4
#else
#define ROTARY_ENCODER_GRANULARITY 2
#endif

typedef int32_t rotenc_t;

void rotaryEncoderInit();

// return impulses / granularity
rotenc_t rotaryEncoderGetValue();

int8_t rotaryEncoderGetAccel();
void rotaryEncoderResetAccel();
