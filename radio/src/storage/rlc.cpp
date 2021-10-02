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

#include <inttypes.h>
#include <assert.h>
#include "debug.h"

#include "rlc.h"

#define CHECK_DST_SIZE() \
  if (cur-dst >= (int)dstsize) { \
    TRACE("RLC encoding size too big"); \
    return 0; \
  }

unsigned int compress(uint8_t * dst, unsigned int dstsize, const uint8_t * src, unsigned int srcsize)
{
  uint8_t * cur = dst;
  bool    run0   = (src[0] == 0);
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;

  for (unsigned int i=1; 1; i++) {
    bool cur0 = (i < srcsize) ? (src[i] == 0) : false;
    if (i==srcsize || cur0!=run0 || cnt==0x3f || (cnt0 && cnt==0xf)) {
      if (run0) {
        assert(cnt0==0);
        if (cnt<8 && i!=srcsize) {
          cnt0 = cnt;
        }
        else {
          CHECK_DST_SIZE();
          *cur++ = (cnt | 0x40);
        }
      }
      else {
        if (cnt0) {
          CHECK_DST_SIZE();
          *cur++ = (0x80 | (cnt0<<4) | cnt);
          cnt0 = 0;
        }
        else {
          CHECK_DST_SIZE();
          *cur++ = cnt;
        }
        for (int j=0; j<cnt; j++) {
          CHECK_DST_SIZE();
          *cur++ = src[i - cnt + j];
        }
      }
      cnt = 0;
      if (i==srcsize) break;
      run0 = cur0;
    }
    cnt++;
  }

  return cur-dst;
}

#undef CHECK_DST_SIZE
#define CHECK_DST_SIZE() \
  if (cur-dst >= (int)dstsize) { \
    TRACE("RLC decoding size too big"); \
    return 0; \
  }

unsigned int uncompress(uint8_t * dst, unsigned int dstsize, const uint8_t * src, unsigned int srcsize)
{
  uint8_t * cur = dst;
  uint8_t zeroes = 0;
  uint8_t bRlc = 0;

  for( ; 1; ) {
    if (zeroes > 0) {
      for (int i = 0; i < zeroes; i++) {
        CHECK_DST_SIZE();
        *cur++ = 0;
      }
      zeroes = 0;
    }

    if (srcsize == 0) {
      return cur - dst;
    }

    for (int i=0; i<bRlc; i++) {
      CHECK_DST_SIZE();
      *cur++ = *src++;
      if (--srcsize == 0) {
        return cur - dst;
      }
    }

    bRlc = *src++;
    --srcsize;

    if (!(bRlc & 0x7f)) {
      TRACE("RLC decoding error");
      return 0;
    }

    if (bRlc & 0x80) { // if contains high byte
      zeroes  = (bRlc>>4) & 0x07;
      bRlc    = bRlc & 0x0f;
    }
    else if (bRlc & 0x40) {
      zeroes = bRlc & 0x3f;
      bRlc   = 0;
    }
  }
}

#undef CHECK_DST_SIZE
