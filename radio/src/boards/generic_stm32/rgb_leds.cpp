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

#include "stm32_pulse_driver.h"
#include "rgb_leds.h"
#include "hal.h"
#include "edgetx.h"

#if defined(LED_STRIP_GPIO)

#include "stm32_ws2812.h"
#include "stm32_dma.h"
#include "stm32_gpio.h"
#include "hal/gpio.h"
#include "os/task.h"
#include "os/timer.h"

#include <string.h>

// Front buffer: read by the WS2812 DMA ISR. Mutated only inside
// _flush_and_update() and only when DMA is idle.
static uint8_t _led_colors[WS2812_BYTES_PER_LED * LED_STRIP_LENGTH];

// Back buffer: written by rgbSetLedColor (menu task / pre-OS, single writer).
// Read only inside _flush_and_update() when a new frame has been published.
static uint8_t _back_colors[WS2812_BYTES_PER_LED * LED_STRIP_LENGTH];

// Seqlock-style publish using a single counter:
//   - LSB set (odd)  → menu task is mid-batch, back buffer not consistent
//   - LSB clear (even) → frame is published; back buffer is stable
// Setters mark the seq odd before touching the back buffer; rgbLedColorApply
// rounds up to the next even value to publish the frame. The timer copies
// only when seq is even AND differs from the last consumed value.
//
// All writes to _commit_seq come from the menu task (or pre-OS). The timer
// task is the sole reader. 32-bit aligned access is atomic on Cortex-M;
// no locks needed.
static volatile uint32_t _commit_seq = 0;
static uint32_t _consumed_seq = 0;

// Compiler barrier: prevent the compiler from reordering plain memory
// accesses around a seq update.
static inline void _compiler_barrier()
{
  asm volatile("" ::: "memory");
}

// Mark the start of a back-buffer write batch (set LSB so the timer skips
// while we're modifying the buffer). Cheap no-op if already mid-batch.
static inline void _begin_batch()
{
  _commit_seq |= 1u;
  _compiler_barrier();
}

extern const stm32_pulse_timer_t _led_timer;

static timer_handle_t _refresh_timer = TIMER_INITIALIZER;

static void _flush_and_update()
{
  if (!ws2812_is_busy(&_led_timer)) {
    uint32_t seq = _commit_seq;
    // Skip if mid-batch (odd) or already consumed.
    if ((seq & 1u) == 0u && seq != _consumed_seq) {
      memcpy(_led_colors, _back_colors, sizeof(_led_colors));
      _consumed_seq = seq;
    }
  }
  ws2812_update(&_led_timer);
}

void rgbSetLedColor(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
  _begin_batch();
  ws2812_set_color_in_buf(_back_colors, led, r, g, b);
}

uint32_t rgbGetLedColor(uint8_t led)
{
  return ws2812_get_color_in_buf(_back_colors, led);
}

bool rgbGetState(uint8_t led)
{
  return ws2812_get_state_in_buf(_back_colors, led);
}

void rgbLedColorApply()
{
  // Publish: ensure the back-buffer write is complete, then advance to
  // the next even value. (seq | 1) + 1 == round up to next even.
  _compiler_barrier();
  _commit_seq = (_commit_seq | 1u) + 1u;
  if (!scheduler_is_running()) {
    // Pre-OS: timer task isn't running, drive the flush synchronously.
    _flush_and_update();
  }
}

void rgbLedClearAll()
{
  _begin_batch();
  memset(_back_colors, 0, sizeof(_back_colors));
  rgbLedColorApply();
}

__attribute__((weak)) void rgbLedOnUpdate() {}

static void _refresh_cb(timer_handle_t* timer)
{
  (void)timer;
  rgbLedOnUpdate();
  _flush_and_update();
}

static void rgbLedStart()
{
  if (!timer_is_created(&_refresh_timer)) {
    timer_create(&_refresh_timer, _refresh_cb, "rgbled",
                 LED_STRIP_REFRESH_PERIOD, true);
  }

  if (timer_start(&_refresh_timer) != 0) {
    TRACE("Failed to start RGB LED refresh timer");
  };
}

void rgbLedStop()
{
  timer_stop(&_refresh_timer);
}

const stm32_pulse_timer_t _led_timer = {
  .GPIO = LED_STRIP_GPIO,
  .GPIO_Alternate = LED_STRIP_GPIO_AF,
  .TIMx = LED_STRIP_TIMER,
  .TIM_Freq = LED_STRIP_TIMER_FREQ,
  .TIM_Channel = LED_STRIP_TIMER_CHANNEL,
  .TIM_IRQn = (IRQn_Type)-1,
  .DMAx = LED_STRIP_TIMER_DMA,
  .DMA_Stream = LED_STRIP_TIMER_DMA_STREAM,
  .DMA_Channel = LED_STRIP_TIMER_DMA_CHANNEL,
  .DMA_IRQn = LED_STRIP_TIMER_DMA_IRQn,
  .DMA_TC_CallbackPtr = nullptr,
};

static bool _hw_initialised = false;

void rgbLedHwInit()
{
  if (_hw_initialised) return;
  ws2812_init(&_led_timer, _led_colors, LED_STRIP_LENGTH, WS2812_GRB);
  _hw_initialised = true;
  rgbLedClearAll();
}

void rgbLedInit()
{
  rgbLedHwInit();
  rgbLedStart();
}

// Make sure the timer channel is supported
static_assert(__STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(LED_STRIP_TIMER_CHANNEL),
              "Unsupported timer channel");

// Make sure the DMA channel is supported
static_assert(__STM32_DMA_IS_STREAM_SUPPORTED(LED_STRIP_TIMER_DMA_STREAM),
              "Unsupported DMA stream");

#if !defined(LED_STRIP_TIMER_DMA_IRQHandler)
  #error "Missing LED_STRIP_TIMER_DMA_IRQHandler definition"
#endif

extern "C" void LED_STRIP_TIMER_DMA_IRQHandler()
{
  ws2812_dma_isr(&_led_timer);
}

#endif
