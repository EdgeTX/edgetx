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
#include "etx_lv_theme.h"

#include "translations.h"

#include "targets/common/arm/stm32/bootloader/boot.h"
#include "bootloader/firmware_files.h"

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

LAYOUT_VAL(LINE_X1, 28, 28, LS(28))
LAYOUT_VAL(LINE_W, 422, 422, LS(422))
LAYOUT_VAL(LINE_H, 2, 2, 2)

LAYOUT_VAL(TITLE_X1, 88, 88, LS(88))
LAYOUT_VAL(TITLE_Y1, 28, 28, LS(28))
LAYOUT_VAL(TITLE_Y2, 56, 56, LS(56))

LAYOUT_VAL(FOOTER_Y1, 30, 30, LS(30))
LAYOUT_VAL(FOOTER_Y2, 38, 38, LS(38))

LAYOUT_VAL(SYM_X, 102, 102, LS(102))
LAYOUT_VAL(SYM_X2, 100, 100, LS(100))
LAYOUT_VAL(LBL_X, 124, 124, LS(124))
LAYOUT_VAL(SYM_Y, 75, 75, LS(75))
LAYOUT_VAL(SYM_H, 35, 35, LS(35))

LAYOUT_VAL(BOX_GAP, 8, 8, LS(8))
LAYOUT_VAL(BOX_Y, 72, 72, LS(72))
LAYOUT_VAL(BOX_H, 26, 26, LS(26))

LAYOUT_VAL(OPTBOX_X, 92, 92, LS(92))

LAYOUT_VAL(USB_ICN_X, 60, 60, LS(60))
LAYOUT_VAL(USB_PLG_X, 136, 136, LS(136))
LAYOUT_VAL(USB_ICN_Y, 106, 106, LS(106))
LAYOUT_VAL(USB_TXT_X, 195, 195, LS(195))
LAYOUT_VAL(USB_TXT_Y1, 97, 97, LS(97))
LAYOUT_VAL(USB_TXT_Y2, 72, 72, LS(72))

LAYOUT_VAL(FILENAM_X1, 94, 94, LS(94))
LAYOUT_VAL(FILENAM_X2, 124, 124, LS(124))
LAYOUT_VAL(FILENAM_Y1, 75, 75, LS(75))
LAYOUT_VAL(FILENAM_H, 25, 25, LS(25))
LAYOUT_VAL(FILESEL_X, 119, 119, LS(119))
LAYOUT_VAL(FILESEL_W, 278, 278, LS(278))

LAYOUT_VAL(NLSYM_X, 305, 305, LS(305))
LAYOUT_VAL(RTNTXT_X, 335, 335, LS(335))

LAYOUT_VAL(DIRMSG_X, 90, 90, LS(90))
LAYOUT_VAL(DIRMSG_Y, 168, 168, LS(168))

LAYOUT_VAL(PROGRESS_X, 70, 70, LS(70))
LAYOUT_VAL(PROGRESS_W, 340, 340, LS(340))
LAYOUT_VAL(PROGRESS_H, 31, 31, LS(31))

LAYOUT_VAL(VERCHK_X, 168, 168, LS(168))
LAYOUT_VAL(VERCHK_Y, 138, 138, LS(138))
LAYOUT_VAL(VERCHK_ICN_X, 78, 78, LS(78))

extern BitmapBuffer * lcd;

void bootloaderInitScreen()
{
  lcdInitDisplayDriver();
}

