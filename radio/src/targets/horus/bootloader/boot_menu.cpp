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
#include "targets/common/arm/stm32/bootloader/bin_files.h"

#include <lvgl/lvgl.h>

const uint8_t __bmp_plug_usb[] {
#include "bmp_plug_usb.lbm"
};
LZ4BitmapBuffer BMP_PLUG_USB(BMP_ARGB4444, (LZ4Bitmap*)__bmp_plug_usb);

const uint8_t __bmp_usb_plugged[] {
#include "bmp_usb_plugged.lbm"
};
LZ4BitmapBuffer BMP_USB_PLUGGED(BMP_ARGB4444, (LZ4Bitmap*)__bmp_usb_plugged);

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

static void bootloaderDrawTitle(unsigned int x, const char* text)
{
  lcd->drawText(x, 28, text, BL_FOREGROUND);
  lcd->drawSolidFilledRect(28, 56, 422, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(28, LCD_H - 38, 422, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
  lcd->clear(BL_BACKGROUND);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
    static bool _first_screen = true;
    
    lcdInitDirectDrawing();
    bootloaderDrawBackground();

    if (!_first_screen) {
        // ... and turn backlight ON
        backlightEnable(BACKLIGHT_LEVEL_MAX);
    }
    
    if (st == ST_START) {
      bootloaderDrawTitle(88, BOOTLOADER_TITLE);

      lcd->drawText(102, 75, LV_SYMBOL_CHARGE, BL_FOREGROUND);
      coord_t pos = lcd->drawText(124, 75, TR_BL_WRITE_FW, BL_FOREGROUND);
      pos += 8;

#if defined(SPI_FLASH)
      lcd->drawText(102, 110, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
      pos = lcd->drawText(124, 110, TR_BL_ERASE_FLASH, BL_FOREGROUND);
      pos += 8;

      lcd->drawText(100, 145, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(124, 145, TR_BL_EXIT, BL_FOREGROUND);
#else
      lcd->drawText(100, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(124, 110, TR_BL_EXIT, BL_FOREGROUND);
#endif

      pos -= 92;
      lcd->drawSolidRect(92, 72 + (opt * 35), pos, 26, 2, BL_SELECTED);

      lcd->drawBitmap(60, LCD_H - 106, (const BitmapBuffer*)&BMP_PLUG_USB);
      lcd->drawText(195, LCD_H - 97, TR_BL_USB_PLUGIN, BL_FOREGROUND);
      lcd->drawText(195, LCD_H - 72, TR_BL_USB_MASS_STORE, BL_FOREGROUND);

      bootloaderDrawFooter();
      lcd->drawText(LCD_W / 2, LCD_H - 30, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    }
#if defined(SPI_FLASH)
    else if (st == ST_CLEAR_FLASH_CHECK) {

        bootloaderDrawTitle(88, TR_BL_ERASE_INT_FLASH);

        lcd->drawText(102, 75, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
        coord_t pos = lcd->drawText(124, 75, TR_BL_ERASE_FLASH, BL_FOREGROUND);
        pos += 8;

        lcd->drawText(100, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(124, 110, TR_BL_EXIT, BL_FOREGROUND);

        pos -= 92;
        lcd->drawSolidRect(92, 72 + (opt * 35), pos, 26, 2, BL_SELECTED);

        bootloaderDrawFooter();
        lcd->drawText(56, 244,
                      LV_SYMBOL_SD_CARD TR_BL_ERASE_KEY, BL_FOREGROUND);
        lcd->drawText(305, 244,
                      LV_SYMBOL_NEW_LINE TR_BL_EXIT_KEY, BL_FOREGROUND);
    }
    else if (st == ST_CLEAR_FLASH) {
        bootloaderDrawTitle(88, TR_BL_ERASE_INT_FLASH);

        lcd->drawText(LCD_W / 2, 75, TR_BL_ERASE_FLASH_MSG, CENTERED | BL_FOREGROUND);
        bootloaderDrawFooter();
    }
#endif
    else if (st == ST_USB) {
        coord_t y = (LCD_H - BMP_USB_PLUGGED.height()) / 2;
        lcd->drawBitmap(136, y, (const BitmapBuffer*)&BMP_USB_PLUGGED);
        lcd->drawText(195, y + 30, TR_BL_USB_CONNECTED, BL_FOREGROUND);
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle(88, LV_SYMBOL_SD_CARD " /FIRMWARE");

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {
          LcdFlags color = BL_RED;

          if (st == ST_FLASH_DONE) {
            color = BL_GREEN;
            opt = 100;  // Completed > 100%
          }

          lcd->drawRect(70, (LCD_H - 31) / 2, 340, 31, 2, SOLID, BL_SELECTED);
          lcd->drawSolidFilledRect(74, (LCD_H - 31) / 2 + 4, (332 * opt) / 100, 23, color);
        } else if (st == ST_DIR_CHECK) {
          if (opt == FR_NO_PATH) {
            lcd->drawText(90, 168, LV_SYMBOL_CLOSE TR_BL_DIR_MISSING,
                          BL_FOREGROUND);
          } else {
            lcd->drawText(90, 168, LV_SYMBOL_CLOSE TR_BL_DIR_EMPTY,
                          BL_FOREGROUND);
          }
        } else if (st == ST_FLASH_CHECK) {
          bootloaderDrawFilename(str, 0, true);

          if (opt == FC_ERROR) {
            lcd->drawText(94, 168, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                          BL_FOREGROUND);
          } else if (opt == FC_OK) {
            VersionTag tag;
            memset(&tag, 0, sizeof(tag));
            extractFirmwareVersion(&tag);

            if (strcmp(tag.flavour, FLAVOUR)) {
              lcd->drawText(94, 168, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                      BL_FOREGROUND);
            } else {
              lcd->drawText(168, 138, TR_BL_FORK, RIGHT | BL_FOREGROUND);
              lcd->drawSizedText(174, 138, tag.fork, 6, BL_FOREGROUND);

              lcd->drawText(168, 158, TR_BL_VERSION, RIGHT | BL_FOREGROUND);
              lcd->drawText(174, 158, tag.version, BL_FOREGROUND);

              lcd->drawText(168, 178, TR_BL_RADIO, RIGHT | BL_FOREGROUND);
              lcd->drawText(174, 178, tag.flavour, BL_FOREGROUND);

              lcd->drawText(78, 158, LV_SYMBOL_OK, BL_GREEN);
            }
          }
        }

        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawText(28, LCD_H - 30, LV_SYMBOL_CHARGE, BL_FOREGROUND);

            if (st == ST_FILE_LIST) {
                lcd->drawText(56, LCD_H - 28, TR_BL_SELECT_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(56, LCD_H - 28, TR_BL_FLASH_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(56, LCD_H - 28, TR_BL_WRITING_FW, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(56, LCD_H - 28, TR_BL_WRITING_COMPL, BL_FOREGROUND);
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawText(305, LCD_H - 28, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
            lcd->drawText(335, LCD_H - 28, TR_BL_EXIT_KEY, BL_FOREGROUND);
        }        
    }

    _first_screen = false;
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawText(94, 75 + (line * 25), LV_SYMBOL_FILE, BL_FOREGROUND);
    lcd->drawText(124, 75 + (line * 25), str, BL_FOREGROUND);

    if (selected) {
        lcd->drawSolidRect(119, 72 + (line * 25), 278, 26, 2, BL_SELECTED);
    }
}

uint32_t bootloaderGetMenuItemCount(int baseCount)
{
    return baseCount;
}

bool bootloaderRadioMenu(uint32_t menuItem, event_t event)
{
    return true;
}

void blExit(void)
{
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
}
