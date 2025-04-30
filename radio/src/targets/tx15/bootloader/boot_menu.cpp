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
#include "bsp_io.h"
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
  backlightInit();
  backlightEnable(100);
}

static void bootloaderDrawTitle(const char* text)
{
  lcd->drawText(LCD_W / 2, DEFAULT_PADDING, text, CENTERED | BL_FOREGROUND);
  lcd->drawSolidFilledRect(DEFAULT_PADDING, DOUBLE_PADDING,
                           LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DEFAULT_PADDING + 10),
                           LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground() { lcd->clear(BL_BACKGROUND); }

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
  lcdInitDirectDrawing();
  bootloaderDrawBackground();
  bootloaderDrawTitle(BOOTLOADER_TITLE);

  if (st == ST_START || st == ST_USB) {

    const char* msg = nullptr;
    if (st == ST_START) {
      msg = "Plug USB cable";
    } else if (st == ST_USB) {
      msg = "Copy firmware.uf2 to EDGETX_UF2 drive";
    }

    coord_t x = LCD_W/2;
    coord_t y = LCD_H/2;
    lcd->drawText(x, y - 10, msg, CENTERED | BL_FOREGROUND);
    lcd->drawText(x, y + 10, "[RTN] to exit", CENTERED | BL_FOREGROUND);

    bootloaderDrawFooter();

    const char* fw_ver = getFirmwareVersion();
    if (!fw_ver) fw_ver = "no version";
    lcd->drawText(x, LCD_H - DEFAULT_PADDING, fw_ver,
                  CENTERED | BL_FOREGROUND);

  } else if (st == ST_FLASHING || st == ST_FLASH_DONE) {

    const coord_t pb_h = 31;
    const coord_t pb_x = 70;
    const coord_t pb_y = (LCD_H - pb_h) / 2;

    const char* msg = (st == ST_FLASH_DONE) ? TR_BL_WRITING_COMPL : TR_BL_WRITING_FW;
    lcd->drawText(pb_x, pb_y - 28, msg, BL_FOREGROUND);
    lcd->drawRect(pb_x, pb_y, 340, pb_h, 2, SOLID, BL_SELECTED);

    LcdFlags color = (st == ST_FLASH_DONE) ? BL_GREEN : BL_RED;
    lcd->drawSolidFilledRect(pb_x + 4, pb_y + 4, (332 * opt) / 100, 23, color);
  }
}

void bootloaderDrawDFUScreen()
{
  lcdInitDirectDrawing();
  bootloaderDrawBackground();
  lcd->drawText(LCD_W / 2, LCD_H / 2, "DFU mode", CENTERED | BL_FOREGROUND);
}

void blExit(void)
{
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
}
