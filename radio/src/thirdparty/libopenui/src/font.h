/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include <lvgl/lvgl.h>
#include "libopenui_types.h"

const lv_font_t* getFont(LcdFlags flags);
uint8_t getFontHeight(LcdFlags flags);
uint8_t getFontHeightCondensed(LcdFlags flags);
int getTextWidth(const char * s, int len = 0, LcdFlags flags = 0);

