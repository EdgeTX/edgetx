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

#ifndef _POPUPS_H_
#define _POPUPS_H_

#include <keys.h>

#define MESSAGEBOX_X                   10
#if LCD_H > 64
#define MESSAGEBOX_Y                   25
#else
#define MESSAGEBOX_Y                   16
#endif
#define MESSAGEBOX_W                   (LCD_W - 19)

#define MENU_X                         MESSAGEBOX_X
#define MENU_Y                         MESSAGEBOX_Y
#define MENU_W                         MESSAGEBOX_W

#define WARNING_LINE_LEN               20
#define WARNING_LINE_X                 16
#if LCD_H > 64
#define WARNING_LINE_Y                 4*FH
#else
#define WARNING_LINE_Y                 3*FH
#endif

#if LCD_H > 64
#define POPUP_MENU_MAX_LINES         24
#else
#define POPUP_MENU_MAX_LINES         12
#endif
#define MENU_MAX_DISPLAY_LINES       6
#define MENU_LINE_LENGTH             (LEN_MODEL_NAME+12)

enum {
  MENU_OFFSET_INTERNAL,
  MENU_OFFSET_EXTERNAL
};

#include "../common/stdlcd/popups.h"

#endif // _POPUPS_H_
