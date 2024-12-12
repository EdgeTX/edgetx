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

#if !defined(SIMU)
#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>
#endif

extern const stm32_pulse_timer_t _led_timer;

static TimerHandle_t rgbLedTimer = nullptr;
static StaticTimer_t rgbLedTimerBuffer;

void rgbSetLedColor(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
#if defined(SIXPOS_SWITCH_INDEX)
  ws2812_set_color(led + 6, r, g, b);
#else
  ws2812_set_color(led, r, g, b);
#endif
}

uint32_t rgbGetLedColor(uint8_t led)
{
  return ws2812_get_color(led);
}

bool rgbGetState(uint8_t led)
{
  return ws2812_get_state(led);
}

void rgbLedColorApply()
{
  ws2812_update(&_led_timer);;
}

static void rgbLedTimerCb(TimerHandle_t xTimer)
{
  (void)xTimer;

  ws2812_update(&_led_timer);
}

void rgbLedStart()
{
  if (!rgbLedTimer) {
    rgbLedTimer =
        xTimerCreateStatic("rgbLed", LED_STRIP_REFRESH_PERIOD / RTOS_MS_PER_TICK, pdTRUE, (void*)0,
                           rgbLedTimerCb, &rgbLedTimerBuffer);
  }
  
  if (rgbLedTimer) {
    if( xTimerStart( rgbLedTimer, 0 ) != pdPASS ) {
      /* The timer could not be set into the Active state. */
    }
  }
}

void rgbLedStop()
{
  if (rgbLedTimer) {
    if( xTimerStop( rgbLedTimer, 5 / RTOS_MS_PER_TICK ) != pdPASS ) {
      /* The timer could not be stopped. */
    }
  }
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
