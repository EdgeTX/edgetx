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

#include "../../common/arm/stm32/bootloader/boot.h"
#include "../../common/arm/stm32/bootloader/bin_files.h"

#include <lvgl/lvgl.h>

#define SELECTED_COLOR (INVERS | COLOR_THEME_SECONDARY1)
#define DEFAULT_PADDING 28
#define DOUBLE_PADDING  56
#define MESSAGE_TOP     (LCD_H - (2*DOUBLE_PADDING))

const uint8_t __bmp_plug_usb_rle[] {
#include "bmp_plug_usb.lbm"
};
RLEBitmap BMP_PLUG_USB(BMP_ARGB4444, __bmp_plug_usb_rle);

const uint8_t __bmp_usb_plugged_rle[] {
#include "bmp_usb_plugged.lbm"
};
RLEBitmap BMP_USB_PLUGGED(BMP_ARGB4444, __bmp_usb_plugged_rle);

const uint8_t __bmp_background_rle[] {
#include "bmp_background.lbm"
};
RLEBitmap BMP_BACKGROUND(BMP_ARGB4444, __bmp_background_rle);

const uint8_t LBM_FLASH[] = {
#include "icon_flash.lbm"
};

const uint8_t LBM_EXIT[] = {
#include "icon_exit.lbm"
};

const uint8_t LBM_SD[] = {
#include "icon_sd.lbm"
};

const uint8_t LBM_FILE[] = {
#include "icon_file.lbm"
};

const uint8_t LBM_ERROR[] = {
#include "icon_error.lbm"
};

const uint8_t LBM_OK[] = {
#include "icon_ok.lbm"
};

#define BL_GREEN      COLOR2FLAGS(RGB(73, 219, 62))
#define BL_RED        COLOR2FLAGS(RGB(229, 32, 30))
#define BL_BACKGROUND COLOR2FLAGS(BLACK)
#define BL_FOREGROUND COLOR2FLAGS(WHITE)
#define BL_SELECTED   COLOR2FLAGS(RGB(11, 65, 244)) // deep blue

extern BitmapBuffer * lcdFront;
extern BitmapBuffer * lcd;

static lv_disp_t* disp = nullptr;
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_buf;

void newLcdRefresh(uint16_t* buffer, const rect_t& copy_area);
static void flushLcd(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  lv_area_t refr_area;
  lv_area_copy(&refr_area, area);

#if defined(LCD_VERTICAL_INVERT)
  lv_coord_t tmp_coord;
  tmp_coord = refr_area.y2;
  refr_area.y2 = LCD_H - refr_area.y1 - 1;
  refr_area.y1 = LCD_H - tmp_coord - 1;
  tmp_coord = refr_area.x2;
  refr_area.x2 = LCD_W - refr_area.x1 - 1;
  refr_area.x1 = LCD_W - tmp_coord - 1;
#endif

  if (refr_area.x1 != 0 || refr_area.x2 != 479 || refr_area.y1 != 0 ||
      refr_area.y2 != 271) {
    TRACE("partial refresh @ 0x%p {%d,%d,%d,%d}", color_p, refr_area.x1,
          refr_area.y1, refr_area.x2, refr_area.y2);
  } else {
    TRACE("full refresh @ 0x%p", color_p);
  }

  rect_t copy_area = {refr_area.x1, refr_area.y1,
                      refr_area.x2 - refr_area.x1 + 1,
                      refr_area.y2 - refr_area.y1 + 1};

  newLcdRefresh((uint16_t*)color_p, copy_area);
  lv_disp_flush_ready(disp_drv);
}

static void init_disp_drv()
{
  lv_disp_draw_buf_init(&disp_buf, lcdFront->getData(), lcd->getData(), LCD_W*LCD_H);
  lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;           /*Set a flush callback to draw to the display*/
  disp_drv.hor_res = LCD_W;               /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H;               /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;
  disp_drv.direct_mode = 0;
#if defined (LCD_VERTICAL_INVERT)
  disp_drv.rotated = LV_DISP_ROT_180;
#endif
  disp_drv.sw_rotate = 1;

  // Register the driver and save the created display objects
  disp = lv_disp_drv_register(&disp_drv);
}

void bootloaderInitScreen()
{
  backlightEnable(BACKLIGHT_LEVEL_MAX);
  lv_init();
  init_disp_drv();
}

