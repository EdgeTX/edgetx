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

#include <cinttypes>

#include <stdio.h>
#include "yaml_bits.h"
#include "yaml_parser.h"

#include <limits.h>     /* CHAR_BIT */

#define _IS_ALIGNED(addr) (((intptr_t)addr & 0x3) == 0)

#define BIT_MASK(__TYPE__, __ONE_COUNT__) \
    ((__TYPE__) (-((__ONE_COUNT__) != 0))) \
    & (((__TYPE__) -1) >> ((sizeof(__TYPE__) * CHAR_BIT) - (__ONE_COUNT__)))

#define MASK_LOWER(bits) BIT_MASK(uint32_t, bits)
#define MASK_UPPER(bits) (0xFF << bits)

void yaml_put_bits(uint8_t* dst, uint32_t i, uint32_t bit_ofs, uint32_t bits)
{
    i &= MASK_LOWER(bits);

    if (bit_ofs) {

        *dst &= ~((MASK_LOWER(bits) << bit_ofs) & 0xFF);
        *(dst++) |= (i << bit_ofs) & 0xFF;

        if (bits <= 8 - bit_ofs)
            return;

        bits -= 8 - bit_ofs;
        i = i >> (8 - bit_ofs);
    }

    while(bits >= 8) {
        *(dst++) = i & 0xFF;
        bits -= 8;
        i = i >> 8;
    }

    if (bits) {
        uint8_t mask = MASK_UPPER(bits);
        *dst &= mask;
        *dst |= i & ~mask;
    }
}


uint32_t yaml_get_bits(uint8_t* src, uint32_t bit_ofs, uint32_t bits)
{
    uint32_t i = 0;
    uint32_t bit_shift = 0;

    if (bit_ofs) {
        i = (*(src++) & MASK_UPPER(bit_ofs)) >> bit_ofs;

        if (bits <= 8 - bit_ofs) {
            i &= MASK_LOWER(bits);
            return i;
        }

        bit_shift = 8 - bit_ofs;
        bits     -= bit_shift;
    }
    
    while(bits >= 8) {

        i |= (uint32_t)*(src++) << bit_shift;

        bits      -= 8;
        bit_shift += 8;
    }

    if (bits) {
        i |= ((uint32_t)*src & MASK_LOWER(bits)) << bit_shift;
    }

    return i;
}

// if the start address is not aligned on a byte,
// checking max 32 bits is supported.
bool yaml_is_zero(uint8_t* data, uint32_t bitoffs, uint32_t bits)
{
  data += bitoffs >> 3;
  bitoffs &= 0x7;

  if (bitoffs) {
    return !yaml_get_bits(data, bitoffs, bits);
  }

  if(_IS_ALIGNED(data)) {
    while (bits >= 32) {
      if (*(uint32_t*)data) return false;
      data += 4;
      bits -= 32;
    }
  }

  while (bits >= 8) {
    if (*data) return false;
    data++;
    bits -= 8;
  }

  if (bits) {
    return !yaml_get_bits(data, 0, bits);
  }

  return true;
}

int32_t yaml_str2int_ref(const char*& val, uint8_t& val_len)
{
    bool  neg = false;
    int i_val = 0;

    while (val_len > 0) {
      if (*val == '-') {
        neg = true;
      } else if (*val >= '0' && *val <= '9') {
        i_val = i_val * 10 + (*val - '0');
      } else {
        break;
      }
      val++; val_len--;
    }

    return neg ? -i_val : i_val;
}

int32_t yaml_str2int(const char* val, uint8_t val_len)
{
    return yaml_str2int_ref(val, val_len);
}

uint32_t yaml_str2uint_ref(const char*& val, uint8_t& val_len)
{
    uint32_t i_val = 0;
    
    while(val_len > 0) {
        if (*val >= '0' && *val <= '9') {
            i_val = i_val * 10 + (*val - '0');
        } else {
            break;
        }
        val++; val_len--;
    }

    return i_val;
}

uint32_t yaml_str2uint(const char* val, uint8_t val_len)
{
    return yaml_str2uint_ref(val, val_len);
}

uint32_t yaml_hex2uint(const char* val, uint8_t val_len)
{
    uint32_t i_val = 0;
    
    while(val_len > 0) {
        if (*val >= '0' && *val <= '9') {
          i_val <<= 4;
          i_val |= (*val - '0') & 0xF;
        } else if (*val >= 'A' && *val <= 'F') {
          i_val <<= 4;
          i_val |= (*val - 'A' + 10) & 0xF;
        } else if (*val >= 'a' && *val <= 'f') {
          i_val <<= 4;
          i_val |= (*val - 'a' + 10) & 0xF;
        } else {
            break;
        }
        val++; val_len--;
    }

    return i_val;
}

constexpr int INT2STR_LEN = 16;

static char int2str_buffer[INT2STR_LEN] = {0};

char* yaml_unsigned2str(uint32_t i)
{
  snprintf(int2str_buffer, INT2STR_LEN, "%" PRIu32, i);
  return int2str_buffer;
}

char* yaml_signed2str(int32_t i)
{
  snprintf(int2str_buffer, INT2STR_LEN, "%" PRId32, i);
  return int2str_buffer;
}

char* yaml_unsigned2hex(uint32_t i)
{
  snprintf(int2str_buffer, INT2STR_LEN, "%08" PRIX32, i);
  return int2str_buffer;
}

char* yaml_rgb2hex(uint32_t i)
{
  snprintf(int2str_buffer, INT2STR_LEN, "%06" PRIX32, i);
  return int2str_buffer;
}

int32_t yaml_to_signed(uint32_t i, uint32_t bits)
{
    if (bits < 32 && (i & (1 << (bits-1)))) {
        i |= 0xFFFFFFFF << bits;
    }

    return i;
}
