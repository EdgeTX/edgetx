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

#include "stm32_pulse_driver.h"

// Offset:           R          G          B
#define WS2812_RGB ((0 << 4) | (1 << 2) | (2))
#define WS2812_GRB ((1 << 4) | (0 << 2) | (2))

// RGB
#define WS2812_BYTES_PER_LED 3

// Number of LED periods used for trailing reset
#if !defined(WS2812_TRAILING_RESET)
#  define WS2812_TRAILING_RESET 10
#endif

void ws2812_init(const stm32_pulse_timer_t* timer, uint8_t* strip_colors,
                 uint8_t strip_len, uint8_t type);
void ws2812_update(const stm32_pulse_timer_t* timer);
void ws2812_dma_isr(const stm32_pulse_timer_t* timer);

// Returns true if a DMA transfer is currently in progress.
bool ws2812_is_busy(const stm32_pulse_timer_t* timer);

// Buffer-parameterised setter/getter. The caller owns the buffer; ws2812
// applies the strip's RGB byte ordering through the offsets configured at
// init time.
void ws2812_set_color_in_buf(uint8_t* buf, uint8_t led,
                             uint8_t r, uint8_t g, uint8_t b);
uint32_t ws2812_get_color_in_buf(const uint8_t* buf, uint8_t led);
bool ws2812_get_state_in_buf(const uint8_t* buf, uint8_t led);
