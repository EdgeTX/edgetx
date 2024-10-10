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

#include "stm32_cmsis.h"
#include <stdint.h>

#define GPIO_UNDEF (0xffffffff)

// #define GPIO_PIN(x, y) ((GPIOA_BASE + (x << 10)) | y)
#define GPIO_PIN(x, y) ((uintptr_t)x | y)

// Generate GPIO mode bitfields
//
// bit 0+1: pin mode (input / output)
// bit 2+3: pull resistor configuration
// bit   4: output type (0: push-pull, 1: open-drain)
//
#define _GPIO_MODE(io, pr, ot) ((io << 0) | (pr << 2) | (ot << 4))

enum {
  GPIO_IN    = _GPIO_MODE(0, 0, 0),    // input w/o pull R
  GPIO_IN_PD = _GPIO_MODE(0, 2, 0),    // input with pull-down
  GPIO_IN_PU = _GPIO_MODE(0, 1, 0),    // input with pull-up
  GPIO_OUT   = _GPIO_MODE(1, 0, 0),    // push-pull output
  GPIO_OD    = _GPIO_MODE(1, 0, 1),    // open-drain w/o pull R
  GPIO_OD_PU = _GPIO_MODE(1, 1, 1)     // open-drain with pull-up
};

enum {
    GPIO_AF0 = 0,
    GPIO_AF1,
    GPIO_AF2,
    GPIO_AF3,
    GPIO_AF4,
    GPIO_AF5,
    GPIO_AF6,
    GPIO_AF7,
    GPIO_AF8,
    GPIO_AF9,
    GPIO_AF10,
    GPIO_AF11,
    GPIO_AF12,
    GPIO_AF13,
    GPIO_AF14,
    GPIO_AF15,
    GPIO_AF_UNDEF
};

enum {
  GPIO_PIN_SPEED_LOW       = 0x00,
  GPIO_PIN_SPEED_MEDIUM    = 0x01,
  GPIO_PIN_SPEED_HIGH      = 0x02,
  GPIO_PIN_SPEED_VERY_HIGH = 0x03,
};
