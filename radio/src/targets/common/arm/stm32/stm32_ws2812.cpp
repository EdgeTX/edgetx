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
#include "stm32_gpio.h"

// LED_STRIP_LENGTH (and LED_STRIP_DEBUG_GPIO when DEBUG_WS2812)
#include "hal.h"

#include "definitions.h"

#include <string.h>

// Pixel values
static uint8_t* _led_colors = nullptr;

// LED strip length
static uint8_t _led_strip_len;

static uint8_t _r_offset;
static uint8_t _g_offset;
static uint8_t _b_offset;

#define WS2812_SLOTS_PER_LED        (WS2812_BYTES_PER_LED * 8)

#define WS2812_FREQ            800000UL // 800 kHz
#define WS2812_TIMER_PERIOD    20UL
#define WS2812_ONE             (3 * WS2812_TIMER_PERIOD / 4)
#define WS2812_ZERO            (1 * WS2812_TIMER_PERIOD / 4)
#define WS2812_DMA_IRQ_PRIO    3

// Zero slots appended to the frame: the line stays low while they are sent,
// so the end-of-frame IRQ may run late without emitting a spurious bit.
#define WS2812_TRAIL_SLOTS          4

#if !defined(LED_STRIP_LENGTH)
  #define LED_STRIP_LENGTH           1
#endif

#define WS2812_FRAME_SLOTS \
  (LED_STRIP_LENGTH * WS2812_SLOTS_PER_LED + WS2812_TRAIL_SLOTS)

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

// The whole frame is emitted in a single DMA transfer: the buffer is filled
// before the transfer starts and no refill deadline exists, so an ISR delayed
// by a higher priority IRQ (SDMMC, USB) can no longer corrupt a frame.
#if defined(STM32_SUPPORT_32BIT_TIMERS)
static led_timer_value_t _led_dma_buffer[WS2812_FRAME_SLOTS * 2] __DMA_NO_CACHE;
#else
static led_timer_value_t _led_dma_buffer[WS2812_FRAME_SLOTS] __DMA_NO_CACHE;
#endif

// Number of slots actually sent, from the strip length given at init
static uint32_t _frame_slots;

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

static void _end_of_frame(const stm32_pulse_timer_t* tim)
{
  WS2812_DBG_HIGH;

  LL_DMA_DisableIT_TC(tim->DMAx, tim->DMA_Stream);
#if defined(STM32H5) || defined(STM32H7RS)
  LL_DMA_DisableChannel(tim->DMAx, tim->DMA_Stream);
#else
  LL_DMA_DisableStream(tim->DMAx, tim->DMA_Stream);

  uint32_t timeout = 1000;
  while (LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream) && timeout--) {
    __NOP();  // Wait
  }
#endif

  // Stop the request source, but leave the channel and the counter running
  // with a null compare value: the output is then actively held low between
  // frames. Disabling the channel releases the pad instead (OSSR = 0), the
  // line floats, and a WS2812 that misses its reset gap keeps counting bits
  // across frames -- which shifts the whole strip by one LED.
  LL_TIM_DisableDMAReq_UPDATE(tim->TIMx);
  stm32_pulse_set_cmp_val(tim, 0);

  WS2812_DBG_LOW;
}

void ws2812_dma_isr(const stm32_pulse_timer_t* tim)
{
  if (LL_DMA_IsEnabledIT_TC(tim->DMAx, tim->DMA_Stream) &&
      stm32_dma_check_tc_flag(tim->DMAx, tim->DMA_Stream)) {
    _end_of_frame(tim);
  }
}

static volatile uint32_t* _led_cmp_reg(const stm32_pulse_timer_t* tim)
{
  switch(tim->TIM_Channel) {
  case LL_TIM_CHANNEL_CH1:
  case LL_TIM_CHANNEL_CH1N:
    return &tim->TIMx->CCR1;
  case LL_TIM_CHANNEL_CH2:
    return &tim->TIMx->CCR2;
  case LL_TIM_CHANNEL_CH3:
    return &tim->TIMx->CCR3;
  case LL_TIM_CHANNEL_CH4:
    return &tim->TIMx->CCR4;
  }
  return nullptr;
}

#if defined(STM32H5) || defined(STM32H7RS)

// GPDMA: one linear block (mem -> CCRx) per frame, triggered by TIMx_UP
static void _led_setup_dma(const stm32_pulse_timer_t* tim)
{
  LL_DMA_DisableChannel(tim->DMAx, tim->DMA_Stream);

  const bool word = (pulse_inc == 2); // 32-bit timer -> 32-bit CCR writes

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  dmaInit.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  dmaInit.SrcBurstLength = 1;
  dmaInit.DestBurstLength = 1;
  dmaInit.SrcIncMode = LL_DMA_SRC_INCREMENT;
  dmaInit.DestIncMode = LL_DMA_DEST_FIXED;
  dmaInit.SrcDataWidth = word ? LL_DMA_SRC_DATAWIDTH_WORD
                              : LL_DMA_SRC_DATAWIDTH_HALFWORD;
  dmaInit.DestDataWidth = word ? LL_DMA_DEST_DATAWIDTH_WORD
                               : LL_DMA_DEST_DATAWIDTH_HALFWORD;
  dmaInit.SrcAddress = (intptr_t)_led_dma_buffer;
  dmaInit.DestAddress = (intptr_t)_led_cmp_reg(tim);
  dmaInit.BlkDataLength = _frame_slots * sizeof(led_timer_value_t) * pulse_inc;
  dmaInit.Request = tim->DMA_Channel;
  dmaInit.Priority = LL_DMA_HIGH_PRIORITY;
  dmaInit.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
  dmaInit.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  dmaInit.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  dmaInit.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
  dmaInit.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  dmaInit.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  dmaInit.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  dmaInit.LinkedListBaseAddr = 0;
  dmaInit.LinkedListAddrOffset = 0;
  dmaInit.Mode = LL_DMA_NORMAL;
  LL_DMA_Init(tim->DMAx, tim->DMA_Stream, &dmaInit);
}

