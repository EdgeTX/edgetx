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

#include "drivers/tas2505.h"

#include "stm32_dma.h"
#include "stm32_i2s.h"
#include "stm32_i2c_driver.h"
#include "stm32_gpio.h"

#include "stm32_hal_ll.h"
#include "hal/gpio.h"

#include "delays_driver.h"
#include "audio.h"
#include "debug.h"

// TAS2505 default codec setup (based on the rm-h750 / TX15 tuning)
#define DEFAULT_DAC_VOL 0 // 0dB; h5test speaker is quiet (rm-h750 uses -7.5dB)
#define DEFAULT_HP_VOL 20
#define DEFAULT_SPK_VOL 40

static tas2505_t _tas2505 = {
  .ndac = 2,
  .mdac = 1,
  .dosr = 256,
  .dac_vol = DEFAULT_DAC_VOL,
  .hp_drv = 0,
  .hp_vol = DEFAULT_HP_VOL,
  .spk_drv = TAS2505_SPK_DRV_32_DB,
  .spk_vol = DEFAULT_SPK_VOL,
};

//
// h5test (N14) pinout - I2S1 (SPI1), all AF5
//
// Pin              MCU        TAS2505
// -----------------------------------
// PC4  (I2S1_MCK):  MCLK  <->  MCLK
// PA15 (I2S1_WS):   WCLK  <->  WCLK
// PB3  (I2S1_CK):   BCLK  <->  BCLK
// PB5  (I2S1_SDO):  DOUT  <->  DIN
// PB4  (I2S1_SDI):  DIN   <->  DOUT
//
const stm32_i2s_t _i2s_dev = {
    .SPIx = AUDIO_SPI,
    .MCK = {AUDIO_I2S_MCK_GPIO, AUDIO_I2S_GPIO_AF},
    .WS  = {AUDIO_I2S_WS_GPIO,  AUDIO_I2S_GPIO_AF},
    .CK  = {AUDIO_I2S_CK_GPIO,  AUDIO_I2S_GPIO_AF},
    .SDO = {AUDIO_I2S_SDO_GPIO, AUDIO_I2S_GPIO_AF},
    .SDI = {AUDIO_I2S_SDI_GPIO, AUDIO_I2S_GPIO_AF},

    .DMA = I2S_DMA,
    .DMA_Stream = I2S_DMA_Stream,
    .DMA_PeriphRequest = I2S_DMA_Stream_Request,
};

// 16 bit, 2 channels (each mono sample is duplicated to L+R)
#define DMA_BUFFER_HALF_LEN AUDIO_BUFFER_SIZE

// 2 buffers (ping-pong)
#define DMA_BUFFER_LEN (DMA_BUFFER_HALF_LEN * 2)

static uint32_t _dma_buffer[DMA_BUFFER_LEN] __DMA_NO_CACHE;
static uintptr_t _ll_addr_reset __DMA_NO_CACHE;

static inline uint32_t _conv_sample(int16_t sample)
{
  // pack the mono sample into both I2S channels (L in high half, R in low half)
  return ((uint32_t)(uint16_t)sample << 16) | (uint16_t)sample;
}

static void enable_dma_irqs()
{
  LL_DMA_EnableIT_HT(I2S_DMA, I2S_DMA_Stream);
  LL_DMA_EnableIT_TC(I2S_DMA, I2S_DMA_Stream);

  NVIC_SetPriority(I2S_DMA_Stream_IRQn, 7);
  NVIC_EnableIRQ(I2S_DMA_Stream_IRQn);
}

static void audio_codec_reset()
{
  // TAS2505 reset: hold low, then release high before any I2C traffic
  // (see SLAU472 fig 4.2)
  gpio_init(AUDIO_RESET_PIN, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(AUDIO_RESET_PIN);
  delay_ms(1);
  gpio_set(AUDIO_RESET_PIN);
  delay_ms(1);
}

// Set only once audioInit() fully succeeds. Guards the runtime path from
// touching SPI1/I2S1 (unclocked -> HardFault) if init bailed early.
static bool _audio_ready = false;

void audioInit()
{
  memset(_dma_buffer, 0, sizeof(_dma_buffer));

  audio_codec_reset();

  if (tas2505_probe(&_tas2505, AUDIO_I2C, 0x30 >> 1) < 0) {
    TRACE("TAS2505 probe failed");
    return;
  }

  if (stm32_i2s_init(&_i2s_dev)) {
    TRACE("I2S init failed");
    return;
  }

  if (tas2505_init(&_tas2505) < 0) {
    TRACE("TAS2505 init failed");
    return;
  }

  enable_dma_irqs();
  _audio_ready = true;
}

static volatile uint32_t _dma_buffer_offset = 0;

static void copy_into_dma_buffer(const AudioBuffer* buffer)
{
  unsigned idx = 0;
  uint32_t offset = _dma_buffer_offset;

  if (buffer) {
    while (idx < buffer->size) {
      _dma_buffer[offset++] = _conv_sample(buffer->data[idx++]);
    }
    audioQueue.buffersFifo.freeNextFilledBuffer();
  }

  for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
    _dma_buffer[offset++] = 0;
  }
}

void audioConsumeCurrentBuffer()
{
  if (!_audio_ready) return;  // SPI1 unclocked if init bailed -> avoid HardFault

  if (!stm32_i2s_is_xfer_started(&_i2s_dev)) {

    auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (!buffer) return;

    copy_into_dma_buffer(buffer);

    // basic DMA configuration
    stm32_i2s_config_dma_stream(&_i2s_dev, _dma_buffer, DMA_BUFFER_LEN * 2);

    // configure source address reset (circular buffer via linked-list node)
    _ll_addr_reset = (uintptr_t)_dma_buffer;

    LL_DMA_SetLinkedListBaseAddr(_i2s_dev.DMA, _i2s_dev.DMA_Stream,
                                 (uintptr_t)&_ll_addr_reset & 0xFFFF0000);

    LL_DMA_ConfigLinkUpdate(_i2s_dev.DMA, _i2s_dev.DMA_Stream,
                            LL_DMA_UPDATE_CSAR,
                            (uintptr_t)&_ll_addr_reset & 0xFFFF);

    // and finally start it!
    stm32_i2s_start_dma_stream(&_i2s_dev);
  }
}

static inline uint32_t _calc_offset(uint8_t tc)
{
  return tc * DMA_BUFFER_HALF_LEN;
}

static void audio_update_dma_buffer(uint8_t tc)
{
  _dma_buffer_offset = _calc_offset(tc);
  copy_into_dma_buffer(audioQueue.buffersFifo.getNextFilledBuffer());
}

bool audioHeadphoneDetect()
{
  // No headphone-detect line on this hardware: always speaker mode.
  return false;
}

void audioSetVolume(uint8_t volume)
{
  tas2505_set_volume(&_tas2505, volume, audioHeadphoneDetect());
}

extern "C" void I2S_DMA_Stream_IRQHandler(void)
{
  if (stm32_dma_check_ht_flag(I2S_DMA, I2S_DMA_Stream)) {
    audio_update_dma_buffer(0);
  }

  if (stm32_dma_check_tc_flag(I2S_DMA, I2S_DMA_Stream)) {
    audio_update_dma_buffer(1);
  }
}
