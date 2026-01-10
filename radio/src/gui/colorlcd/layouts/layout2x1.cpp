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

// Zone map: 2x1 (2 columns, 1 row)
// Each zone is 1/2 width, full height
// clang-format off
static const uint8_t zmap[] = {
    LAYOUT_MAP_0,    LAYOUT_MAP_0, LAYOUT_MAP_HALF, LAYOUT_MAP_FULL,  // Left
    LAYOUT_MAP_HALF, LAYOUT_MAP_0, LAYOUT_MAP_HALF, LAYOUT_MAP_FULL,  // Right
};
// clang-format on

BaseLayoutFactory<Layout> Layout2x1("Layout2x1", "2 x 1", defaultLayoutOptions,
                                    2, (uint8_t*)zmap);
