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

#include "layout.h"

// Zone map: 1x4 (1 column, 4 rows)
// Each zone is full width, 1/4 height
// clang-format off
static const uint8_t zmap[] = {
    LAYOUT_MAP_0, LAYOUT_MAP_0,    LAYOUT_MAP_FULL, LAYOUT_MAP_1QTR,  // Top
    LAYOUT_MAP_0, LAYOUT_MAP_1QTR, LAYOUT_MAP_FULL, LAYOUT_MAP_1QTR,  // 2nd
    LAYOUT_MAP_0, LAYOUT_MAP_HALF, LAYOUT_MAP_FULL, LAYOUT_MAP_1QTR,  // 3rd
    LAYOUT_MAP_0, LAYOUT_MAP_3QTR, LAYOUT_MAP_FULL, LAYOUT_MAP_1QTR,  // Bottom
};
// clang-format on

BaseLayoutFactory<Layout> Layout1x4("Layout1x4", "1 x 4", defaultLayoutOptions,
                                    4, (uint8_t*)zmap);