static void bootloaderDrawTitle(const char* text)
{
    lcd->drawText(LCD_W/2, DEFAULT_PADDING, text, CENTERED | BL_FOREGROUND);
    lcd->drawSolidFilledRect(DEFAULT_PADDING, DOUBLE_PADDING, LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
    lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DOUBLE_PADDING + 4), LCD_W - DOUBLE_PADDING, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
  // we have plenty of memory, let's cache that background
  static BitmapBuffer* _background = nullptr;

  if (!_background) {
    _background = new BitmapBuffer(BMP_RGB565, LCD_W, LCD_H);
    
    for (int i=0; i<LCD_W; i += BMP_BACKGROUND.width()) {
      for (int j=0; j<LCD_H; j += BMP_BACKGROUND.height()) {
        _background->drawBitmap(i, j, &BMP_BACKGROUND);
      }
    }
    _background->drawFilledRect(0, 0, LCD_W, LCD_H, SOLID,
                                COLOR2FLAGS(BLACK), OPACITY(4));
  }

  if (_background) {
    lcd->drawBitmap(0, 0, _background);
  }
  else {
    lcd->clear(BL_BACKGROUND);
  }
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
    // clear screen
    bootloaderDrawBackground();

    int center = LCD_W/2;
    if (st == ST_START) {

        bootloaderDrawTitle(BOOTLOADER_TITLE);
        
        lcd->drawBitmapPattern(50, 72, LBM_FLASH, BL_FOREGROUND);
        lcd->drawText(84,  75, "Write Firmware", BL_FOREGROUND);

        lcd->drawBitmapPattern(50, 107, LBM_EXIT, BL_FOREGROUND);
        lcd->drawText(84, 110, "Exit", BL_FOREGROUND);

        lcd->drawSolidRect(79, (opt == 0) ? 72 : 107, LCD_W - 79 - 28, 26, 2, BL_SELECTED);
        
        lcd->drawBitmap(center - 55, 165, &BMP_PLUG_USB);
        lcd->drawText(center, 250, "Or plug in a USB cable", CENTERED | BL_FOREGROUND);
        lcd->drawText(center, 275, "for mass storage", CENTERED | BL_FOREGROUND);

        bootloaderDrawFooter();
        lcd->drawText(center, LCD_H - DOUBLE_PADDING, "Current Firmware:", CENTERED | BL_FOREGROUND);        
        lcd->drawText(center, LCD_H - DEFAULT_PADDING, getFirmwareVersion(nullptr), CENTERED | BL_FOREGROUND);
    }
    else if (st == ST_USB) {
        lcd->drawBitmap(center - 26, 98, &BMP_USB_PLUGGED);
        lcd->drawText(center, 168, "USB Connected", CENTERED | BL_FOREGROUND);
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle("SD>FIRMWARE");
        lcd->drawBitmapPattern(DEFAULT_PADDING, 16, LBM_SD, BL_FOREGROUND);

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {

            LcdFlags color = BL_RED; // red

            if (st == ST_FLASH_DONE) {
                color = BL_GREEN/* green */;
                opt   = 100; // Completed > 100%
            }

            lcd->drawRect(DEFAULT_PADDING, 120, LCD_W - DOUBLE_PADDING, 31, 2, SOLID, BL_SELECTED);
            lcd->drawSolidFilledRect(DEFAULT_PADDING+4, 124, ((LCD_W - DOUBLE_PADDING - 8) * opt) / 100, 23, color);
        }
        else if (st == ST_DIR_CHECK) {

            if (opt == FR_NO_PATH) {
                lcd->drawText(20, MESSAGE_TOP, "Directory is missing", BL_FOREGROUND);
            }
            else {
                lcd->drawText(20, MESSAGE_TOP, "Directory is empty", BL_FOREGROUND);
            }

            lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_ERROR, BL_RED);
        }
        else if (st == ST_FLASH_CHECK) {

            bootloaderDrawFilename(str, 0, true);

            if (opt == FC_ERROR) {
                lcd->drawText(20, MESSAGE_TOP, STR_INVALID_FIRMWARE, BL_FOREGROUND);
                lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_ERROR, BL_RED);
            }
            else if (opt == FC_OK) {
                VersionTag tag;
                memset(&tag, 0, sizeof(tag));
                extractFirmwareVersion(&tag);

                lcd->drawText(LCD_W/4 + DEFAULT_PADDING, MESSAGE_TOP - DEFAULT_PADDING, "Fork:", RIGHT | BL_FOREGROUND);
                lcd->drawSizedText(LCD_W/4 + 6 + DEFAULT_PADDING, MESSAGE_TOP - DEFAULT_PADDING, tag.fork, 6, BL_FOREGROUND);

                lcd->drawText(LCD_W/4 + DEFAULT_PADDING, MESSAGE_TOP, "Version:", RIGHT | BL_FOREGROUND);
                lcd->drawText(LCD_W/4 + 6 + DEFAULT_PADDING, MESSAGE_TOP, tag.version, BL_FOREGROUND);
                
                lcd->drawText(LCD_W/4 + DEFAULT_PADDING, MESSAGE_TOP + DEFAULT_PADDING, "Radio:", RIGHT | BL_FOREGROUND);
                lcd->drawText(LCD_W/4 + 6 + DEFAULT_PADDING, MESSAGE_TOP + DEFAULT_PADDING, tag.flavour, BL_FOREGROUND);
                
                lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_OK, BL_GREEN);
            }
        }
        
        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawBitmapPattern(DEFAULT_PADDING, LCD_H - DOUBLE_PADDING - 2, LBM_FLASH, BL_FOREGROUND);

            if (st == ST_FILE_LIST) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "[R TRIM] to select file", BL_FOREGROUND);
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Hold [R TRIM] long to flash", BL_FOREGROUND);
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Writing Firmware ...", BL_FOREGROUND);
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Writing Completed", BL_FOREGROUND);
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawBitmapPattern(DEFAULT_PADDING, LCD_H - DEFAULT_PADDING - 2, LBM_EXIT, BL_FOREGROUND);
            lcd->drawText(DOUBLE_PADDING, LCD_H - DEFAULT_PADDING, "[L TRIM] to exit", BL_FOREGROUND);
        }        
    }
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawBitmapPattern(DEFAULT_PADDING, 76 + (line * 25), LBM_FILE, BL_FOREGROUND);
    lcd->drawText(DEFAULT_PADDING + 30, 75 + (line * 25), str, BL_FOREGROUND);

    if (selected) {
        lcd->drawSolidRect(DEFAULT_PADDING + 25, 72 + (line * 25), LCD_W - (DEFAULT_PADDING + 25) - 28, 26, 2, BL_SELECTED);
    }
}
