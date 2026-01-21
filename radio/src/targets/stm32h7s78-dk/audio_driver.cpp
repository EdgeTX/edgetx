#include "audio.h"
#include "delays_driver.h"
#include "drivers/wm8904.h"

#include "hal/gpio.h"
#include "stm32_dma.h"
#include "stm32_i2s.h"
#include "stm32_i2c_driver.h"

#include "stm32_hal_ll.h"

#include "debug.h"
#include "timers_driver.h"

static wm8904_t _wm8904 = {0};

//
// STM32H7S78-DK pinout
//
// Pin             MCU       WM8904
// ---------------------------------
// PA3 (I2S2_MCK):  MCLK  <->  MCLK
// PA4 (I2S2_WS):   WCLK  <->  WCLK
// PA5 (I2S2_CK):   BCLK  <->  BCLK
// PB5 (I2S2_SDO):  DOUT  <->  DIN
// PG12 (I2S2_SDI): DIN   <->  DOUT

#define I2S_DMA                   GPDMA1
#define I2S_DMA_Stream            LL_DMA_CHANNEL_2
#define I2S_DMA_Stream_Request    LL_GPDMA1_REQUEST_SPI6_TX
#define I2S_DMA_Stream_IRQn       GPDMA1_Channel2_IRQn
#define I2S_DMA_Stream_IRQHandler GPDMA1_Channel2_IRQHandler

const stm32_i2s_t _i2s_dev = {
  .SPIx = AUDIO_SPI,
  .MCK = {GPIO_PIN(GPIOA, 3), GPIO_AF5},
  .WS = {GPIO_PIN(GPIOA, 4), GPIO_AF8},
  .CK = {GPIO_PIN(GPIOA, 5), GPIO_AF8},
  .SDO = {GPIO_PIN(GPIOB, 5), GPIO_AF8},
  .SDI = {GPIO_PIN(GPIOG, 12), GPIO_AF5},

  .DMA = I2S_DMA,
  .DMA_Stream = I2S_DMA_Stream,
  .DMA_PeriphRequest = I2S_DMA_Stream_Request,
};

// 16 bit, 2 channels
#define DMA_BUFFER_HALF_LEN AUDIO_BUFFER_SIZE

// 2 buffers
#define DMA_BUFFER_LEN (DMA_BUFFER_HALF_LEN * 2)

static uint32_t _dma_buffer[DMA_BUFFER_LEN] __DMA_NO_CACHE;
static uintptr_t _ll_addr_reset __DMA_NO_CACHE;

static inline uint32_t _conv_sample(int16_t sample)
{
  return ((uint32_t)(uint16_t)sample << 16) | (uint16_t)sample;
}

#define PI 3.14159265358979323846f

static void generate_sine_wave(const uint32_t num_samples)
{
  const uint32_t sample_rate = 32000;
  const float amplitude = 0.8f;
  const float frequency = 440.0f;

   // Generate and write samples
    for (uint32_t i = 0; i < num_samples; i++) {
        float time = (float)i / sample_rate;
        // Calculate sample value (-32768 to 32767)
        int16_t sample = (int16_t)(amplitude * 32767.0f * 
            sinf(2.0f * PI * frequency * time));

        // write into dma buffer
        _dma_buffer[i] = _conv_sample(sample);
    }
}

static int enable_dma_irqs()
{
  // enable DMA channel IRQ
  LL_DMA_EnableIT_HT(I2S_DMA, I2S_DMA_Stream);
  LL_DMA_EnableIT_TC(I2S_DMA, I2S_DMA_Stream);

  NVIC_SetPriority(GPDMA1_Channel2_IRQn, 7);
  NVIC_EnableIRQ(GPDMA1_Channel2_IRQn);

  return 0;
}

int audioInit()
{
  memset(_dma_buffer, 0, sizeof(_dma_buffer));
  
  if (wm8904_probe(&_wm8904, 0, 0x34 >> 1) < 0) {
    return -1;
  }

  if (stm32_i2s_init(&_i2s_dev)) {
    return -1;
  }

  if (wm8904_init(&_wm8904) < 0) {
    return -1;
  }

  enable_dma_irqs();

  return 0;
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

  for (;idx < DMA_BUFFER_HALF_LEN; idx++) {
    _dma_buffer[offset++] = 0;
  }
}

void audioConsumeCurrentBuffer()
{
  if (!stm32_i2s_is_xfer_started(&_i2s_dev)) {

    auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (!buffer) return;

    copy_into_dma_buffer(buffer);

    // basic DMA configuration
    stm32_i2s_config_dma_stream(&_i2s_dev, _dma_buffer, DMA_BUFFER_LEN * 2);

    // configure source address reset (circular buffer)
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

extern "C" void GPDMA1_Channel2_IRQHandler()
{
  if (stm32_dma_check_ht_flag(GPDMA1, LL_DMA_CHANNEL_2)) {
    audio_update_dma_buffer(0);
  }

  if (stm32_dma_check_tc_flag(GPDMA1, LL_DMA_CHANNEL_2)) {
    audio_update_dma_buffer(1);
  }
}

