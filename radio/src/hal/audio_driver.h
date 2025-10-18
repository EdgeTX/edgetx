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

// Audio volume is defined between 0 and VOLUME_LEVEL_MAX.
#define VOLUME_LEVEL_MAX 23
#define VOLUME_LEVEL_DEF 12

#define AUDIO_SAMPLE_RATE 32000

#define AUDIO_SAMPLE_FMT_S16 0
#define AUDIO_SAMPLE_FMT_U16 1

bool audioHeadphoneDetect();
void audioSetVolume(uint8_t volume);
void audioConsumeCurrentBuffer();

