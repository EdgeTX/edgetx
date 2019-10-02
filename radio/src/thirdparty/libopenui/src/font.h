/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _FONT_H_
#define _FONT_H_

#include "libopenui_types.h"

uint8_t getMappedChar(uint8_t c);
uint8_t getFontHeight(LcdFlags flags);
int getCharWidth(uint8_t c, const uint16_t * spec);
int getTextWidth(const char * s, int len = 0, LcdFlags flags = 0);

#endif // _FONT_H_
