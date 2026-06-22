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

#include "stm32_rgbleds.h"
#include "stm32_dma.h"

#if defined(DEBUG_RGBLEDS)
  // LED_STRIP_DEBUG_GPIO && LED_STRIP_DEBUG_GPIO_PIN
  #include "hal.h"
#endif

#include "definitions.h"

#include <string.h>

// Pixel values
static uint8_t* _led_colors = nullptr;

// LED strip length
static uint8_t _led_strip_len;

static uint8_t _r_offset;
static uint8_t _g_offset;
static uint8_t _b_offset;

// DMA buffer contains data for 2 LEDs and is filled
// half by half on DMA HT and TC IRQs
#define RGBLEDS_DMA_BUFFER_HALF_LEN  (RGBLEDS_BYTES_PER_LED * 8)
#define RGBLEDS_DMA_BUFFER_LEN       (RGBLEDS_DMA_BUFFER_HALF_LEN * 2)
#define RGBLEDS_DMA_IRQ_PRIO         3

// Bit timing in ns (period / '1' HIGH / '0' HIGH), converted to timer ticks at
// init from the actual timer clock so it holds on any clock domain.
#if defined(RGB_LEDS_900NS)
#define RGBLEDS_PERIOD_NS        917UL
#define RGBLEDS_T1H_NS           583UL
#define RGBLEDS_T0H_NS           250UL
#else
#define RGBLEDS_PERIOD_NS        1250UL
#define RGBLEDS_T1H_NS           938UL
#define RGBLEDS_T0H_NS           313UL
#endif

#define RGBLEDS_NS_TO_TICKS(freq, ns) \
  (uint32_t)(((uint64_t)(freq) * (ns) + 500000000UL) / 1000000000UL)


// Debug facility
#if defined(LED_STRIP_DEBUG_GPIO) && defined(LED_STRIP_DEBUG_GPIO_PIN)

#define RGBLEDS_DBG_INIT _led_dbg_init()

#define RGBLEDS_DBG_HIGH                                                 \
  LL_GPIO_SetOutputPin(LED_STRIP_DEBUG_GPIO, LED_STRIP_DEBUG_GPIO_PIN)

#define RGBLEDS_DBG_LOW                                                  \
  LL_GPIO_ResetOutputPin(LED_STRIP_DEBUG_GPIO, LED_STRIP_DEBUG_GPIO_PIN)

static void _led_dbg_init() {
  LL_GPIO_InitTypeDef pinInit{0};
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pin = LED_STRIP_DEBUG_GPIO_PIN;
  LL_GPIO_Init(LED_STRIP_DEBUG_GPIO, &pinInit);
  RGBLEDS_DBG_LOW;
}

#else // LED_STRIP_DEBUG_GPIO && LED_STRIP_DEBUG_GPIO_PIN

#define RGBLEDS_DBG_INIT
#define RGBLEDS_DBG_HIGH
#define RGBLEDS_DBG_LOW

#endif

typedef uint16_t led_timer_value_t;
uint8_t pulse_inc = 1;

// HIGH time (timer ticks) for a '1' and a '0' bit, computed at init.
static led_timer_value_t _led_one;
static led_timer_value_t _led_zero;

// DMA buffer contains pulses for 2 LED at a time
// (allows for refill at HT and TC)
#if defined(STM32_SUPPORT_32BIT_TIMERS)
static led_timer_value_t _led_dma_buffer[RGBLEDS_DMA_BUFFER_LEN * 2] __DMA_NO_CACHE;
#else
static led_timer_value_t _led_dma_buffer[RGBLEDS_DMA_BUFFER_LEN] __DMA_NO_CACHE;
#endif

static uint8_t _led_seq_cnt;

