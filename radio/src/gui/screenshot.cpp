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

#include "edgetx.h"

constexpr uint16_t BMP_HEADERSIZE = 0x76;

#if defined(COLORLCD)
constexpr uint8_t BMP_BITS_PER_PIXEL = 32;
#else
constexpr uint8_t BMP_BITS_PER_PIXEL = 4;
#endif

constexpr uint32_t BMP_FILESIZE = uint32_t(BMP_HEADERSIZE + (LCD_W * LCD_H) * BMP_BITS_PER_PIXEL / 8);

const uint8_t BMP_HEADER[] = {
  'B',  'M',
  /* file size */ BMP_FILESIZE & 0xFF, (BMP_FILESIZE >> 8) & 0xFF, (BMP_FILESIZE >> 16) & 0xFF, (BMP_FILESIZE >> 24) & 0xFF,
  0x00, 0x00, 0x00, 0x00,
  /* header size */ BMP_HEADERSIZE, 0x00, 0x00, 0x00,
  /* extra header size */ 0x28, 0x00, 0x00, 0x00,
  /* width */ LCD_W & 0xFF, LCD_W >> 8, 0x00, 0x00,
  /* height */ LCD_H & 0xFF, LCD_H >> 8, 0x00, 0x00,
  /* planes */ 0x01, 0x00,
  /* depth */ BMP_BITS_PER_PIXEL, 0x00,
  0x00, 0x00,
  0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xee, 0xee, 0xee, 0x00, 0xdd, 0xdd,
  0xdd, 0x00, 0xcc, 0xcc, 0xcc, 0x00, 0xbb, 0xbb, 0xbb, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x99, 0x99,
  0x99, 0x00, 0x88, 0x88, 0x88, 0x00, 0x77, 0x77, 0x77, 0x00, 0x66, 0x66, 0x66, 0x00, 0x55, 0x55,
  0x55, 0x00, 0x44, 0x44, 0x44, 0x00, 0x33, 0x33, 0x33, 0x00, 0x22, 0x22, 0x22, 0x00, 0x11, 0x11,
  0x11, 0x00, 0x00, 0x00, 0x00, 0x00
};

const char * writeScreenshot()
{
  FIL bmpFile;
  UINT written;
  char filename[42]; // /SCREENSHOTS/screen-2013-01-01-123540.bmp

  if (sdIsFull()) {
    POPUP_WARNING(STR_SDCARD_FULL_EXT);
    return STR_SDCARD_FULL_EXT;
  }

  // check and create folder here
  strcpy(filename, SCREENSHOTS_PATH);
  const char * error = sdCheckAndCreateDirectory(filename);
  if (error) {
    return error;
  }

#if defined(RTCLOCK)
  char * tmp = strAppend(&filename[sizeof(SCREENSHOTS_PATH)-1], "/screen");
  tmp = strAppendDate(tmp, true);
  strcpy(tmp, BMP_EXT);
#endif

  FRESULT result = f_open(&bmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  result = f_write(&bmpFile, BMP_HEADER, sizeof(BMP_HEADER), &written);
  if (result != FR_OK || written != sizeof(BMP_HEADER)) {
    f_close(&bmpFile);
    return SDCARD_ERROR(result);
  }

#if defined(COLORLCD)
  lv_img_dsc_t* snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR);
  if (!snapshot) { f_close(&bmpFile); return nullptr; }

  auto w = snapshot->header.w;
  auto h = snapshot->header.h;

  for (int y = h - 1; y >= 0; y--) {
    for (uint32_t x = 0; x < w; x++) {

      lv_color_t pixel = lv_img_buf_get_px_color(snapshot, x, y, {});

      uint32_t dst = (0xFF << 24)
          | (pixel.ch.red << 19)
          | (pixel.ch.green << 10)
          | (pixel.ch.blue << 3);

      if (f_write(&bmpFile, &dst, sizeof(dst), &written) != FR_OK || written != sizeof(dst)) {
        lv_snapshot_free(snapshot);
        f_close(&bmpFile);
        return SDCARD_ERROR(result);
      }
    }
  }

  lv_snapshot_free(snapshot);

#else // stdlcd

  for (int y=LCD_H-1; y>=0; y-=1) {
    for (int x=0; x<8*((LCD_W+7)/8); x+=2) {
      pixel_t byte = getPixel(x+1, y) + (getPixel(x, y) << 4);
#if defined(OLED_SCREEN)
      byte ^= 0xFF;
#endif
      if (f_write(&bmpFile, &byte, 1, &written) != FR_OK || written != 1) {
        f_close(&bmpFile);
        return SDCARD_ERROR(result);
      }
    }
  }
#endif

  f_close(&bmpFile);

  return nullptr;
}
