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

typedef struct {
  uint32_t (*get_size_kb)();
  uint32_t (*get_sector)(uint32_t address);
  uint32_t (*get_sector_size)(uint32_t sector);

  int (*erase_sector)(uint32_t address);
  int (*program)(uint32_t address, void* data, uint32_t len);
  int (*read)(uint32_t address, void* data, uint32_t len);

  void (*unlock)();
  void (*lock)();
} etx_flash_driver_t;
