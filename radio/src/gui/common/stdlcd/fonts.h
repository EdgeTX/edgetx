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

constexpr int FONT_BASE_START = 0x20;
constexpr int FONT_SYMS_START = 0x80;
constexpr int FONT_LANG_START = 0x95;

constexpr int FONT_BASE_CNT = 96;       // Number of characters in standard 5x7 font
constexpr int FONT_BASE_CNT_10x14 = 68; // Number of characters in compressed 10x14 font

constexpr int FONT_SYMS_CNT = 21;       // Max # of extra symbols (based on 5x7 standard font)
constexpr int FONT_SYMS_CNT_4x6 = 8;    // # of extra symbols in 4x6 font (adjust if font is changed)
constexpr int FONT_SYMS_CNT_10x14 = 8;  // # of extra symbols in 10x14 font (adjust if font is changed)

extern const unsigned char font_5x7[];
extern const unsigned char font_5x7_B[];
extern const unsigned char font_3x5[];
extern const unsigned char font_4x6[];
extern const unsigned char font_8x10[];
extern const unsigned char font_10x14[];
extern const unsigned char font_22x38_num[];
