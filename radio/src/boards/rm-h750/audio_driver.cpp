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

#include "stm32_hal_ll.h"
#include "timers_driver.h"

#include "audio.h"
#include "debug.h"

#define DEFAULT_DAC_VOL -15 // -7dB
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
// TX15 pinout
//
// Pin             MCU       TAS2505
// ---------------------------------
// PC6 (I2S2_MCK): MCLK  <->  MCLK
// PI0 (I2S2_WS):  WCLK  <->  WCLK
// PI1 (I2S2_CK):  BCLK  <->  BCLK
// PI3 (I2S2_SDO): DOUT  <->  DIN
// PC2 (I2S2_SDI): DIN   <->  DOUT


const stm32_i2s_t _i2s_dev = {
    .SPIx = AUDIO_SPI,
    .MCK = {GPIO_PIN(GPIOC, 6), GPIO_AF5},
    .WS = {GPIO_PIN(GPIOI, 0), GPIO_AF5},
    .CK = {GPIO_PIN(GPIOI, 1), GPIO_AF5},
    .SDO = {GPIO_PIN(GPIOI, 3), GPIO_AF5},
    .SDI = {GPIO_PIN(GPIOI, 2), GPIO_AF5},

    .DMA = I2S_DMA,
    .DMA_Stream = I2S_DMA_Stream,
    .DMA_PeriphRequest = I2S_DMA_Stream_Request,
};

// 16 bit, 2 channels
#define DMA_BUFFER_HALF_LEN AUDIO_BUFFER_SIZE

// 2 buffers
#define DMA_BUFFER_LEN (DMA_BUFFER_HALF_LEN * 2)

static int32_t _dma_buffer[DMA_BUFFER_LEN] __DMA_NO_CACHE;

static void enable_dma_irqs()
{
  LL_DMA_EnableIT_HT(I2S_DMA, I2S_DMA_Stream);
  LL_DMA_EnableIT_TC(I2S_DMA, I2S_DMA_Stream);

  NVIC_SetPriority(I2S_DMA_Stream_IRQn, 7);
  NVIC_EnableIRQ(I2S_DMA_Stream_IRQn);
}

int audioInit()
{
  gpio_init(AUDIO_HP_DETECT_PIN, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);

  memset(_dma_buffer, 0, sizeof(_dma_buffer));

  if (tas2505_probe(&_tas2505, AUDIO_I2C, 0x30 >> 1) < 0) {
    return -1;
  }

  if (stm32_i2s_init(&_i2s_dev)) {
    return -1;
  }

  enable_dma_irqs();
  return tas2505_init(&_tas2505);
}

static volatile uint32_t _dma_buffer_offset = 0;

void audioConsumeCurrentBuffer()
{
  if (!stm32_i2s_is_xfer_started(&_i2s_dev)) {

    auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (!buffer) return;

    unsigned idx = 0;
    for (; idx < buffer->size; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = (uint32_t)buffer->data[idx];
    }
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = 0;
    }
    audioQueue.buffersFifo.freeNextFilledBuffer();
    stm32_i2s_config_dma_stream(&_i2s_dev, _dma_buffer, DMA_BUFFER_LEN * 2);
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

  auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
  if (!buffer) {
    unsigned idx = 0;
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = 0;
    }
  } else {
    unsigned idx = 0;
    for (; idx < buffer->size; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = (uint16_t)buffer->data[idx];
    }
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = 0;
    }
    audioQueue.buffersFifo.freeNextFilledBuffer();
  }
}

bool audioHeadphoneDetect()
{
#if defined(KCX_BTAUDIO)
  return gpio_read(AUDIO_HP_DETECT_PIN) || gpio_read(BTAUDIO_LINKED_GPIO);
#else
  return gpio_read(AUDIO_HP_DETECT_PIN);
#endif
}

void audioSetVolume(uint8_t volume)
{
#if defined(KCX_BTAUDIO)
  // KCX need a volume boost
  if (btAudioLinked())
    volume = volume + (volume	>> 2);
#endif
  tas2505_set_volume(&_tas2505, volume * 9 / 10, audioHeadphoneDetect()); // TX15 HP cannot handle the full power of TAS2505
}

extern "C" void DMA1_Stream4_IRQHandler(void)
{
  if (stm32_dma_check_ht_flag(I2S_DMA, I2S_DMA_Stream)) {
    audio_update_dma_buffer(0);
  }

  if (stm32_dma_check_tc_flag(I2S_DMA, I2S_DMA_Stream)) {
    audio_update_dma_buffer(1);
  }
}