#else

static void _led_set_dma_periph_addr(const stm32_pulse_timer_t* tim)
{
  LL_DMA_SetPeriphAddress(tim->DMAx, tim->DMA_Stream,
                          (uint32_t)_led_cmp_reg(tim));
}

#endif

static void _init_timer(const stm32_pulse_timer_t* tim)
{
  stm32_pulse_init(tim, WS2812_FREQ * WS2812_TIMER_PERIOD);
  stm32_pulse_config_output(tim, true, LL_TIM_OCMODE_PWM1, 0);
  LL_TIM_SetAutoReload(tim->TIMx, WS2812_TIMER_PERIOD - 1);

  // Insurance for the windows where the timer does not drive the pad (boot,
  // de-init): a floating WS2812 input can miss the inter-frame reset.
  LL_GPIO_SetPinPull(gpio_get_port(tim->GPIO), 1 << gpio_get_pin(tim->GPIO),
                     LL_GPIO_PULL_DOWN);

#if !defined(STM32H5) && !defined(STM32H7RS)
  // pulse driver uses DMA to ARR, but we need CCRx
  _led_set_dma_periph_addr(tim);

  // One shot per frame: NDTR and the memory address are re-programmed by
  // ws2812_update() before every transfer.
  LL_DMA_SetMode(tim->DMAx, tim->DMA_Stream, LL_DMA_MODE_NORMAL);
#endif

  // we need to use a higher prio to avoid having
  // issues with some other things used during boot
  NVIC_SetPriority(tim->DMA_IRQn, WS2812_DMA_IRQ_PRIO);
}

void ws2812_init(const stm32_pulse_timer_t* timer, uint8_t* strip_colors,
                 uint8_t strip_len, uint8_t type)
{
  WS2812_DBG_INIT;
  pulse_inc = IS_TIM_32B_COUNTER_INSTANCE(timer->TIMx) ? 2 : 1;

  if (strip_len > LED_STRIP_LENGTH) strip_len = LED_STRIP_LENGTH;

  _led_colors = strip_colors;
  _led_strip_len = strip_len;
  _frame_slots = strip_len * WS2812_SLOTS_PER_LED + WS2812_TRAIL_SLOTS;
  memset(_led_colors, 0, strip_len * WS2812_BYTES_PER_LED);
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  _r_offset = (type >> 4) & 0b11;
  _g_offset = (type >> 2) & 0b11;
  _b_offset = type & 0b11;

  _init_timer(timer);

  // Drive the data line low right away, and keep it driven until the first
  // frame: an idle WS2812 input must never be left floating.
  stm32_pulse_set_cmp_val(timer, 0);
  LL_TIM_CC_EnableChannel(timer->TIMx, timer->TIM_Channel);
  LL_TIM_EnableCounter(timer->TIMx);
}

void ws2812_set_color_in_buf(uint8_t* buf, uint8_t led,
                             uint8_t r, uint8_t g, uint8_t b)
{
  if (led >= _led_strip_len) return;

  uint8_t* pixel = &buf[led * WS2812_BYTES_PER_LED];
  pixel[_r_offset] = r;
  pixel[_g_offset] = g;
  pixel[_b_offset] = b;
}

uint32_t ws2812_get_color_in_buf(const uint8_t* buf, uint8_t led)
{
  if (led >= _led_strip_len) return 0;

  const uint8_t* pixel = &buf[led * WS2812_BYTES_PER_LED];
  return (pixel[1] << 16) + (pixel[0] << 8) + pixel[2];
}

bool ws2812_get_state_in_buf(const uint8_t* buf, uint8_t led)
{
  if (led >= _led_strip_len) return false;

  const uint8_t* pixel = &buf[led * WS2812_BYTES_PER_LED];
  return pixel[0] || pixel[1] || pixel[2];
}

bool ws2812_is_busy(const stm32_pulse_timer_t* tim)
{
#if defined(STM32H5) || defined(STM32H7RS)
  return LL_DMA_IsEnabledChannel(tim->DMAx, tim->DMA_Stream);
#else
  return LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream);
#endif
}

void ws2812_update(const stm32_pulse_timer_t* tim)
{
  WS2812_DBG_HIGH;
  if (!stm32_pulse_if_not_running_disable(tim)) return;

  // Build the whole frame up front, trailing slots left at 0
  _fill_pulses(_led_colors, _led_dma_buffer,
               _led_strip_len * WS2812_BYTES_PER_LED);
  memset(&_led_dma_buffer[_led_strip_len * WS2812_SLOTS_PER_LED * pulse_inc],
         0, WS2812_TRAIL_SLOTS * sizeof(led_timer_value_t) * pulse_inc);

  stm32_dma_clear_flags(tim->DMAx, tim->DMA_Stream);
#if defined(STM32H5) || defined(STM32H7RS)
  _led_setup_dma(tim);
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableChannel(tim->DMAx, tim->DMA_Stream);
#else
  // NDTR and the address are not reloaded by enabling the stream again
  LL_DMA_SetMemoryAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)_led_dma_buffer);
  LL_DMA_SetDataLength(tim->DMAx, tim->DMA_Stream, _frame_slots);

  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);
#endif

  LL_TIM_SetCounter(tim->TIMx, 0);
  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_CC_EnableChannel(tim->TIMx, tim->TIM_Channel);
  LL_TIM_EnableCounter(tim->TIMx);

  WS2812_DBG_LOW;
}
