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
#include "bitmaps.h"

#include "translations/bl_translations.h"

#include "bootloader/boot.h"
#include "bootloader/firmware_files.h"

#include <lvgl/lvgl.h>

#define BL_GREEN      COLOR2FLAGS(RGB(73, 219, 62))
#define BL_RED        COLOR2FLAGS(RGB(229, 32, 30))
#define BL_BACKGROUND COLOR_BLACK
#define BL_FOREGROUND COLOR_WHITE
#define BL_SELECTED   COLOR2FLAGS(RGB(11, 65, 244)) // deep blue

static constexpr coord_t LINE_H = 2;

LAYOUT_VAL_SCALED(TITLE_Y1, 28)
LAYOUT_VAL_SCALED(TITLE_Y2, 56)

LAYOUT_ORIENTATION(FOOTER_X1, PAD_LARGE * 2, LCD_W / 2)
LAYOUT_ORIENTATION(FOOTER_X2, LCD_W - 1 - PAD_LARGE * 2, LCD_W / 2)
LAYOUT_ORIENTATION(FOOTER_ALIGN1, LEFT, CENTERED)
LAYOUT_ORIENTATION(FOOTER_ALIGN2, RIGHT, LEFT)
LAYOUT_VAL_SCALED(FOOTER_Y1, 30)
LAYOUT_ORIENTATION_SCALED(FOOTER_Y2, 30, 58)
LAYOUT_ORIENTATION_SCALED(FOOTER_LY, 38, 62)

constexpr coord_t PROGRESS_W = LCD_W * 3 / 4;
constexpr coord_t PROGRESS_X = (LCD_W - PROGRESS_W) / 2;
LAYOUT_VAL_SCALED(PROGRESS_H, 31)

extern BitmapBuffer * lcd;

#if defined(USB_SW_GPIO)
#include "hal/gpio.h"
#include "stm32_gpio.h"
  #define USB_SW_TO_INTERNAL_MODULE() gpio_set(USB_SW_GPIO);
  #define USB_SW_TO_MCU()             gpio_clear(USB_SW_GPIO);
  static bool rfUsbAccess = false;
#endif

void blExit(void)
{
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
}

static void bootloaderDrawTitle(const char* text)
{
  lcd->drawText(LCD_W / 2, TITLE_Y1, text, CENTERED | BL_FOREGROUND);
  lcd->drawSolidFilledRect(PAD_LARGE, TITLE_Y2, LCD_W - PAD_LARGE * 2, LINE_H, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(PAD_LARGE, LCD_H - FOOTER_LY, LCD_W - PAD_LARGE * 2, LINE_H, BL_FOREGROUND);
}

static void bootloaderDrawVerFooter()
{
  bootloaderDrawFooter();
  if (LCD_W < LCD_H)
    lcd->drawText(LCD_W / 2, LCD_H - FOOTER_Y2 + PAD_TINY, TR_BL_CURRENT_FW, CENTERED | BL_FOREGROUND);
  const char* fw_ver = getFirmwareVersion();
  if (!fw_ver) fw_ver = TR_BL_NO_VERSION;
  lcd->drawText(LCD_W / 2, LCD_H - FOOTER_Y1, fw_ver, CENTERED | BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
  lcd->clear(BL_BACKGROUND);
}

bool bootloaderRadioMenu(uint32_t menuItem, event_t event)
{
#if defined(USB_SW_GPIO)
  #define RADIO_MENU_LEN 2

  static int pos = 0;

  if (IS_NEXT_EVENT(event)) {
    if (pos < RADIO_MENU_LEN-1)
      pos++;
  } else if (IS_PREVIOUS_EVENT(event)) {
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
        rfUsbAccess = false;
        INTERNAL_MODULE_OFF();
        USB_SW_TO_MCU();
        pos = 0;
        return true;
    }
  } else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    rfUsbAccess = false;
    INTERNAL_MODULE_OFF();
    USB_SW_TO_MCU();
    pos = 0;
    return true;
  }
  bootloaderDrawScreen(ST_RADIO_MENU, pos, nullptr);
  return false;
#else
  return true;
#endif
}

#if defined(FIRMWARE_FORMAT_UF2)

