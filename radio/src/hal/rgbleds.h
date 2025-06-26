/*
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
// MUST match TR_FS_COLOR_LIST (except 'Custom') -  Custom gets displayed when none match
//                                    "Off",  "White",    "Red",  "Green", "Yellow", "Orange",   "Blue",   "Pink"
constexpr uint32_t colorTable[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0xFFFF00, 0xFF4000, 0x0000FF, 0xFF00FF};

void setFSLedOverride(uint8_t index, bool state, uint8_t r, uint8_t g, uint8_t b);
void setFSLedOFF(uint8_t index);
void setFSLedON(uint8_t index);
bool getFSLedState(uint8_t index);
uint32_t getFSLedRGBColor(uint8_t index);
