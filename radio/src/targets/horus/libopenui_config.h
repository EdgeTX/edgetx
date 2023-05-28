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

#include "debug.h"
#include "libopenui_defines.h"

#include "colors.h"
#include "board.h"
#include "keys.h"

constexpr uint32_t MENU_HEADER_BUTTON_WIDTH =      33;
constexpr uint32_t MENU_HEADER_BUTTONS_LEFT =      47;

constexpr uint32_t MENU_HEADER_HEIGHT =            45;
constexpr uint32_t MENU_TITLE_TOP =                48;
constexpr uint32_t MENU_TITLE_HEIGHT =             21;
constexpr uint32_t MENU_BODY_TOP =                 MENU_TITLE_TOP + MENU_TITLE_HEIGHT;
constexpr uint32_t MENU_FOOTER_HEIGHT =            0;
constexpr uint32_t MENU_FOOTER_TOP =               LCD_H - MENU_FOOTER_HEIGHT;
constexpr uint32_t MENU_BODY_HEIGHT =              MENU_FOOTER_TOP - MENU_BODY_TOP;
constexpr uint32_t MENUS_MARGIN_LEFT =             6;

constexpr uint32_t MENU_HEADER_BACK_BUTTON_WIDTH  = MENU_HEADER_HEIGHT;
constexpr uint32_t MENU_HEADER_BACK_BUTTON_HEIGHT = MENU_HEADER_HEIGHT;

constexpr coord_t PAGE_PADDING =                   6;
constexpr uint32_t PAGE_LINE_HEIGHT =              20;
constexpr uint32_t FH =                            PAGE_LINE_HEIGHT;
constexpr uint32_t NUM_BODY_LINES =                MENU_BODY_HEIGHT / PAGE_LINE_HEIGHT;

constexpr uint32_t FIELD_PADDING_LEFT =            3;
constexpr uint32_t FIELD_PADDING_TOP =             2;

constexpr uint32_t CURVE_SIDE_WIDTH =              100;
constexpr uint32_t CURVE_CENTER_X =                LCD_W - CURVE_SIDE_WIDTH - 7;
constexpr uint32_t CURVE_CENTER_Y =                151;
constexpr uint32_t CURVE_COORD_WIDTH =             36;
constexpr uint32_t CURVE_COORD_HEIGHT =            17;

constexpr uint32_t DATETIME_SEPARATOR_X =          LCD_W - 53;
constexpr uint32_t DATETIME_LINE1 =                7;
constexpr uint32_t DATETIME_LINE2 =                22;
constexpr uint32_t DATETIME_MIDDLE =               (LCD_W + DATETIME_SEPARATOR_X + 1) / 2;

constexpr uint32_t RSSI_X =                        LCD_W - 90;
constexpr uint32_t AUDIO_X =                       LCD_W - 130;
constexpr uint32_t USB_X =                         LCD_W - 98;
constexpr uint32_t LOG_X =                         LCD_W - 98;
constexpr uint32_t GPS_X =                         LCD_W - 148;

constexpr uint32_t MENUS_TOOLBAR_BUTTON_WIDTH =    30;
constexpr uint32_t MENUS_TOOLBAR_BUTTON_PADDING =  3;
constexpr rect_t MENUS_TOOLBAR_RECT =              { 100, 51, 30, 209 };

constexpr uint32_t ALERT_FRAME_TOP =               70;
constexpr uint32_t ALERT_FRAME_HEIGHT =            (LCD_H - 2 * ALERT_FRAME_TOP);
constexpr uint32_t ALERT_BITMAP_TOP =              ALERT_FRAME_TOP + 15;
constexpr uint32_t ALERT_BITMAP_LEFT =             40;
constexpr uint32_t ALERT_TITLE_TOP =               ALERT_FRAME_TOP + 10;
constexpr uint32_t ALERT_TITLE_LEFT =              186;
constexpr uint32_t ALERT_MESSAGE_TOP =             ALERT_TITLE_TOP + 90;
constexpr uint32_t ALERT_MESSAGE_LEFT =            ALERT_TITLE_LEFT;

constexpr uint32_t PAGE_TITLE_TOP =                2;
constexpr uint32_t PAGE_TITLE_LEFT =               50;

constexpr coord_t INPUT_EDIT_CURVE_WIDTH = 132;
constexpr coord_t INPUT_EDIT_CURVE_HEIGHT = INPUT_EDIT_CURVE_WIDTH;
constexpr coord_t MENUS_LINE_HEIGHT = 30;
constexpr coord_t MENUS_WIDTH = 200;
constexpr coord_t POPUP_HEADER_HEIGHT = 30;
constexpr coord_t MENUS_MAX_HEIGHT = LCD_H * 0.9;
constexpr coord_t MODEL_SELECT_FOOTER_HEIGHT = 24;

#define ROTARY_ENCODER_SPEED() rotencSpeed
