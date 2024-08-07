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

uint8_t * lcdLoadBitmap(uint8_t * bmp, const char * filename, uint16_t width, uint16_t height)
{
  FIL bmpFile;
  UINT read;
  uint8_t palette[16];
  uint8_t bmpBuf[LCD_W]; /* maximum with LCD_W */
  uint8_t * buf = &bmpBuf[0];

  if (width > LCD_W) {
    return nullptr;
  }

  FRESULT result = f_open(&bmpFile, filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return nullptr;
  }

  if (f_size(&bmpFile) < 14) {
    f_close(&bmpFile);
    return nullptr;
  }

  result = f_read(&bmpFile, buf, 14, &read);
  if (result != FR_OK || read != 14) {
    f_close(&bmpFile);
    return nullptr;
  }

  if (buf[0] != 'B' || buf[1] != 'M') {
    f_close(&bmpFile);
    return nullptr;
  }

  uint32_t fsize  = *((uint32_t *)&buf[2]);
  uint32_t hsize  = *((uint32_t *)&buf[10]); /* header size */

  uint32_t len = limit((uint32_t)4, (uint32_t)(hsize-14), (uint32_t)32);
  result = f_read(&bmpFile, buf, len, &read);
  if (result != FR_OK || read != len) {
    f_close(&bmpFile);
    return nullptr;
  }

  uint32_t ihsize = *((uint32_t *)&buf[0]); /* more header size */

  /* invalid header size */
  if (ihsize + 14 > hsize) {
    f_close(&bmpFile);
    return nullptr;
  }

  /* sometimes file size is set to some headers size, set a real size in that case */
  if (fsize == 14 || fsize == ihsize + 14)
    fsize = f_size(&bmpFile) - 2;

  /* declared file size less than header size */
  if (fsize <= hsize) {
    f_close(&bmpFile);
    return nullptr;
  }

  uint32_t w, h;

  switch (ihsize){
    case  40: // windib
    case  56: // windib v3
    case  64: // OS/2 v2
    case 108: // windib v4
    case 124: // windib v5
      w  = *((uint32_t *)&buf[4]);
      h = *((uint32_t *)&buf[8]);
      buf += 12;
      break;
    case  12: // OS/2 v1
      w  = *((uint16_t *)&buf[4]);
      h = *((uint16_t *)&buf[6]);
      buf += 8;
      break;
    default:
      f_close(&bmpFile);
      return nullptr;
  }

  if (*((uint16_t *)&buf[0]) != 1) { /* planes */
    f_close(&bmpFile);
    return nullptr;
  }

  if (w > width || h > height) {
    f_close(&bmpFile);
    return nullptr;
  }

  uint16_t depth = *((uint16_t *)&buf[2]);

  buf = &bmpBuf[0];

  if (depth == 4) {
    if (f_lseek(&bmpFile, hsize-64) != FR_OK || f_read(&bmpFile, buf, 64, &read) != FR_OK || read != 64) {
      f_close(&bmpFile);
      return nullptr;
    }
    for (uint8_t i=0; i<16; i++) {
      palette[i] = buf[4*i] >> 4;
    }
  }
  else {
    if (f_lseek(&bmpFile, hsize) != FR_OK) {
      f_close(&bmpFile);
      return nullptr;
    }
  }

  uint8_t * dest = bmp;

  *dest++ = w;
  *dest++ = h;

  memset(dest, 0, BITMAP_BUFFER_SIZE(w, h) - 2);

  uint32_t rowSize;

  switch (depth) {
    case 1:
      rowSize = ((w+31)/32)*4;
      for (uint32_t i=0; i<h; i+=2) {
        result = f_read(&bmpFile, buf, rowSize*2, &read);
        if (result != FR_OK || read != rowSize*2) {
          f_close(&bmpFile);
          return nullptr;
        }

        for (uint32_t j=0; j<w; j++) {
          uint8_t * dst = dest + (h-i-2)/2 * w + j;
          if (!(buf[j/8] & (1<<(7-(j%8)))))
            *dst |= 0xF0;
          if (!(buf[rowSize+j/8] & (1<<(7-(j%8)))))
            *dst |= 0x0F;
        }
      }
      break;

    case 4:
      rowSize = ((4*w+31)/32)*4;
      for (int32_t i=h-1; i>=0; i--) {
        result = f_read(&bmpFile, buf, rowSize, &read);
        if (result != FR_OK || read != rowSize) {
          f_close(&bmpFile);
          return nullptr;
        }
        uint8_t * dst = dest + (i/2)*w;
        for (uint32_t j=0; j<w; j++) {
          uint8_t index = (buf[j/2] >> ((j & 1) ? 0 : 4)) & 0x0F;
          uint8_t val = palette[index] << ((i & 1) ? 4 : 0);
          *dst++ |= val ^ ((i & 1) ? 0xF0 : 0x0F);
        }
      }
      break;

    default:
      f_close(&bmpFile);
      return nullptr;
  }

  f_close(&bmpFile);
  return bmp;
}

uint8_t modelBitmap[MODEL_BITMAP_SIZE] __DMA;

bool loadModelBitmap(char * name, uint8_t * bitmap)
{
  uint8_t len = zlen(name, LEN_BITMAP_NAME);
  if (len > 0) {
    char lfn[] = BITMAPS_PATH "/xxxxxxxxxx.bmp";
    strncpy(lfn+sizeof(BITMAPS_PATH), name, len);
    strcpy(lfn+sizeof(BITMAPS_PATH)+len, BMP_EXT);
    if (lcdLoadBitmap(bitmap, lfn, MODEL_BITMAP_WIDTH, MODEL_BITMAP_HEIGHT)) {
      return true;
    }
  }

  // In all error cases, we set the default logo
  RleBitmap pic(logo_taranis, 0);
  *bitmap++ = pic.getWidth();
  *bitmap++ = pic.getRawRows();
  for(int i=0; i < MODEL_BITMAP_SIZE-2; i++)
    *bitmap++ = pic.getNext();
  return false;
}
