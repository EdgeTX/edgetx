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

#include "hal/gpio.h"
#include "stm32_gpio.h"

#include "board.h"
#include "fw_version.h"
#include "lcd.h"

#include "translations.h"

#include "targets/common/arm/stm32/bootloader/boot.h"
#include "targets/common/arm/stm32/bootloader/bin_files.h"

#include <lvgl/lvgl.h>

#define RADIO_MENU_LEN 2

#define SELECTED_COLOR (INVERS | COLOR_THEME_SECONDARY1)
#define DEFAULT_PADDING 28
#define DOUBLE_PADDING  56
#define MESSAGE_TOP     (LCD_H - (2*DOUBLE_PADDING))

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

#if defined(USB_SW_GPIO)
  #define USB_SW_TO_INTERNAL_MODULE() gpio_set(USB_SW_GPIO);
  #define USB_SW_TO_MCU()             gpio_clear(USB_SW_GPIO);
  static bool rfUsbAccess = false;
#endif

void bootloaderInitScreen()
{
  lcdInitDisplayDriver();
#if defined(USE_HATS_AS_KEYS)
  setHatsAsKeys(true);
#endif
}

static void bootloaderDrawTitle(const char* text)
{
    lcd->drawText(LCD_W/2, DEFAULT_PADDING, text, CENTERED | BL_FOREGROUND);
    lcd->drawSolidFilledRect(DEFAULT_PADDING, DOUBLE_PADDING, LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  if (LCD_W > LCD_H)
    lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DEFAULT_PADDING + 10), LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
  else
    lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DOUBLE_PADDING + 4), LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
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

    int center = LCD_W/2;
    int menuItemX = center - 120;
    if (st == ST_START) {
        int y = 75;

        bootloaderDrawTitle(BOOTLOADER_TITLE);
        
        lcd->drawText(menuItemX + 2, y, LV_SYMBOL_CHARGE, BL_FOREGROUND);
        coord_t pos = lcd->drawText(menuItemX + 24, y, TR_BL_WRITE_FW, BL_FOREGROUND);
        pos += 8;
        coord_t npos;
        y += 35;

#if defined(SPI_FLASH)
        lcd->drawText(menuItemX + 2, y, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
        npos = lcd->drawText(menuItemX + 24, y, TR_BL_ERASE_FLASH, BL_FOREGROUND);
        npos += 8;
        if (npos > pos)
          pos = npos;
        y += 35;
#endif
#if defined(USB_SW_GPIO)
        lcd->drawText(menuItemX, y, LV_SYMBOL_WIFI, BL_FOREGROUND);
        npos = lcd->drawText(menuItemX + 24, y, TR_BL_RF_USB_ACCESS, BL_FOREGROUND);        
        npos += 8;
        if (npos > pos)
          pos = npos;
        y += 35;
#endif
        lcd->drawText(menuItemX, y, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(menuItemX + 24, y, TR_BL_EXIT, BL_FOREGROUND);

        pos -= menuItemX - 8;
        lcd->drawSolidRect(menuItemX - 8, 72 + (opt * 35), pos, 26, 2, BL_SELECTED);

        if (LCD_W > LCD_H) {
          lcd->drawBitmap(menuItemX - 40, LCD_H - 106, (const BitmapBuffer*)&BMP_PLUG_USB);
          lcd->drawText(menuItemX + 95, LCD_H - 97, TR_BL_USB_PLUGIN, BL_FOREGROUND);
          lcd->drawText(menuItemX + 95, LCD_H - 72, TR_BL_USB_MASS_STORE, BL_FOREGROUND);
        } else {
          lcd->drawBitmap(center - 55, LCD_H - DEFAULT_PADDING - 157, (const BitmapBuffer*)&BMP_PLUG_USB);
          lcd->drawText(center, LCD_H - DEFAULT_PADDING - 97, TR_BL_USB_PLUGIN, CENTERED | BL_FOREGROUND);
          lcd->drawText(center, LCD_H - DEFAULT_PADDING - 72, TR_BL_USB_MASS_STORE, CENTERED | BL_FOREGROUND);
        }

        bootloaderDrawFooter();
        if (LCD_W < LCD_H)
          lcd->drawText(center, LCD_H - DOUBLE_PADDING, TR_BL_CURRENT_FW, CENTERED | BL_FOREGROUND);
        lcd->drawText(center, LCD_H - DEFAULT_PADDING, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    }
#if defined(SPI_FLASH)
    else if (st == ST_CLEAR_FLASH_CHECK) {

        bootloaderDrawTitle(TR_BL_ERASE_INT_FLASH);

        lcd->drawText(menuItemX + 2, 75, LV_SYMBOL_SD_CARD, BL_FOREGROUND);
        coord_t pos = lcd->drawText(menuItemX + 24, 75, TR_BL_ERASE_FLASH, BL_FOREGROUND);
        pos += 8;

        lcd->drawText(menuItemX, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
        lcd->drawText(menuItemX + 24, 110, TR_BL_EXIT, BL_FOREGROUND);

        pos -= menuItemX - 8;
        lcd->drawSolidRect(menuItemX - 8, 72 + (opt * 35), pos, 26, 2, BL_SELECTED);

        bootloaderDrawFooter();
        if (LCD_W > LCD_H) {
          lcd->drawText(DEFAULT_PADDING, LCD_H - DEFAULT_PADDING,
                        LV_SYMBOL_SD_CARD " " TR_BL_ERASE_KEY, BL_FOREGROUND);
          lcd->drawText(LCD_W - 175, LCD_H - DEFAULT_PADDING,
                        LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, BL_FOREGROUND);
        } else {
          lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING,
                        LV_SYMBOL_SD_CARD " " TR_BL_ERASE_KEY, BL_FOREGROUND);
          lcd->drawText(DOUBLE_PADDING, LCD_H - DEFAULT_PADDING,
                        LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, BL_FOREGROUND);
        }
    }
    else if (st == ST_CLEAR_FLASH) {
        bootloaderDrawTitle(TR_BL_ERASE_INT_FLASH);

        lcd->drawText(center, 75, TR_BL_ERASE_FLASH_MSG, CENTERED | BL_FOREGROUND);
        bootloaderDrawFooter();
    }
#endif
#if defined(USB_SW_GPIO)
    else if (st == ST_RADIO_MENU) {
      bootloaderDrawTitle(TR_BL_RF_USB_ACCESS);

      lcd->drawText(menuItemX + 2, 75, LV_SYMBOL_USB, BL_FOREGROUND);
      coord_t pos = lcd->drawText(menuItemX + 24, 75, rfUsbAccess ? TR_BL_DISABLE : TR_BL_ENABLE, BL_FOREGROUND);
      pos += 8;

      lcd->drawText(menuItemX + 2, 110, LV_SYMBOL_NEW_LINE, BL_FOREGROUND);
      lcd->drawText(menuItemX + 24, 110, TR_BL_EXIT, BL_FOREGROUND);

      pos -= menuItemX - 8;
      lcd->drawSolidRect(menuItemX - 8, 72 + (opt * 35), pos, 26, 2, BL_SELECTED);
    }
#endif
    else if (st == ST_USB) {
      lcd->drawBitmap(center - 26, 98, (const BitmapBuffer*)&BMP_USB_PLUGGED);
      lcd->drawText(center, 168, TR_BL_USB_CONNECTED, CENTERED | BL_FOREGROUND);
    } else if (st == ST_FILE_LIST || st == ST_DIR_CHECK ||
               st == ST_FLASH_CHECK || st == ST_FLASHING ||
               st == ST_FLASH_DONE) {

      bootloaderDrawTitle(LV_SYMBOL_SD_CARD " /FIRMWARE");

      if (st == ST_FLASHING || st == ST_FLASH_DONE) {
        LcdFlags color = BL_RED;  // red

        if (st == ST_FLASH_DONE) {
          color = BL_GREEN /* green */;
          opt = 100;  // Completed > 100%
        }

        lcd->drawRect(DEFAULT_PADDING, 120, LCD_W - DOUBLE_PADDING, 31, 2,
                      SOLID, BL_SELECTED);
        lcd->drawSolidFilledRect(DEFAULT_PADDING + 4, 124,
                                 ((LCD_W - DOUBLE_PADDING - 8) * opt) / 100, 23,
                                 color);
      } else if (st == ST_DIR_CHECK) {
        if (opt == FR_NO_PATH) {
          lcd->drawText(20, MESSAGE_TOP,
                        LV_SYMBOL_CLOSE TR_BL_DIR_MISSING, BL_FOREGROUND);
        } else {
          lcd->drawText(20, MESSAGE_TOP, LV_SYMBOL_CLOSE TR_BL_DIR_EMPTY,
                        BL_FOREGROUND);
        }
      } else if (st == ST_FLASH_CHECK) {
        bootloaderDrawFilename(str, 0, true);

        if (opt == FC_ERROR) {
          lcd->drawText(20, MESSAGE_TOP,
                        LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                        BL_FOREGROUND);
        } else if (opt == FC_OK) {
          VersionTag tag;
          memset(&tag, 0, sizeof(tag));
          extractFirmwareVersion(&tag);
          if (strcmp(tag.flavour, FLAVOUR)) {
            lcd->drawText(20, MESSAGE_TOP, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,
                    BL_FOREGROUND);
          } else {
            lcd->drawText(LCD_W / 4 + DEFAULT_PADDING,
                          MESSAGE_TOP - DEFAULT_PADDING,
                          TR_BL_FORK, RIGHT | BL_FOREGROUND);
            lcd->drawSizedText(LCD_W / 4 + 6 + DEFAULT_PADDING,
                               MESSAGE_TOP - DEFAULT_PADDING, tag.fork, 6,
                               BL_FOREGROUND);

            lcd->drawText(LCD_W / 4 + DEFAULT_PADDING, MESSAGE_TOP,
                          TR_BL_VERSION, RIGHT | BL_FOREGROUND);
            lcd->drawText(LCD_W / 4 + 6 + DEFAULT_PADDING, MESSAGE_TOP,
                          tag.version, BL_FOREGROUND);

            lcd->drawText(LCD_W / 4 + DEFAULT_PADDING,
                          MESSAGE_TOP + DEFAULT_PADDING,
                          TR_BL_RADIO, RIGHT | BL_FOREGROUND);
            lcd->drawText(LCD_W / 4 + 6 + DEFAULT_PADDING,
                          MESSAGE_TOP + DEFAULT_PADDING, tag.flavour,
                          BL_FOREGROUND);

            lcd->drawText(DOUBLE_PADDING, MESSAGE_TOP, LV_SYMBOL_OK, BL_GREEN);
          }
        }
      }

      bootloaderDrawFooter();

      if (st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

        int padding = DOUBLE_PADDING;
        if (LCD_W > LCD_H)
          padding = DEFAULT_PADDING;
        if (st == ST_FILE_LIST) {
          lcd->drawText(padding, LCD_H - padding,
                        LV_SYMBOL_CHARGE " " TR_BL_SELECT_KEY, BL_FOREGROUND);
        } else if (st == ST_FLASH_CHECK && opt == FC_OK) {
          lcd->drawText(padding, LCD_H - padding,
                        LV_SYMBOL_CHARGE " " TR_BL_FLASH_KEY, BL_FOREGROUND);
        } else if (st == ST_FLASHING) {
          lcd->drawText(padding, LCD_H - padding,
                        LV_SYMBOL_CHARGE " " TR_BL_WRITING_FW, BL_FOREGROUND);
        } else if (st == ST_FLASH_DONE) {
          lcd->drawText(padding, LCD_H - padding,
                        LV_SYMBOL_CHARGE " " TR_BL_WRITING_COMPL, BL_FOREGROUND);
        }
      }

      if (st != ST_FLASHING) {
        if (LCD_W > LCD_H) {
          lcd->drawText(LCD_W - 175, LCD_H - DEFAULT_PADDING,
                        LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, BL_FOREGROUND);
        } else {
          lcd->drawText(DOUBLE_PADDING, LCD_H - DEFAULT_PADDING,
                        LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, BL_FOREGROUND);
        }
      }
    }
    _first_screen = false;
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
uint32_t bootloaderGetMenuItemCount(int baseCount)
{
#if defined(USB_SW_GPIO)
    return baseCount+1;
#else
    return baseCount;
#endif
}

bool bootloaderRadioMenu(uint32_t menuItem, event_t event)
{
#if defined(USB_SW_GPIO)
    static int pos = 0;

    if (event == EVT_KEY_FIRST(KEY_DOWN)) {
        if (pos < RADIO_MENU_LEN-1)
            pos++;
    } else if (event == EVT_KEY_FIRST(KEY_UP)) {
        if (pos > 0)
            pos--;
    } else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
        switch (pos) {
        case 0:
            if (rfUsbAccess)
            {
                rfUsbAccess = false;
                INTERNAL_MODULE_OFF();
                USB_SW_TO_MCU();
            } else {
                rfUsbAccess = true;
                INTERNAL_MODULE_ON();
                USB_SW_TO_INTERNAL_MODULE();
            }
            break;
        case 1: // fall through
        default:
            USB_SW_TO_MCU();
            pos = 0;
            return true;
        }
    }
    bootloaderDrawScreen(ST_RADIO_MENU, pos, nullptr);
    return false;
#else
    return true;
#endif
}

void blExit(void)
{
#if defined(USB_SW_GPIO)
  USB_SW_TO_MCU();
#endif
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
}
