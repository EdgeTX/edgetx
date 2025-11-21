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
#include "popups.h"
#include "widgets_common.h"

#include "navigation/navigation.h"
#include "draw_functions.h"

#define MODEL_BITMAP_WIDTH             64
#define MODEL_BITMAP_HEIGHT            32
#define MODEL_BITMAP_SIZE              BITMAP_BUFFER_SIZE(MODEL_BITMAP_WIDTH, MODEL_BITMAP_HEIGHT)
#define LOAD_MODEL_BITMAP()            loadModelBitmap(g_model.header.bitmap, modelBitmap)

#define MIXES_2ND_COLUMN               (18*FW)

#define MENUS_SCROLLBAR_WIDTH          2

extern uint8_t modelBitmap[MODEL_BITMAP_SIZE];
bool loadModelBitmap(char * name, uint8_t * bitmap);

void drawColumnHeader(const char * const * headers, uint8_t index);

void copySelection(char * dst, const char * src, uint8_t size);

extern coord_t scrollbar_X;
#define SET_SCROLLBAR_X(x) scrollbar_X = (x);

#define IS_OTHER_VIEW_DISPLAYED()      (menuHandlers[0] == menuMainViewChannelsMonitor || menuHandlers[0] == menuChannelsView)
