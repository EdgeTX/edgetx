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

#include <keys.h>

#define MESSAGEBOX_X                   8
#define MESSAGEBOX_Y                   8
#define MESSAGEBOX_W                   (LCD_W - 15)

#define MENU_X                         MESSAGEBOX_X
#define MENU_Y                         MESSAGEBOX_Y
#define MENU_W                         MESSAGEBOX_W

#define WARNING_LINE_LEN               24
#define WARNING_LINE_X                 12
#define WARNING_LINE_Y                 MESSAGEBOX_Y + 2

#define POPUP_MENU_MAX_LINES         13
#define MENU_MAX_DISPLAY_LINES       6
#define MENU_LINE_LENGTH             (LEN_MODEL_NAME+12)

enum {
  MENU_OFFSET_INTERNAL,
  MENU_OFFSET_EXTERNAL
};

#include "../common/stdlcd/popups.h"
