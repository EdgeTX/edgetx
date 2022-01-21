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

#define CHECKSUM_SUCCESS 0
#define CHECKSUM_FAILED  1
#define CHECKSUM_NONE    2

constexpr uint8_t MODELIDX_STRLEN = sizeof(MODEL_FILENAME_PREFIX "00");

const char * loadRadioSettingsYaml();
const char * writeModelYaml(const char* filename);
const char * readModelYaml(const char * filename, uint8_t * buffer, uint32_t size);
void getModelNumberStr(uint8_t idx, char* model_idx);
