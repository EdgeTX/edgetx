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

constexpr coord_t INPUT_EDIT_CURVE_WIDTH = 132;
constexpr coord_t INPUT_EDIT_CURVE_HEIGHT = INPUT_EDIT_CURVE_WIDTH;
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 8) + 8;

// Disable rotary encoder, as the PL18 does not have one
#define ROTARY_ENCODER_SPEED() 0
