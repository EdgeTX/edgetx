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

#include "gui_common.h"
#include "menus.h"
#include "widgets_common.h"

#include "navigation/navigation.h"
#include "draw_functions.h"

#define MENUS_SCROLLBAR_WIDTH          0

#define MIXES_2ND_COLUMN               (10*FW)

void drawWheel(coord_t centrex, int16_t wval);
void drawThrottle(coord_t centrex, int16_t tval);

void drawProgressScreen(const char * title, const char * message, int num, int den);

#define SET_SCROLLBAR_X(x)
#define LOAD_MODEL_BITMAP()

#define IS_OTHER_VIEW_DISPLAYED()      menuHandlers[0] == menuChannelsView
