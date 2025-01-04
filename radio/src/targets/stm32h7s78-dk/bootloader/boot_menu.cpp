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

#include "board.h"
#include "fw_version.h"
#include "lcd.h"

#include "translations.h"

#include "targets/common/arm/stm32/bootloader/boot.h"
#include "bootloader/firmware_files.h"

#include <lvgl/lvgl.h>

#define SELECTED_COLOR (INVERS | COLOR_THEME_SECONDARY1)
#define DEFAULT_PADDING 28
#define DOUBLE_PADDING  56
#define MESSAGE_TOP     (LCD_H - (2*DOUBLE_PADDING))

#define BL_GREEN      COLOR2FLAGS(RGB(73, 219, 62))
#define BL_RED        COLOR2FLAGS(RGB(229, 32, 30))
#define BL_BACKGROUND COLOR_BLACK
#define BL_FOREGROUND COLOR_WHITE
#define BL_SELECTED   COLOR2FLAGS(RGB(11, 65, 244)) // deep blue

extern BitmapBuffer * lcd;

void bootloaderInitScreen()
{
  lcdInitDisplayDriver();
}

static void bootloaderDrawTitle(const char* text)
{
    lcd->drawText(LCD_W/2, DEFAULT_PADDING, text, CENTERED | BL_FOREGROUND);
    lcd->drawSolidFilledRect(DEFAULT_PADDING, DOUBLE_PADDING, LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
    lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DEFAULT_PADDING + 10), LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
    lcd->clear(BL_BACKGROUND);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
    lcdInitDirectDrawing();
    bootloaderDrawBackground();

    int center = LCD_W/2;
    if (st == ST_START) {
        bootloaderDrawTitle(BOOTLOADER_TITLE);
        lcd->drawText(195, 223, TR_BL_USB_PLUGIN, BL_FOREGROUND);
        lcd->drawText(195, 248, TR_BL_USB_MASS_STORE, BL_FOREGROUND);
        bootloaderDrawFooter();
        // lcd->drawText(center, LCD_H - DEFAULT_PADDING, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    } else if (st == ST_USB) {
      lcd->drawText(center, 168, TR_BL_USB_CONNECTED, CENTERED | BL_FOREGROUND);
    }
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawText(DEFAULT_PADDING, 75 + (line * 25), LV_SYMBOL_FILE, BL_FOREGROUND);
    lcd->drawText(DEFAULT_PADDING + 30, 75 + (line * 25), str, BL_FOREGROUND);

    if (selected) {
        lcd->drawSolidRect(DEFAULT_PADDING + 25, 72 + (line * 25),
                           LCD_W - (DEFAULT_PADDING + 25) - 28, 26, 2, BL_SELECTED);
    }
}

void blExit(void)
{
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
}