static void _fill_byte(uint8_t c, led_timer_value_t* dma_buffer)
{
  for (int i = 0; i < 8; i++) {
    dma_buffer[i*pulse_inc] = c & 0x80 ? _led_one : _led_zero;
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
  return tc * RGBLEDS_DMA_BUFFER_HALF_LEN * pulse_inc;
}

static void _update_dma_buffer(const stm32_pulse_timer_t* tim, uint8_t tc)
{
  RGBLEDS_DBG_HIGH;
  if (_led_seq_cnt < _led_strip_len) {

    auto idx = RGBLEDS_BYTES_PER_LED * _led_seq_cnt;
    auto offset = _calc_offset(tc);
    _fill_pulses(&_led_colors[idx], &_led_dma_buffer[offset], RGBLEDS_BYTES_PER_LED);
    _led_seq_cnt++;

  } else if(_led_seq_cnt < _led_strip_len + RGBLEDS_TRAILING_RESET) {

    // no need to reset the buffer after 2 cycles
    if (_led_seq_cnt < _led_strip_len + 2) {
      auto offset = _calc_offset(tc);
      auto size = RGBLEDS_DMA_BUFFER_HALF_LEN * sizeof(led_timer_value_t) * pulse_inc;
      memset(&_led_dma_buffer[offset], 0, size);
    }
    _led_seq_cnt++;

  } else {

    LL_DMA_DisableIT_TC(tim->DMAx, tim->DMA_Stream);
    LL_DMA_DisableIT_HT(tim->DMAx, tim->DMA_Stream);
    LL_DMA_DisableStream(tim->DMAx, tim->DMA_Stream);

    uint32_t timeout = 1000;
    while (LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream) && timeout--) 	{
      __NOP();  // Wait
    }

    LL_TIM_CC_DisableChannel(tim->TIMx, tim->TIM_Channel);
  }
  RGBLEDS_DBG_LOW;
}

void rgbleds_dma_isr(const stm32_pulse_timer_t* tim)
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
  // Run at the full timer clock (prescaler 0) and derive the periods from it.
  uint32_t cnt_freq = tim->TIM_Freq;
  stm32_pulse_init(tim, cnt_freq);
  stm32_pulse_config_output(tim, true, LL_TIM_OCMODE_PWM1, 0);

  uint32_t period = RGBLEDS_NS_TO_TICKS(cnt_freq, RGBLEDS_PERIOD_NS);
  _led_one  = RGBLEDS_NS_TO_TICKS(cnt_freq, RGBLEDS_T1H_NS);
  _led_zero = RGBLEDS_NS_TO_TICKS(cnt_freq, RGBLEDS_T0H_NS);
  LL_TIM_SetAutoReload(tim->TIMx, period - 1);

  // pulse driver uses DMA to ARR, but we need CCRx
  _led_set_dma_periph_addr(tim);

  LL_DMA_SetMode(tim->DMAx, tim->DMA_Stream, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetDataLength(tim->DMAx, tim->DMA_Stream, RGBLEDS_DMA_BUFFER_LEN);
  LL_DMA_SetMemoryAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)_led_dma_buffer);

  // we need to use a higher prio to avoid having
  // issues with some other things used during boot
  NVIC_SetPriority(tim->DMA_IRQn, RGBLEDS_DMA_IRQ_PRIO);
}

void rgbleds_init(const stm32_pulse_timer_t* timer, uint8_t* strip_colors,
                 uint8_t strip_len, uint8_t type)
{
  RGBLEDS_DBG_INIT;
  pulse_inc = IS_TIM_32B_COUNTER_INSTANCE(timer->TIMx) ? 2 : 1;

  _led_colors = strip_colors;
  _led_strip_len = strip_len;
  memset(_led_colors, 0, strip_len * RGBLEDS_BYTES_PER_LED);
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  _r_offset = (type >> 4) & 0b11;
  _g_offset = (type >> 2) & 0b11;
  _b_offset = type & 0b11;

  _init_timer(timer);
}

void rgbleds_set_color_in_buf(uint8_t* buf, uint8_t led,
                              uint8_t r, uint8_t g, uint8_t b)
{
  if (led >= _led_strip_len) return;

  uint8_t* pixel = &buf[led * RGBLEDS_BYTES_PER_LED];
  pixel[_r_offset] = r;
  pixel[_g_offset] = g;
  pixel[_b_offset] = b;
}

uint32_t rgbleds_get_color_in_buf(const uint8_t* buf, uint8_t led)
{
  if (led >= _led_strip_len) return 0;

  const uint8_t* pixel = &buf[led * RGBLEDS_BYTES_PER_LED];
  return (pixel[1] << 16) + (pixel[0] << 8) + pixel[2];
}

bool rgbleds_get_state_in_buf(const uint8_t* buf, uint8_t led)
{
  if (led >= _led_strip_len) return false;

  const uint8_t* pixel = &buf[led * RGBLEDS_BYTES_PER_LED];
  return pixel[0] || pixel[1] || pixel[2];
}

bool rgbleds_is_busy(const stm32_pulse_timer_t* tim)
{
  return LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream);
}

void rgbleds_update(const stm32_pulse_timer_t* tim)
{
  RGBLEDS_DBG_HIGH;
  if (!stm32_pulse_if_not_running_disable(tim)) return;

  _led_seq_cnt = 0;
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  LL_DMA_EnableIT_HT(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_CC_EnableChannel(tim->TIMx, tim->TIM_Channel);
  LL_TIM_EnableCounter(tim->TIMx);

  RGBLEDS_DBG_LOW;
}
