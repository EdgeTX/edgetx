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

#include "stm32_ws2812.h"
#include "stm32_dma.h"

#if defined(DEBUG_WS2812)
  // LED_STRIP_DEBUG_GPIO && LED_STRIP_DEBUG_GPIO_PIN
  #include "hal.h"
#endif

#include "definitions.h"

#include <string.h>

// Pixel values
static uint8_t _led_colors[WS2812_BYTES_PER_LED * WS2812_MAX_LEDS];

// Timer used
// static const stm32_pulse_timer_t* _led_timer;

// LED strip length
static uint8_t _led_strip_len;

static uint8_t _r_offset;
static uint8_t _g_offset;
static uint8_t _b_offset;

// DMA buffer contains data for 2 LEDs and is filled
// half by half on DMA HT and TC IRQs
#define WS2821_DMA_BUFFER_HALF_LEN (WS2812_BYTES_PER_LED * 8)
#define WS2821_DMA_BUFFER_LEN      (WS2821_DMA_BUFFER_HALF_LEN * 2)

#define WS2812_FREQ            800000UL // 800 kHz
#define WS2812_TIMER_PERIOD    20UL
#define WS2812_ONE             (3 * WS2812_TIMER_PERIOD / 4)
#define WS2812_ZERO            (1 * WS2812_TIMER_PERIOD / 4)
#define WS2812_DMA_IRQ_PRIO    3

// Debug facility
#if defined(LED_STRIP_DEBUG_GPIO) && defined(LED_STRIP_DEBUG_GPIO_PIN)

#define WS2812_DBG_INIT _led_dbg_init()

#define WS2812_DBG_HIGH                                                 \
  LL_GPIO_SetOutputPin(LED_STRIP_DEBUG_GPIO, LED_STRIP_DEBUG_GPIO_PIN)

#define WS2812_DBG_LOW                                                  \
  LL_GPIO_ResetOutputPin(LED_STRIP_DEBUG_GPIO, LED_STRIP_DEBUG_GPIO_PIN)

static void _led_dbg_init() {
  LL_GPIO_InitTypeDef pinInit{0};
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pin = LED_STRIP_DEBUG_GPIO_PIN;
  LL_GPIO_Init(LED_STRIP_DEBUG_GPIO, &pinInit);
  WS2812_DBG_LOW;
}

#else // LED_STRIP_DEBUG_GPIO && LED_STRIP_DEBUG_GPIO_PIN

#define WS2812_DBG_INIT
#define WS2812_DBG_HIGH
#define WS2812_DBG_LOW

#endif

typedef uint16_t led_timer_value_t;
uint8_t pulse_inc = 1;

// DMA buffer contains pulses for 2 LED at a time
// (allows for refill at HT and TC)
#if defined(STM32_SUPPORT_32BIT_TIMERS)
static led_timer_value_t _led_dma_buffer[WS2821_DMA_BUFFER_LEN * 2] __DMA;
#else
static led_timer_value_t _led_dma_buffer[WS2821_DMA_BUFFER_LEN] __DMA;
#endif

static uint8_t _led_seq_cnt;

static void _fill_byte(uint8_t c, led_timer_value_t* dma_buffer)
{
  for (int i = 0; i < 8; i++) {
    dma_buffer[i*pulse_inc] = c & 0x80 ? WS2812_ONE : WS2812_ZERO;
    c <<= 1;
  }
}

static void _fill_pulses(const uint8_t* colors, led_timer_value_t* dma_buffer, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    _fill_byte(*colors, dma_buffer);
    dma_buffer += 8 * pulse_inc;
    colors++;
  }
}

static inline uint32_t _calc_offset(uint8_t tc)
{
  return tc * WS2821_DMA_BUFFER_HALF_LEN * pulse_inc;
}

static void _update_dma_buffer(const stm32_pulse_timer_t* tim, uint8_t tc)
{
  WS2812_DBG_HIGH;
  if (_led_seq_cnt < _led_strip_len) {

    auto idx = WS2812_BYTES_PER_LED * _led_seq_cnt;
    auto offset = _calc_offset(tc);
    _fill_pulses(&_led_colors[idx], &_led_dma_buffer[offset], WS2812_BYTES_PER_LED);
    _led_seq_cnt++;

  } else if(_led_seq_cnt < _led_strip_len + WS2812_TRAILING_RESET) {

    // no need to reset the buffer after 2 cycles
    if (_led_seq_cnt < _led_strip_len + 2) {
      auto offset = _calc_offset(tc);
      auto size = WS2821_DMA_BUFFER_HALF_LEN * sizeof(led_timer_value_t) * pulse_inc;
      memset(&_led_dma_buffer[offset], 0, size);
    }
    _led_seq_cnt++;

  } else {

    LL_DMA_DisableIT_TC(tim->DMAx, tim->DMA_Stream);
    LL_DMA_DisableIT_HT(tim->DMAx, tim->DMA_Stream);
    LL_DMA_DisableStream(tim->DMAx, tim->DMA_Stream);
    LL_TIM_CC_DisableChannel(tim->TIMx, tim->TIM_Channel);
  }
  WS2812_DBG_LOW;
}

