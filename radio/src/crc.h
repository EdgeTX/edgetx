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

#include <inttypes.h>

enum {
  CRC_1021,
  CRC_1189,
};

extern const unsigned short * const crc16tab[2];

uint8_t crc8(const uint8_t * ptr, uint32_t len);
uint8_t crc8_BA(const uint8_t * ptr, uint32_t len);
uint16_t crc16(uint8_t index, const uint8_t * buf, uint32_t len, uint16_t start = 0);
