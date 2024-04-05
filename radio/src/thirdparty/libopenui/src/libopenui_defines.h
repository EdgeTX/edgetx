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

#include "debug.h"
#include "hal.h"
#include "colors.h"
#include "keys.h"
#include "opentx_types.h"

constexpr uint32_t MENU_HEADER_BUTTON_WIDTH =      33;
constexpr uint32_t MENU_HEADER_BUTTONS_LEFT =      47;

constexpr uint32_t MENU_HEADER_HEIGHT =            45;
constexpr uint32_t MENU_TITLE_TOP =                48;
constexpr uint32_t MENU_TITLE_HEIGHT =             21;
constexpr uint32_t MENU_BODY_TOP =                 MENU_TITLE_TOP + MENU_TITLE_HEIGHT;
constexpr uint32_t MENU_BODY_HEIGHT =              LCD_H - MENU_BODY_TOP;
constexpr uint32_t MENUS_MARGIN_LEFT =             6;

constexpr uint32_t PAGE_LINE_HEIGHT =              20;
constexpr uint32_t FH =                            PAGE_LINE_HEIGHT;
constexpr uint32_t NUM_BODY_LINES =                MENU_BODY_HEIGHT / PAGE_LINE_HEIGHT;

constexpr uint32_t PAGE_TITLE_TOP =                2;
constexpr uint32_t PAGE_TITLE_LEFT =               50;

// Used by Lua API
#define INVERS                         0x01u
#define BLINK                          0x1000u
#define TIMEHOUR                       0x2000u

/* drawText flags */
#define LEFT                           0x00u /* align left */
#define VCENTERED                      0x02u /* align center vertically */
#define CENTERED                       0x04u /* align center */
#define RIGHT                          0x08u /* align right */
#define SHADOWED                       0x80u /* black copy at +1 +1 */
// 0x1000u used by Lua in api_colorlcd.h
// 0x8000u used by Lua in api_colorlcd.h

/* drawNumber flags */
#define LEADING0                       0x10u
#define PREC1                          0x20u
#define PREC2                          0x30u
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

/* telemetry flags */
#define NO_UNIT                        0x40u

#define LV_OBJ_FLAG_ENCODER_ACCEL LV_OBJ_FLAG_USER_1

constexpr coord_t MENUS_LINE_HEIGHT = 35;

#if LCD_W > LCD_H
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 7) + 8;
#else
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 10);
#endif