void ws2812_dma_isr(const stm32_pulse_timer_t* tim)
{
  if (LL_DMA_IsEnabledIT_HT(tim->DMAx, tim->DMA_Stream) &&
      stm32_dma_check_ht_flag(tim->DMAx, tim->DMA_Stream)) {
    _update_dma_buffer(tim, 0);
  }

  if (LL_DMA_IsEnabledIT_TC(tim->DMAx, tim->DMA_Stream) &&
      stm32_dma_check_tc_flag(tim->DMAx, tim->DMA_Stream)) {
    _update_dma_buffer(tim, 1);
  }
}

static void _led_set_dma_periph_addr(const stm32_pulse_timer_t* tim)
{
  volatile uint32_t* cmp_reg = nullptr;
  switch(tim->TIM_Channel) {
  case LL_TIM_CHANNEL_CH1:
  case LL_TIM_CHANNEL_CH1N:
    cmp_reg = &tim->TIMx->CCR1;
    break;
  case LL_TIM_CHANNEL_CH2:
    cmp_reg = &tim->TIMx->CCR2;
    break;
  case LL_TIM_CHANNEL_CH3:
    cmp_reg = &tim->TIMx->CCR3;
    break;
  case LL_TIM_CHANNEL_CH4:
    cmp_reg = &tim->TIMx->CCR4;
    break;
  }

  LL_DMA_SetPeriphAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)cmp_reg);
}

static void _init_timer(const stm32_pulse_timer_t* tim)
{
  stm32_pulse_init(tim, WS2812_FREQ * WS2812_TIMER_PERIOD);
  stm32_pulse_config_output(tim, true, LL_TIM_OCMODE_PWM1, 0);
  LL_TIM_SetAutoReload(tim->TIMx, WS2812_TIMER_PERIOD - 1);

  // pulse driver uses DMA to ARR, but we need CCRx
  _led_set_dma_periph_addr(tim);

  LL_DMA_SetMode(tim->DMAx, tim->DMA_Stream, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetDataLength(tim->DMAx, tim->DMA_Stream, WS2821_DMA_BUFFER_LEN);
  LL_DMA_SetMemoryAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)_led_dma_buffer);
  
  // we need to use a higher prio to avoid having
  // issues with some other things used during boot
  NVIC_SetPriority(tim->DMA_IRQn, WS2812_DMA_IRQ_PRIO);
}

void ws2812_init(const stm32_pulse_timer_t* timer, uint8_t strip_len, uint8_t type)
{
  WS2812_DBG_INIT;
  pulse_inc = IS_TIM_32B_COUNTER_INSTANCE(timer->TIMx) ? 2 : 1;

  memset(_led_colors, 0, sizeof(_led_colors));
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  if (strip_len <= WS2812_MAX_LEDS) {
    _led_strip_len = strip_len;
  } else {
    _led_strip_len = WS2812_MAX_LEDS;
  }

  _r_offset = (type >> 4) & 0b11;
  _g_offset = (type >> 2) & 0b11;
  _b_offset = type & 0b11;

  _init_timer(timer);
}

void ws2812_set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
  if (led >= _led_strip_len) return;

  uint8_t* pixel = &_led_colors[led * WS2812_BYTES_PER_LED];
  pixel[_r_offset] = r;
  pixel[_g_offset] = g;
  pixel[_b_offset] = b;
}

uint32_t ws2812_get_color(uint8_t led)
{
  if (led >= _led_strip_len) return 0;

  uint8_t* pixel = &_led_colors[led * WS2812_BYTES_PER_LED];
  return  (pixel[1] << 16) +  (pixel[0] << 8) + pixel[2];
}

bool ws2812_get_state(uint8_t led)
{
  if (led >= _led_strip_len) return false;

  uint8_t* pixel = &_led_colors[led * WS2812_BYTES_PER_LED];
  return pixel[0] || pixel[1] || pixel[2];
}

void ws2812_update(const stm32_pulse_timer_t* tim)
{
  WS2812_DBG_HIGH;
  if (!stm32_pulse_if_not_running_disable(tim)) return;

  _led_seq_cnt = 0;
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  LL_DMA_EnableIT_HT(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_CC_EnableChannel(tim->TIMx, tim->TIM_Channel);
  LL_TIM_EnableCounter(tim->TIMx);

  WS2812_DBG_LOW;
}
