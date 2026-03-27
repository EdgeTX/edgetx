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

#if defined(COLORLCD)
// Scaling for RBG565 to RGB888
uint8_t rbScale[32] = {
  0,8,16,24,32,41,49,57,65,74,82,90,98,106,115,123,131,139,148,156,164,172,180,189,197,205,213,222,230,238,246,255
};
uint8_t gScale[64] = {
  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,
  129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255
};
#endif

// Uncomment to enable screenshots to be saved as PNG files
// Currently does not work on TX16S Mk3 for unknown reason
// #define ENABLE_PNG_SCREENSHOT

#if defined(ENABLE_PNG_SCREENSHOT) && defined(COLORLCD) && (defined(STM32H7) || defined(STM32H7RS))

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

static void convert_RGB565_to_RGB888(uint8_t* dst, lv_img_dsc_t* src, coord_t w, coord_t h)
{
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      lv_color_t pixel = lv_img_buf_get_px_color(src, x, y, {});
      *(dst++) = rbScale[pixel.ch.red];
      *(dst++) = gScale[pixel.ch.green];
      *(dst++) = rbScale[pixel.ch.blue];
    }
  }
}

static const char* writeResult = nullptr;

static void writeScreenToFile(void* context, void* data, int size)
{
  const char* file = (const char*)context;
  FIL fp;
  UINT written = 0;

  FRESULT result = f_open(&fp, file, FA_CREATE_ALWAYS | FA_WRITE);
  if (result == FR_OK) {
    result = f_write(&fp, data, size, &written);
  }
  f_close(&fp);

  if (result != FR_OK || (int)written != size) {
    writeResult = SDCARD_ERROR(result);
  }
}

static const char* _writeScreenshot()
{
  char filename[42]; // /SCREENSHOTS/screen-2013-01-01-123540.bmp

  if (sdIsFull()) {
    return STR_SDCARD_FULL_EXT;
  }

  // check and create folder here
  char* s = strAppend(filename, SCREENSHOTS_PATH);
  const char * error = sdCheckAndCreateDirectory(filename);
  if (error) {
    return error;
  }

  lv_img_dsc_t* snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR);
  if (!snapshot) {
    return "LVGL error creating snapshot";
  }

  s = strAppend(s, "/screen");
  s = strAppendDate(s, true);
  strAppend(s, ".png");

	// Open output file
  FRESULT result = f_open(&fp, file, FA_CREATE_ALWAYS | FA_WRITE);
	FILE *fout = fopen(filename, "wb");
	if (fout == NULL) {
		perror("Error: fopen");
		return EXIT_FAILURE;
	}
	
	// Initialize Tiny PNG Output
	struct TinyPngOut pngout;
	enum TinyPngOut_Status status = TinyPngOut_init(&pngout, (uint32_t)WIDTH, (uint32_t)HEIGHT, fout);
	if (status != TINYPNGOUT_OK)
		return printError(status);
	
	// Write image data
	status = TinyPngOut_write(&pngout, PIXELS, (size_t)(WIDTH * HEIGHT));
	if (status != TINYPNGOUT_OK)
		return printError(status);
	
	// Close output file
	if (fclose(fout) != 0) {
		perror("Error: fclose");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

  writeResult = nullptr;

  // allocate enough for 3 channels
  auto pixels = LCD_W * LCD_H;
  auto stride = LCD_W * 3;
  uint8_t* img = (uint8_t*)malloc(pixels * 3);
  convert_RGB565_to_RGB888(img, snapshot, LCD_W, LCD_H);
  stbi_write_png_to_func(writeScreenToFile, filename, LCD_W, LCD_H, 3, img, stride);
  free(img);

  lv_snapshot_free(snapshot);

  return writeResult;
}

#else

#if defined(COLORLCD)
constexpr uint16_t BMP_HEADERSIZE = 0x36;
constexpr uint8_t BMP_BITS_PER_PIXEL = 32;
#else
constexpr uint16_t BMP_HEADERSIZE = 0x76;
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
  /* compression */ 0x00, 0x00, 0x00, 0x00,
  /* image size */ 0x00, 0x00, 0x00, 0x00,
  /* hor res 72dpi */ 0x13, 0x0B, 0x00, 0x00,
  /* ver res 72 dpi */ 0x13, 0x0B, 0x00, 0x00,
  /* num colors */ 0x00, 0x00, 0x00, 0x00,
  /* important colors */ 0x00, 0x00, 0x00, 0x00,
#if !defined(COLORLCD)
  /* grayscale palette */
