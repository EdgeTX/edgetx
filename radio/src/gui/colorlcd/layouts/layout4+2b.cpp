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

// Zone map: 4+2B (4 small zones + 2 zones, bottom-biased)
// Left: 4 zones stacked (1/2 width, 1/4 height each), Right: 1 large zone (3/4
// height) + 1 small zone (1/4 height)
// clang-format off
static const uint8_t zmap[] = {
    LAYOUT_MAP_0,    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Left top
    LAYOUT_MAP_0,    LAYOUT_MAP_1QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Left 2nd
    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Left 3rd
    LAYOUT_MAP_0,    LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Left bottom
    LAYOUT_MAP_HALF, LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_3QTR,  // Right large (3/4 height)
    LAYOUT_MAP_HALF, LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Right small (1/4 height)
};
// clang-format on

BaseLayoutFactory<Layout> layout4P2B("Layout4P2B", "4 + 2B",
                                     defaultLayoutOptions, 6, (uint8_t*)zmap);
