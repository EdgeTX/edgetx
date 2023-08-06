/*
 * Copyright (C) EdgeTx
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
  uint8_t  polarity;
  uint16_t cmp_val;
} etx_timer_config_t;

typedef struct {
  void* (*init)(void* hw_def, const etx_timer_config_t* cfg);
  void (*deinit)(void* ctx);
  void (*send)(void* ctx, const etx_timer_config_t* cfg, const void* pulses, uint16_t length);
} etx_timer_driver_t;
