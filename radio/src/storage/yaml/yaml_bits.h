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

#pragma once

#include <stdint.h>

void yaml_put_bits(uint8_t* dst, uint32_t i, uint32_t bit_ofs, uint32_t bits);
uint32_t yaml_get_bits(uint8_t* src, uint32_t bit_ofs, uint32_t bits);

// assumes bits is a multiple of 8
bool yaml_is_zero(uint8_t* data, uint32_t bitoffs, uint32_t bits);

int32_t yaml_str2int_ref(const char*& val, uint8_t& val_len);
uint32_t yaml_str2uint_ref(const char*& val, uint8_t& val_len);

int32_t  yaml_str2int(const char* val, uint8_t val_len);
uint32_t yaml_str2uint(const char* val, uint8_t val_len);

uint32_t yaml_hex2uint(const char* val, uint8_t val_len);

char* yaml_unsigned2str(uint32_t i);
char* yaml_signed2str(int32_t i);

char* yaml_unsigned2hex(uint32_t i);
char* yaml_rgb2hex(uint32_t i);

int32_t yaml_to_signed(uint32_t i, uint32_t bits);