void bootloaderInitScreen()
{
  lcdInitDisplayDriver();
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

void bootloaderDrawDFUScreen()
{
  lcdInitDirectDrawing();
  bootloaderDrawBackground();
  lcd->drawText(LCD_W / 2, LCD_H / 2, TR_BL_DFU_MODE, CENTERED | BL_FOREGROUND);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
  lcdInitDirectDrawing();
  bootloaderDrawBackground();

  bootloaderDrawTitle(BOOTLOADER_TITLE);

  if (st == ST_START || st == ST_USB) {

    const char* msg = (st == ST_START) ? TR_BL_PLUG_USB : TR_BL_COPY_UF2;

    lcd->drawText(LCD_W / 2, LCD_H / 2 - TITLE_Y1, msg, CENTERED | BL_FOREGROUND);
    lcd->drawText(LCD_W / 2, LCD_H / 2 + TITLE_Y1, LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, CENTERED | BL_FOREGROUND);

    bootloaderDrawVerFooter();

  } else if (st == ST_FLASHING || st == ST_FLASH_DONE) {

    const coord_t pb_y = (LCD_H - PROGRESS_H) / 2;

    const char* msg = (st == ST_FLASH_DONE) ? TR_BL_WRITING_COMPL : TR_BL_WRITING_FW;
    lcd->drawText(PROGRESS_X, pb_y - TITLE_Y1, msg, BL_FOREGROUND);
    lcd->drawRect(PROGRESS_X, pb_y, PROGRESS_W, PROGRESS_H, LINE_H, SOLID, BL_SELECTED);

    LcdFlags color = (st == ST_FLASH_DONE) ? BL_GREEN : BL_RED;
    lcd->drawSolidFilledRect(PROGRESS_X + PAD_SMALL, pb_y + PAD_SMALL, ((PROGRESS_W - PAD_SMALL * 2) * opt) / 100, PROGRESS_H - PAD_SMALL * 2, color);
  }
}

#else

LAYOUT_ORIENTATION_SCALED(SYM_X, 102, 38)
LAYOUT_ORIENTATION_SCALED(LBL_X, 124, 60)
LAYOUT_VAL_SCALED(SYM_Y, 75)
LAYOUT_VAL_SCALED(SYM_H, 35)

LAYOUT_VAL_SCALED(BOX_GAP, 8)
LAYOUT_VAL_SCALED(BOX_Y, 71)
LAYOUT_VAL_SCALED(BOX_H, 27)

LAYOUT_ORIENTATION_SCALED(OPTBOX_X, 92, 28)

LAYOUT_ORIENTATION_SCALED(USB_ICN_X, 60, 105)
LAYOUT_ORIENTATION_SCALED(USB_ICN_Y, 106, 185)
LAYOUT_ORIENTATION_SCALED(USB_TXT_X, 195, 160)
LAYOUT_ORIENTATION_SCALED(USB_TXT_Y1, 97, 125)
LAYOUT_ORIENTATION_SCALED(USB_TXT_Y2, 72, 100)
LAYOUT_ORIENTATION(USB_TXT_ALIGN, 0, CENTERED)

LAYOUT_ORIENTATION_SCALED(USB_PLG_X, 136, 134)
LAYOUT_ORIENTATION_SCALED(USB_PLG_TXT_YO, 30, 58)

LAYOUT_ORIENTATION_SCALED(FILENAM_X1, 94, 28)
LAYOUT_ORIENTATION_SCALED(FILENAM_X2, 124, 58)
LAYOUT_VAL_SCALED(FILENAM_Y1, 75)
LAYOUT_VAL_SCALED(FILENAM_H, 25)
LAYOUT_ORIENTATION_SCALED(FILESEL_X, 119, 53)
LAYOUT_ORIENTATION_SCALED(FILESEL_W, 278, 240)

LAYOUT_ORIENTATION_SCALED(VERCHK_X, 168, 112)
LAYOUT_ORIENTATION_SCALED(VERCHK_Y, 138, 240)
LAYOUT_ORIENTATION_SCALED(VERCHK_ICN_X, 78, 22)

const uint8_t __bmp_plug_usb[] {
  #include "bmp_bootloader_plug_usb.lbm"
};
LZ4BitmapBuffer BMP_PLUG_USB(BMP_ARGB4444);

const uint8_t __bmp_usb_plugged[] {
  #include "bmp_bootloader_usb_plugged.lbm"
};
LZ4BitmapBuffer BMP_USB_PLUGGED(BMP_ARGB4444);

void bootloaderInitScreen()
{
  BMP_PLUG_USB.load((LZ4Bitmap*)__bmp_plug_usb);
  BMP_USB_PLUGGED.load((LZ4Bitmap*)__bmp_usb_plugged);

  lcdInitDisplayDriver();
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

void bootloaderDrawItem(const char* symbol, const char* text, coord_t& y, coord_t& lastX, coord_t symXO = 0)
{
  lcd->drawText(SYM_X - symXO, y, symbol, BL_FOREGROUND);
  coord_t nx = lcd->drawText(LBL_X, y, text, BL_FOREGROUND) + BOX_GAP;
  if (nx > lastX) lastX = nx;
  y += SYM_H;
}

void bootloaderDrawSelected(coord_t boxX2, int opt)
{
  boxX2 -= OPTBOX_X;
  lcd->drawSolidRect(OPTBOX_X, BOX_Y + (opt * SYM_H), boxX2, BOX_H, PAD_TINY, BL_SELECTED);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
  lcdInitDirectDrawing();
  bootloaderDrawBackground();

  coord_t y = SYM_Y, boxX2 = 0;

  if (st == ST_START) {
    bootloaderDrawTitle(BOOTLOADER_TITLE);

    bootloaderDrawItem(LV_SYMBOL_CHARGE, TR_BL_WRITE_FW, y, boxX2);

#if defined(SPI_FLASH)
    bootloaderDrawItem(LV_SYMBOL_SD_CARD, TR_BL_ERASE_FLASH, y, boxX2);
#endif

#if defined(USB_SW_GPIO)
#if defined(RADIO_NV14_FAMILY)
    if (hardwareOptions.pcbrev == PCBREV_EL18)
#endif
      bootloaderDrawItem(LV_SYMBOL_WIFI, TR_BL_RF_USB_ACCESS, y, boxX2);
#endif

    bootloaderDrawItem(LV_SYMBOL_NEW_LINE, TR_BL_EXIT, y, boxX2, PAD_TINY);

    bootloaderDrawSelected(boxX2, opt);

    lcd->drawBitmap(USB_ICN_X, LCD_H - USB_ICN_Y, (const BitmapBuffer*)&BMP_PLUG_USB);
    lcd->drawText(USB_TXT_X, LCD_H - USB_TXT_Y1, TR_BL_USB_PLUGIN, USB_TXT_ALIGN | BL_FOREGROUND);
    lcd->drawText(USB_TXT_X, LCD_H - USB_TXT_Y2, TR_BL_USB_MASS_STORE, USB_TXT_ALIGN | BL_FOREGROUND);

    bootloaderDrawVerFooter();
  }
#if defined(SPI_FLASH)
  else if (st == ST_CLEAR_FLASH_CHECK) {
    bootloaderDrawTitle(TR_BL_ERASE_INT_FLASH);

    bootloaderDrawItem(LV_SYMBOL_SD_CARD, TR_BL_ERASE_FLASH, y, boxX2);

    bootloaderDrawSelected(boxX2, opt);

    bootloaderDrawFooter();
    int pos = lcd->drawText(FOOTER_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_SD_CARD " " TR_BL_ERASE_KEY, FOOTER_ALIGN1 | BL_FOREGROUND);
#if LANDSCAPE
    pos = 0;
#else    
    if (pos != 0) {
      pos = (pos - FOOTER_X1) / 2;
    }
#endif
    lcd->drawText(FOOTER_X2 - pos, LCD_H - FOOTER_Y2, LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, FOOTER_ALIGN2 | BL_FOREGROUND);
  }
  else if (st == ST_CLEAR_FLASH) {
    bootloaderDrawTitle(TR_BL_ERASE_INT_FLASH);

    lcd->drawText(LCD_W / 2, SYM_Y, TR_BL_ERASE_FLASH_MSG, CENTERED | BL_FOREGROUND);
    bootloaderDrawFooter();
  }
#endif
#if defined(USB_SW_GPIO)
  else if (st == ST_RADIO_MENU) {
    bootloaderDrawTitle(TR_BL_RF_USB_ACCESS);

    bootloaderDrawItem(LV_SYMBOL_USB, rfUsbAccess ? TR_BL_DISABLE : TR_BL_ENABLE, y, boxX2);

    bootloaderDrawSelected(boxX2, opt);

    bootloaderDrawFooter();
    int pos = lcd->drawText(FOOTER_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_USB " " TR_BL_TOGGLE_KEY, FOOTER_ALIGN1 | BL_FOREGROUND);
#if LANDSCAPE
    pos = 0;
#else    
    if (pos != 0) {
      pos = (pos - FOOTER_X1) / 2;
    }
#endif
    lcd->drawText(FOOTER_X2 - pos, LCD_H - FOOTER_Y2, LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, FOOTER_ALIGN2 | BL_FOREGROUND);
  }
#endif
  else if (st == ST_USB) {
    y = (LCD_H - BMP_USB_PLUGGED.height()) / 2;
    lcd->drawBitmap(USB_PLG_X, y, (const BitmapBuffer*)&BMP_USB_PLUGGED);
    lcd->drawText(USB_TXT_X, y + USB_PLG_TXT_YO, TR_BL_USB_CONNECTED, USB_TXT_ALIGN | BL_FOREGROUND);
  }
  else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
           st == ST_FLASHING || st == ST_FLASH_DONE) {

    bootloaderDrawTitle(LV_SYMBOL_SD_CARD " /FIRMWARE");

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
        lcd->drawText(LCD_W / 2, LCD_H / 2, LV_SYMBOL_CLOSE " " TR_BL_DIR_MISSING, CENTERED | BL_FOREGROUND);
      } else {
        lcd->drawText(LCD_W / 2, LCD_H / 2, LV_SYMBOL_CLOSE " " TR_BL_DIR_EMPTY,  CENTERED | BL_FOREGROUND);
      }
    } else if (st == ST_FLASH_CHECK) {
      bootloaderDrawFilename(str, 0, true);

      if (opt == FC_ERROR) {
        lcd->drawText(LCD_W / 2, LCD_H / 2, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE,  CENTERED | BL_FOREGROUND);
      } else if (opt == FC_OK) {
        VersionTag tag;
        memset(&tag, 0, sizeof(tag));
        getFileFirmwareVersion(&tag);

        if (strcmp(tag.flavour, FLAVOUR)) {
          lcd->drawText(LCD_W / 2, LCD_H / 2, LV_SYMBOL_CLOSE " " TR_BL_INVALID_FIRMWARE, CENTERED | BL_FOREGROUND);
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

    int pos = 0;
    int align = FOOTER_ALIGN2;
    if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {
      if (st == ST_FILE_LIST) {
        pos = lcd->drawText(FOOTER_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_CHARGE " " TR_BL_SELECT_KEY, FOOTER_ALIGN1 | BL_FOREGROUND);
      }
      else if (st == ST_FLASH_CHECK && opt == FC_OK) {
        pos = lcd->drawText(FOOTER_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_CHARGE " " TR_BL_FLASH_KEY, FOOTER_ALIGN1 | BL_FOREGROUND);
      }
      else if (st == ST_FLASHING) {
        pos = lcd->drawText(LCD_W / 2, LCD_H - FOOTER_Y1, LV_SYMBOL_CHARGE " " TR_BL_WRITING_FW, CENTERED | BL_FOREGROUND);
      }
      else if (st == ST_FLASH_DONE) {
        pos = lcd->drawText(FOOTER_X1, LCD_H - FOOTER_Y1, LV_SYMBOL_CHARGE " " TR_BL_WRITING_COMPL, FOOTER_ALIGN1 | BL_FOREGROUND);
      }
    }
#if LANDSCAPE
    pos = 0;
#else
    if (pos != 0) {
      pos = (pos - FOOTER_X1) / 2;
    } else {
      align = FOOTER_ALIGN1;
    }
#endif
    if (st != ST_FLASHING) {
      lcd->drawText(FOOTER_X2 - pos, LCD_H - FOOTER_Y2, LV_SYMBOL_NEW_LINE " " TR_BL_EXIT_KEY, align | BL_FOREGROUND);
    }        
  }
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
  uint8_t n = 0;
#if defined(USB_SW_GPIO)
#if defined(RADIO_NV14_FAMILY)
  if(hardwareOptions.pcbrev == PCBREV_EL18)
#endif
    n = 1;
#endif
  return baseCount + n;
}

#endif
