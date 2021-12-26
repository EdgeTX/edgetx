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
LZ4Bitmap BMP_PLUG_USB(BMP_ARGB4444, __bmp_plug_usb);

const uint8_t __bmp_usb_plugged[] {
#include "bmp_usb_plugged.lbm"
};
LZ4Bitmap BMP_USB_PLUGGED(BMP_ARGB4444, __bmp_usb_plugged);

const uint8_t __bmp_background[] {
#include "bmp_background.lbm"
};
LZ4Bitmap BMP_BACKGROUND(BMP_ARGB4444, __bmp_background);

#define BL_GREEN      COLOR2FLAGS(RGB(73, 219, 62))
#define BL_RED        COLOR2FLAGS(RGB(229, 32, 30))
#define BL_BACKGROUND COLOR2FLAGS(BLACK)
#define BL_FOREGROUND COLOR2FLAGS(WHITE)
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
  lcd->drawSolidFilledRect(28, 234, 422, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
  // we have plenty of memory, let's cache that background
  static BitmapBuffer* _background = nullptr;

  if (!_background) {
    _background = new BitmapBuffer(BMP_RGB565, LCD_W, LCD_H);
    
    for (int i=0; i<LCD_W; i += BMP_BACKGROUND.width()) {
      for (int j=0; j<LCD_H; j += BMP_BACKGROUND.height()) {
        BitmapBuffer* bg_bmp = &BMP_BACKGROUND;
        _background->drawBitmap(i, j, bg_bmp);
      }
    }
  }

  if (_background) {
    lcd->drawBitmap(0, 0, _background);
    lcd->drawFilledRect(0, 0, LCD_W, LCD_H, SOLID,
                        COLOR2FLAGS(BLACK), OPACITY(4));
  }
  else {
    lcd->clear(BL_BACKGROUND);
  }
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
      lcd->drawText(100, 110, LV_SYMBOL_WARNING, BL_FOREGROUND);
      pos = lcd->drawText(124, 110, "Erase Flash Storage", BL_FOREGROUND);
      pos += 8;

      lcd->drawText(100, 145, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(124, 145, "Exit", BL_FOREGROUND);
#else
      lcd->drawText(100, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(124, 110, TR_BL_EXIT, BL_FOREGROUND);
#endif

      pos -= 92;
      lcd->drawSolidRect(92, 72 + (opt*35), pos, 26, 2, BL_SELECTED);

      lcd->drawBitmap(60, 166, (const BitmapBuffer*)&BMP_PLUG_USB);
      lcd->drawText(195, 175, TR_BL_USB_PLUGIN, BL_FOREGROUND);
      lcd->drawText(195, 200, TR_BL_USB_MASS_STORE, BL_FOREGROUND);

      bootloaderDrawFooter();
      lcd->drawText(LCD_W / 2, 242, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    }
#if defined(SPI_FLASH) && defined(SDCARD)
    else if (st == ST_SELECT_STORAGE) {

        bootloaderDrawTitle(88, LV_SYMBOL_DIRECTORY " select storage");

        lcd->drawText(102, 75, LV_SYMBOL_DIRECTORY, BL_FOREGROUND);
        coord_t pos = lcd->drawText(124, 75, "Internal", BL_FOREGROUND);

        pos += 8;

        lcd->drawText(100, 110, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
        lcd->drawText(124, 110, "SD Card", BL_FOREGROUND);

        pos -= 92;
        lcd->drawSolidRect(92, (opt == 0) ? 72 : 107, pos, 26, 2, BL_SELECTED);

        bootloaderDrawFooter();
        lcd->drawText(56, 244, "[ENT] to select storage", BL_FOREGROUND);
        lcd->drawText(305, 244, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(335, 244, "[RTN] to exit", BL_FOREGROUND);
    }
#endif
#if defined(SPI_FLASH)
    else if (st == ST_CLEAR_FLASH_CHECK) {

        bootloaderDrawTitle(88, "erase internal flash storage");

        lcd->drawText(100, 75, LV_SYMBOL_DRIVE, BL_FOREGROUND);
        coord_t pos = lcd->drawText(124, 75, "Erase Flash Storage", BL_FOREGROUND);
        pos += 8;

        lcd->drawText(102, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(124, 110, "Exit", BL_FOREGROUND);

        pos -= 92;
        lcd->drawSolidRect(92, (opt == 0) ? 72 : 107, pos, 26, 2, BL_SELECTED);

        bootloaderDrawFooter();
        lcd->drawText(56, 244, "Hold [ENT] long to erase storage", BL_FOREGROUND);
        lcd->drawText(305, 244, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(335, 244, "[RTN] to exit", BL_FOREGROUND);
    }
    else if (st == ST_CLEAR_FLASH) {
        bootloaderDrawTitle(88, "erasing internal flash storage");

        lcd->drawText(102, 75, "This may take up to 150s", BL_FOREGROUND);
        bootloaderDrawFooter();
    }
#endif
    else if (st == ST_USB) {

        lcd->drawBitmap(136, 98, (const BitmapBuffer*)&BMP_USB_PLUGGED);
        lcd->drawText(195, 128, TR_BL_USB_CONNECTED, BL_FOREGROUND);
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

          lcd->drawRect(70, 120, 340, 31, 2, SOLID, BL_SELECTED);
          lcd->drawSolidFilledRect(74, 124, (332 * opt) / 100, 23, color);
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

            lcd->drawText(168, 138, TR_BL_FORK, RIGHT | BL_FOREGROUND);
            lcd->drawSizedText(174, 138, tag.fork, 6, BL_FOREGROUND);

            lcd->drawText(168, 158, TR_BL_VERSION, RIGHT | BL_FOREGROUND);
            lcd->drawText(174, 158, tag.version, BL_FOREGROUND);

            lcd->drawText(168, 178, TR_BL_RADIO, RIGHT | BL_FOREGROUND);
            lcd->drawText(174, 178, tag.flavour, BL_FOREGROUND);

            lcd->drawText(356, 156, LV_SYMBOL_OK, BL_GREEN);
          }
        }

        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawText(28, 242, LV_SYMBOL_CHARGE, BL_FOREGROUND);

            if (st == ST_FILE_LIST) {
                lcd->drawText(56, 244, TR_BL_SELECT_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(56, 244, TR_BL_FLASH_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(56, 244, TR_BL_WRITING_FW, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(56, 244, TR_BL_WRITING_COMPL, BL_FOREGROUND);
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawText(305, 244, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
            lcd->drawText(335, 244, TR_BL_EXIT_KEY, BL_FOREGROUND);
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