static void bootloaderDrawTitle(unsigned int x, const char* text)
{
  lcd->drawText(x, TITLE_Y1, text, BL_FOREGROUND);
  lcd->drawSolidFilledRect(LINE_X1, TITLE_Y2, LINE_W, LINE_H, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(LINE_X1, LCD_H - FOOTER_Y2, LINE_W, LINE_H, BL_FOREGROUND);
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
      bootloaderDrawTitle(TITLE_X1, BOOTLOADER_TITLE);

      lcd->drawText(SYM_X, SYM_Y, LV_SYMBOL_CHARGE, BL_FOREGROUND);
      coord_t pos = lcd->drawText(LBL_X, SYM_Y, TR_BL_WRITE_FW, BL_FOREGROUND);
      pos += BOX_GAP;

#if defined(SPI_FLASH)
      lcd->drawText(SYM_X, SYM_Y + SYM_H, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
      pos = lcd->drawText(LBL_X, SYM_Y + SYM_H, TR_BL_ERASE_FLASH, BL_FOREGROUND);
      pos += BOX_GAP;

      lcd->drawText(SYM_X2, SYMY + SYM_H * 2, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(LBL_X, SYMY + SYM_H * 2, TR_BL_EXIT, BL_FOREGROUND);
#else
      lcd->drawText(SYM_X2, SYM_Y + SYM_H, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(LBL_X, SYM_Y + SYM_H, TR_BL_EXIT, BL_FOREGROUND);
#endif

      pos -= OPTBOX_X;
      lcd->drawSolidRect(OPTBOX_X, BOX_Y + (opt * SYM_H), pos, BOX_H, PAD_TINY, BL_SELECTED);

      lcd->drawBitmap(USB_ICN_X, LCD_H - USB_ICN_Y, (const BitmapBuffer*)&BMP_PLUG_USB);
      lcd->drawText(USB_TXT_X, LCD_H - USB_TXT_Y1, TR_BL_USB_PLUGIN, BL_FOREGROUND);
      lcd->drawText(USB_TXT_X, LCD_H - USB_TXT_Y2, TR_BL_USB_MASS_STORE, BL_FOREGROUND);

      bootloaderDrawFooter();
      lcd->drawText(LCD_W / 2, LCD_H - FOOTER_Y1, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    }
#if defined(SPI_FLASH)
    else if (st == ST_CLEAR_FLASH_CHECK) {

        bootloaderDrawTitle(TITLE_X1, TR_BL_ERASE_INT_FLASH);

        lcd->drawText(SYM_X, SYM_Y, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
        coord_t pos = lcd->drawText(LBL_X, SYM_Y, TR_BL_ERASE_FLASH, BL_FOREGROUND);
        pos += BOX_GAP;

        lcd->drawText(SYM_X2, SYM_Y + SYM_H, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(LBL_X, SYM_Y + SYM_H, TR_BL_EXIT, BL_FOREGROUND);

        pos -= OPTBOX_X;
        lcd->drawSolidRect(OPTBOX_X, BOX_Y + (opt * SYM_H), pos, BOX_H, PAD_TINY, BL_SELECTED);

        bootloaderDrawFooter();
        lcd->drawText(LINE_X1 * 2, 244,
                      LV_SYMBOL_SD_CARD TR_BL_ERASE_KEY, BL_FOREGROUND);
        lcd->drawText(NLSYM_X, 244,
                      LV_SYMBOL_NEW_LINE TR_BL_EXIT_KEY, BL_FOREGROUND);
    }
    else if (st == ST_CLEAR_FLASH) {
        bootloaderDrawTitle(TITLE_X1, TR_BL_ERASE_INT_FLASH);

        lcd->drawText(LCD_W / 2, SYM_Y, TR_BL_ERASE_FLASH_MSG, CENTERED | BL_FOREGROUND);
        bootloaderDrawFooter();
    }
#endif
    else if (st == ST_USB) {
        coord_t y = (LCD_H - BMP_USB_PLUGGED.height()) / 2;
        lcd->drawBitmap(USB_PLG_X, y, (const BitmapBuffer*)&BMP_USB_PLUGGED);
        lcd->drawText(USB_TXT_X, y + FOOTER_Y1, TR_BL_USB_CONNECTED, BL_FOREGROUND);
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle(TITLE_X1, LV_SYMBOL_SD_CARD " /FIRMWARE");

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {
          LcdFlags color = BL_RED;

          if (st == ST_FLASH_DONE) {
            color = BL_GREEN;
            opt = 100;  // Completed > 100%
          }

          lcd->drawRect(PROGRESS_X, (LCD_H - PROGRESS_H) / 2, PROGRESS_W, PROGRESS_H, LINE_H, SOLID, BL_SELECTED);
          lcd->drawSolidFilledRect(PROGRESS_X + PAD_SMALL, (LCD_H - PROGRESS_H) / 2 + PAD_SMALL, ((PROGRESS_W - PAD_SMALL * 2) * opt) / 100, PROGRESS_H - PAD_SMALL * 2, color);
        } else if (st == ST_DIR_CHECK) {
          if (opt == FR_NO_PATH) {
            lcd->drawText(DIRMSG_X, DIRMSG_Y, LV_SYMBOL_CLOSE TR_BL_DIR_MISSING,
                          BL_FOREGROUND);
          } else {
            lcd->drawText(DIRMSG_X, DIRMSG_Y, LV_SYMBOL_CLOSE TR_BL_DIR_EMPTY,
                          BL_FOREGROUND);
          }
        } else if (st == ST_FLASH_CHECK) {
          bootloaderDrawFilename(str, 0, true);

          if (opt == FC_ERROR) {
            lcd->drawText(DIRMSG_X, DIRMSG_Y, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                          BL_FOREGROUND);
          } else if (opt == FC_OK) {
            VersionTag tag;
            memset(&tag, 0, sizeof(tag));
            getFileFirmwareVersion(&tag);

            if (strcmp(tag.flavour, FLAVOUR)) {
              lcd->drawText(DIRMSG_X, DIRMSG_Y, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                      BL_FOREGROUND);
            } else {
              lcd->drawText(VERCHK_X, VERCHK_Y, TR_BL_FORK, RIGHT | BL_FOREGROUND);
              lcd->drawSizedText(VERCHK_X + PAD_MEDIUM, VERCHK_Y, tag.fork, 6, BL_FOREGROUND);

              lcd->drawText(VERCHK_X, VERCHK_Y + EdgeTxStyles::STD_FONT_HEIGHT, TR_BL_VERSION, RIGHT | BL_FOREGROUND);
              lcd->drawText(VERCHK_X + PAD_MEDIUM, VERCHK_Y + EdgeTxStyles::STD_FONT_HEIGHT, tag.version, BL_FOREGROUND);

              lcd->drawText(VERCHK_X, VERCHK_Y + EdgeTxStyles::STD_FONT_HEIGHT * 2, TR_BL_RADIO, RIGHT | BL_FOREGROUND);
              lcd->drawText(VERCHK_X + PAD_MEDIUM, VERCHK_Y + EdgeTxStyles::STD_FONT_HEIGHT * 2, tag.flavour, BL_FOREGROUND);

              lcd->drawText(VERCHK_ICN_X, VERCHK_Y + EdgeTxStyles::STD_FONT_HEIGHT, LV_SYMBOL_OK, BL_GREEN);
            }
          }
        }

        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawText(LINE_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_CHARGE, BL_FOREGROUND);

            if (st == ST_FILE_LIST) {
                lcd->drawText(LINE_X1 * 2, LCD_H - TITLE_Y1, TR_BL_SELECT_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(LINE_X1 * 2, LCD_H - TITLE_Y1, TR_BL_FLASH_KEY, BL_FOREGROUND);
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(LINE_X1 * 2, LCD_H - TITLE_Y1, TR_BL_WRITING_FW, BL_FOREGROUND);
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(LINE_X1 * 2, LCD_H - TITLE_Y1, TR_BL_WRITING_COMPL, BL_FOREGROUND);
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawText(NLSYM_X, LCD_H - TITLE_Y1, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
            lcd->drawText(RTNTXT_X, LCD_H - TITLE_Y1, TR_BL_EXIT_KEY, BL_FOREGROUND);
        }        
    }

    _first_screen = false;
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawText(FILENAM_X1, FILENAM_Y1 + (line * FILENAM_H), LV_SYMBOL_FILE, BL_FOREGROUND);
    lcd->drawText(FILENAM_X2, FILENAM_Y1 + (line * FILENAM_H), str, BL_FOREGROUND);

    if (selected) {
        lcd->drawSolidRect(FILESEL_X, BOX_Y + (line * FILENAM_H), FILESEL_W, BOX_H, PAD_TINY, BL_SELECTED);
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
