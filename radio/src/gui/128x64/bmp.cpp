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
#include "VirtualFS.h"

uint8_t * lcdLoadBitmap(uint8_t * bmp, const char * filename, uint8_t width, uint8_t height)
{
  VfsFile bmpFile;
  size_t read;
  uint8_t bmpBuf[LCD_W]; /* maximum with LCD_W */
  uint8_t * buf = &bmpBuf[0];

  if (width > LCD_W) {
    return nullptr;
  }

  VfsError result = VirtualFS::instance().openFile(bmpFile, filename, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
  if (result != VfsError::OK) {
    return nullptr;
  }

  if (bmpFile.size() < 14) {
    bmpFile.close();
    return nullptr;
  }

  result = bmpFile.read(buf, 14, read);
  if (result != VfsError::OK || read != 14) {
    bmpFile.close();
    return nullptr;
  }

  if (buf[0] != 'B' || buf[1] != 'M') {
    &bmpFile.close();
    return nullptr;
  }

  uint32_t fsize  = *((uint32_t *)&buf[2]);
  uint32_t hsize  = *((uint32_t *)&buf[10]); /* header size */

  uint32_t len = limit((uint32_t)4, (uint32_t)(hsize-14), (uint32_t)32);
  result = bmpFile.read(buf, len, read);
  if (result != VfsError::OK || read != len) {
    bmpFile.close();
    return nullptr;
  }

  uint32_t ihsize = *((uint32_t *)&buf[0]); /* more header size */

  /* invalid header size */
  if (ihsize + 14 > hsize) {
    bmpFile.close();
    return nullptr;
  }

  /* sometimes file size is set to some headers size, set a real size in that case */
  if (fsize == 14 || fsize == ihsize + 14) {
    fsize = bmpFile.size() - 2;
  }

  /* declared file size less than header size */
  if (fsize <= hsize) {
    bmpFile.close();
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
        bmpFile.close();
      return nullptr;
  }

  if (*((uint16_t *)&buf[0]) != 1) { /* planes */
      bmpFile.close();
    return nullptr;
  }

  if (w > width || h > height) {
      bmpFile.close();
    return nullptr;
  }

  uint16_t depth = *((uint16_t *)&buf[2]);

  buf = &bmpBuf[0];

  if (bmpFile.lseek(hsize) != VfsError::OK) {
      bmpFile.close();
    return nullptr;
  }

  uint8_t * dest = bmp;

  *dest++ = w;
  *dest++ = h;

  memset(dest, 0, BITMAP_BUFFER_SIZE(w, h) - 2);

  uint8_t rowSize;

  switch (depth) {
    case 1:
      rowSize = ((w + 31) / 32) * 4;
      for (int8_t i=h-1; i>=0; i--) {
        result = bmpFile.read(buf, rowSize, read);
        if (result != VfsError::OK || read != rowSize) {
            bmpFile.close();
          return nullptr;
        }

        for (uint8_t j=0; j<w; j++) {
          if (!(buf[j/8] & (1<<(7-(j%8))))) {
            uint8_t *dst = dest + i / 8 * w + j;
            *dst |= (0x01 << (i & 0x07));
          }
        }
      }
      break;

    default:
      bmpFile.close();
      return nullptr;
  }

  bmpFile.close();
  return bmp;
}
