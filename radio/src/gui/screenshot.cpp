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

#include "opentx.h"

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
  VirtualFS& vfs = VirtualFS::instance();
  VfsFile bmpFile;
  size_t written;
  char filename[42]; // /SCREENSHOTS/screen-2013-01-01-123540.bmp

  // check and create folder here
  strcpy(filename, SCREENSHOTS_PATH);
  const char * error = vfs.checkAndCreateDirectory(filename);
  if (error) {
    return error;
  }

  char * tmp = strAppend(&filename[sizeof(SCREENSHOTS_PATH)-1], "/screen");
  tmp = strAppendDate(tmp, true);
  strcpy(tmp, BMP_EXT);

  VfsError result = vfs.openFile(bmpFile, filename, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }

  result = bmpFile.write(BMP_HEADER, sizeof(BMP_HEADER), written);
  if (result != VfsError::OK || written != sizeof(BMP_HEADER)) {
    bmpFile.close();
    return STORAGE_ERROR(result);
  }

#if defined(COLORLCD)
  for (int y = LCD_H - 1; y >= 0; y--) {
    for (int x = 0; x < LCD_W; x++) {
      lcdFront->reset();
      auto pixel = *(lcdFront->getPixelPtr(x, y));
      uint32_t dst = (0xFF << 24) + (GET_RED(pixel) << 16) + (GET_GREEN(pixel) << 8) + (GET_BLUE(pixel) << 0);
      if (bmpFile.write(&dst, sizeof(dst), written) != VfsError::OK || written != sizeof(dst)) {
        bmpFile.close();
        return STORAGE_ERROR(result);
      }
    }
  }
#else
  for (int y=LCD_H-1; y>=0; y-=1) {
    for (int x=0; x<8*((LCD_W+7)/8); x+=2) {
      pixel_t byte = getPixel(x+1, y) + (getPixel(x, y) << 4);
      if (bmpFile.write(&byte, 1, written) != VfsError::OK || written != 1) {
        bmpFile.close();
        return STORAGE_ERROR(result);
      }
    }
  }
#endif

  bmpFile.close();

  return nullptr;
}
