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
static uint8_t* _led_colors = nullptr;

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
static led_timer_value_t _led_dma_buffer[WS2821_DMA_BUFFER_LEN * 2] __DMA_NO_CACHE;
#else
static led_timer_value_t _led_dma_buffer[WS2821_DMA_BUFFER_LEN] __DMA_NO_CACHE;
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
#if defined(STM32H7RS) || defined(STM32H5)
    LL_DMA_DisableChannel(tim->DMAx, tim->DMA_Stream);
#else
    LL_DMA_DisableStream(tim->DMAx, tim->DMA_Stream);

    uint32_t timeout = 1000;
    while (LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream) && timeout--) 	{
      __NOP();  // Wait
    }
#endif
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

// GPDMA has no circular mode: emulate it with a single self-linking linked-list
// node (mem -> CCRx, triggered by the timer UPDATE request). HT/TC events fire
// per block iteration, driving the half/half buffer refill in the DMA ISR.
// Mirrors the linked-list setup used by stm32_usart_init_rx_dma.
static LL_DMA_LinkNodeTypeDef _led_dma_node;

#define WS2812_LL_NODE_UPDATE                                              \
  (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 |          \
   LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 |          \
   LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR)

static void _led_setup_circular_dma(const stm32_pulse_timer_t* tim)
{
  LL_DMA_DisableChannel(tim->DMAx, tim->DMA_Stream);

  const bool word = (pulse_inc == 2); // 32-bit timer -> 32-bit CCR writes
  const uint32_t blkBytes =
      WS2821_DMA_BUFFER_LEN * sizeof(led_timer_value_t) * pulse_inc;

  LL_DMA_InitNodeTypeDef nodeInit;
  LL_DMA_NodeStructInit(&nodeInit);

  nodeInit.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  nodeInit.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  nodeInit.DestByteExchange  = LL_DMA_DEST_BYTE_PRESERVE;
  nodeInit.DestBurstLength   = 1;
  nodeInit.DestIncMode       = LL_DMA_DEST_FIXED;            // CCRx fixed
  nodeInit.DestDataWidth     = word ? LL_DMA_DEST_DATAWIDTH_WORD
                                    : LL_DMA_DEST_DATAWIDTH_HALFWORD;
  nodeInit.SrcAllocatedPort  = LL_DMA_SRC_ALLOCATED_PORT1;
  nodeInit.SrcByteExchange   = LL_DMA_SRC_BYTE_PRESERVE;
  nodeInit.DataAlignment     = LL_DMA_DATA_ALIGN_ZEROPADD;
  nodeInit.SrcBurstLength    = 1;
  nodeInit.SrcIncMode        = LL_DMA_SRC_INCREMENT;         // walk the buffer
  nodeInit.SrcDataWidth      = word ? LL_DMA_SRC_DATAWIDTH_WORD
                                    : LL_DMA_SRC_DATAWIDTH_HALFWORD;
  nodeInit.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;     // HT/TC per block
  nodeInit.TriggerPolarity   = LL_DMA_TRIG_POLARITY_MASKED;
  nodeInit.BlkHWRequest      = LL_DMA_HWREQUEST_SINGLEBURST;
  nodeInit.Direction         = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  nodeInit.Request           = tim->DMA_Channel;             // TIMx_UP
  nodeInit.UpdateRegisters   = WS2812_LL_NODE_UPDATE;
  nodeInit.NodeType          = LL_DMA_GPDMA_LINEAR_NODE;
  nodeInit.SrcAddress        = (intptr_t)_led_dma_buffer;
  nodeInit.DestAddress       = (intptr_t)_led_cmp_reg(tim);
  nodeInit.BlkDataLength     = blkBytes;
  LL_DMA_CreateLinkNode(&nodeInit, &_led_dma_node);

  // Connect the node to itself to loop forever (circular emulation)
  LL_DMA_ConnectLinkNode(&_led_dma_node, LL_DMA_CLLR_OFFSET5,
                         &_led_dma_node, LL_DMA_CLLR_OFFSET5);

  LL_DMA_SetLinkedListBaseAddr(tim->DMAx, tim->DMA_Stream,
                               (intptr_t)&_led_dma_node);
  LL_DMA_ConfigLinkUpdate(tim->DMAx, tim->DMA_Stream, WS2812_LL_NODE_UPDATE,
                          (intptr_t)&_led_dma_node);

  LL_DMA_InitLinkedListTypeDef ll = {0};
  ll.Priority          = LL_DMA_HIGH_PRIORITY;
  ll.LinkStepMode      = LL_DMA_LSM_FULL_EXECUTION;
  ll.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT0;
  ll.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(tim->DMAx, tim->DMA_Stream, &ll);
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

#if defined(STM32H5) || defined(STM32H7RS)
  // GPDMA: circular transfer is (re)armed via the linked-list node in
  // ws2812_update(); nothing to pre-configure here.
#else
  // pulse driver uses DMA to ARR, but we need CCRx
  _led_set_dma_periph_addr(tim);

  LL_DMA_SetMode(tim->DMAx, tim->DMA_Stream, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetDataLength(tim->DMAx, tim->DMA_Stream, WS2821_DMA_BUFFER_LEN);
  LL_DMA_SetMemoryAddress(tim->DMAx, tim->DMA_Stream, (uint32_t)_led_dma_buffer);
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

  _led_colors = strip_colors;
  _led_strip_len = strip_len;
  memset(_led_colors, 0, strip_len * WS2812_BYTES_PER_LED);
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

  _r_offset = (type >> 4) & 0b11;
  _g_offset = (type >> 2) & 0b11;
  _b_offset = type & 0b11;

  _init_timer(timer);
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
#if defined(STM32H7RS) || defined(STM32H5)
  return LL_DMA_IsEnabledChannel(tim->DMAx, tim->DMA_Stream);
#else
  return LL_DMA_IsEnabledStream(tim->DMAx, tim->DMA_Stream);
#endif
}

void ws2812_update(const stm32_pulse_timer_t* tim)
{
  WS2812_DBG_HIGH;
  if (!stm32_pulse_if_not_running_disable(tim)) return;

  _led_seq_cnt = 0;
  memset(_led_dma_buffer, 0, sizeof(_led_dma_buffer));

#if defined(STM32H5) || defined(STM32H7RS)
  // (re)arm the self-linking circular linked-list and start the channel
  _led_setup_circular_dma(tim);
  LL_DMA_EnableIT_HT(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableChannel(tim->DMAx, tim->DMA_Stream);
#else
  LL_DMA_EnableIT_HT(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableIT_TC(tim->DMAx, tim->DMA_Stream);
  LL_DMA_EnableStream(tim->DMAx, tim->DMA_Stream);
#endif

  LL_TIM_EnableDMAReq_UPDATE(tim->TIMx);
  LL_TIM_CC_EnableChannel(tim->TIMx, tim->TIM_Channel);
  LL_TIM_EnableCounter(tim->TIMx);

  WS2812_DBG_LOW;
}