#if defined(OLED_SCREEN)
  0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x00, 0x22, 0x22, 0x22, 0x00, 0x33, 0x33, 0x33, 0x00,
  0x44, 0x44, 0x44, 0x00, 0x55, 0x55, 0x55, 0x00, 0x66, 0x66, 0x66, 0x00, 0x77, 0x77, 0x77, 0x00,
  0x88, 0x88, 0x88, 0x00, 0x99, 0x99, 0x99, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0xbb, 0xbb, 0xbb, 0x00,
  0xcc, 0xcc, 0xcc, 0x00, 0xdd, 0xdd, 0xdd, 0x00, 0xee, 0xee, 0xee, 0x00, 0xff, 0xff, 0xff, 0x00,
#else
  0xff, 0xff, 0xff, 0x00, 0xee, 0xee, 0xee, 0x00, 0xdd, 0xdd, 0xdd, 0x00, 0xcc, 0xcc, 0xcc, 0x00,
  0xbb, 0xbb, 0xbb, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x99, 0x99, 0x99, 0x00, 0x88, 0x88, 0x88, 0x00,
  0x77, 0x77, 0x77, 0x00, 0x66, 0x66, 0x66, 0x00, 0x55, 0x55, 0x55, 0x00, 0x44, 0x44, 0x44, 0x00,
  0x33, 0x33, 0x33, 0x00, 0x22, 0x22, 0x22, 0x00, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif
#endif
};

static const char* _writeScreenshot()
{
  FIL bmpFile;
  UINT written;
  char filename[42]; // /SCREENSHOTS/screen-2013-01-01-123540.bmp

  if (sdIsFull()) {
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

#if defined(COLORLCD)
  lv_img_dsc_t* snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR);
  if (!snapshot) {
    f_close(&bmpFile);
    return "LVGL error creating snapshot";
  }

  auto w = snapshot->header.w;
  auto h = snapshot->header.h;

  constexpr int BMP_BUF_SIZE = FF_MAX_SS * 4;
  static uint8_t _bmp_buf[BMP_BUF_SIZE];
  memcpy(_bmp_buf, BMP_HEADER, BMP_HEADERSIZE);
  UINT bpos = BMP_HEADERSIZE;

  for (int y = h - 1; y >= 0; y--) {
    for (uint32_t x = 0; x < w; x++) {

      lv_color_t pixel = lv_img_buf_get_px_color(snapshot, x, y, {});

      _bmp_buf[bpos++] = rbScale[pixel.ch.blue];
      _bmp_buf[bpos++] = gScale[pixel.ch.green];

      // Header size is a multiple of 2, sector size is a multiple of 4
      // so we will hit end of buffer half way through a pixel
      if (bpos == BMP_BUF_SIZE) {
        result = f_write(&bmpFile, _bmp_buf, bpos, &written);

        if (result != FR_OK || written != bpos) {
          lv_snapshot_free(snapshot);
          f_close(&bmpFile);
          return SDCARD_ERROR(result);
        }

        bpos = 0;
      }

      _bmp_buf[bpos++] = rbScale[pixel.ch.red];
      _bmp_buf[bpos++] = 0xFF;
    }
  }

  if (bpos > 0) {
    result = f_write(&bmpFile, _bmp_buf, bpos, &written);

    if (result != FR_OK || written != bpos) {
      lv_snapshot_free(snapshot);
      f_close(&bmpFile);
      return SDCARD_ERROR(result);
    }
  }

  lv_snapshot_free(snapshot);

#else // stdlcd

  result = f_write(&bmpFile, BMP_HEADER, sizeof(BMP_HEADER), &written);
  if (result != FR_OK || written != sizeof(BMP_HEADER)) {
    f_close(&bmpFile);
    return SDCARD_ERROR(result);
  }

#if LCD_W == 128
  pixel_t inv = g_eeGeneral.invertLCD ? 0xFF : 0;
#endif

  for (int y=LCD_H-1; y>=0; y-=1) {
    for (int x=0; x<8*((LCD_W+7)/8); x+=2) {
#if LCD_W == 128
      pixel_t byte = (getPixel(x+1, y) | (getPixel(x, y) << 4)) ^ inv;
#else
      pixel_t byte = getPixel(x+1, y) | (getPixel(x, y) << 4);
#endif
      result = f_write(&bmpFile, &byte, 1, &written);
      if (result != FR_OK || written != 1) {
        f_close(&bmpFile);
        return SDCARD_ERROR(result);
      }
    }
  }
#endif

  f_close(&bmpFile);

  return nullptr;
}

#endif

void writeScreenshot()
{
  const char* res = _writeScreenshot();
  if (res)
    POPUP_WARNING(res);
}
