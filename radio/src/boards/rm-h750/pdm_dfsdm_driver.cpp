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

#include "board.h"

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_dma.h"
#include "stm32_hal_ll.h"

#if defined(PDM_DFSDM_FILTER)

// CKOUT = dfsdm_ker_ck / (CKOUTDIV + 1)
static constexpr uint32_t PDM_CKOUT_DIV = PDM_DFSDM_KER_FREQ / PDM_CLOCK_FREQ;

static_assert(PDM_CKOUT_DIV * PDM_CLOCK_FREQ == PDM_DFSDM_KER_FREQ,
              "PDM_CLOCK_FREQ must divide the DFSDM kernel clock exactly");
static_assert(PDM_CKOUT_DIV >= 2 && PDM_CKOUT_DIV <= 256,
              "CKOUTDIV out of range");

static constexpr uint32_t PDM_PCM_RATE = PDM_CLOCK_FREQ / PDM_DFSDM_FOSR;

static_assert(PDM_PCM_RATE * PDM_DFSDM_FOSR == PDM_CLOCK_FREQ,
              "PDM_DFSDM_FOSR must divide PDM_CLOCK_FREQ exactly");
static_assert(PDM_PCM_RATE == 16000,
              "PdmWavRecorder::DST_RATE expects 16 kHz PCM");

// Sinc3 full scale for a 1-bit input; must fit the 24-bit RDATA field.
static constexpr uint32_t PDM_FULL_SCALE =
    (uint32_t)PDM_DFSDM_FOSR * PDM_DFSDM_FOSR * PDM_DFSDM_FOSR;

static_assert(PDM_FULL_SCALE <= 0x7FFFFFU, "Sinc3 output saturates RDATA");

// Maps full swing onto int16, at the software driver's loudness.
static constexpr int PDM_SCALE_SHIFT =
    (31 - __builtin_clz((unsigned)PDM_FULL_SCALE)) - 14;

static_assert(PDM_SCALE_SHIFT >= 0, "Oversampling too small to fit PCM range");

// 512 samples = 32 ms of slack for audio-task stalls.
static constexpr uint32_t PDM_RING_SAMPLES = 512;
static int32_t pdmRingBuf[PDM_RING_SAMPLES] __DMA_NO_CACHE;
static uint32_t pdmRingReadPos = 0;

// 8 ms per burst, and no more than PdmWavRecorder::PCM_MAX.
static constexpr uint32_t PDM_BURST_SAMPLES = 128;
static int32_t pdmBurstBuf[PDM_BURST_SAMPLES];

static uint8_t lastSoundLevel = 0;
static bool pdmRunning = false;

static inline int16_t pdmScale(int32_t v)
{
  int32_t s = (v >> PDM_SCALE_SHIFT) << PDM_POST_GAIN_SHIFT;
  if (s > 32767) s = 32767;
  if (s < -32768) s = -32768;
  return (int16_t)s;
}

void pdmStart()
{
  if (pdmRunning) return;

  pdmRingReadPos = 0;
  lastSoundLevel = 0;

  gpio_init_af(PDM_CLOCK, PDM_CLOCK_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(PDM_DATA, PDM_DATA_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN);
  (void)READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN);

  DFSDM_Filter_TypeDef* filter = PDM_DFSDM_FILTER;
  DFSDM_Channel_TypeDef* channel = PDM_DFSDM_CHANNEL;

  // Global bits live in channel 0; CKOUTDIV only takes while DFSDMEN is clear.
  DFSDM1_Channel0->CHCFGR1 = ((PDM_CKOUT_DIV - 1U) << DFSDM_CHCFGR1_CKOUTDIV_Pos);
  SET_BIT(DFSDM1_Channel0->CHCFGR1, DFSDM_CHCFGR1_DFSDMEN);

  channel->CHCFGR2 = 0;
  MODIFY_REG(channel->CHCFGR1,
             DFSDM_CHCFGR1_SPICKSEL | DFSDM_CHCFGR1_SITP |
                 DFSDM_CHCFGR1_CHINSEL | DFSDM_CHCFGR1_DATMPX |
                 DFSDM_CHCFGR1_DATPACK,
             (1U << DFSDM_CHCFGR1_SPICKSEL_Pos)  // internal CKOUT
                 | (PDM_DFSDM_SITP << DFSDM_CHCFGR1_SITP_Pos));
  SET_BIT(channel->CHCFGR1, DFSDM_CHCFGR1_CHEN);

  filter->FLTFCR = (3U << DFSDM_FLTFCR_FORD_Pos)   // Sinc3
                 | ((PDM_DFSDM_FOSR - 1U) << DFSDM_FLTFCR_FOSR_Pos);

  filter->FLTCR2 = 0;
  filter->FLTCR1 = (PDM_DFSDM_CHANNEL_ID << DFSDM_FLTCR1_RCH_Pos)
                 | DFSDM_FLTCR1_FAST
                 | DFSDM_FLTCR1_RCONT
                 | DFSDM_FLTCR1_RDMAEN;

  stm32_dma_enable_clock(PDM_DFSDM_DMA);

  LL_DMA_DeInit(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.PeriphRequest             = PDM_DFSDM_DMA_REQUEST;
  dmaInit.Mode                      = LL_DMA_MODE_CIRCULAR;
  dmaInit.Direction                 = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dmaInit.PeriphOrM2MSrcAddress     = (uintptr_t)&filter->FLTRDATAR;
  dmaInit.PeriphOrM2MSrcIncMode     = LL_DMA_PERIPH_NOINCREMENT;
  dmaInit.PeriphOrM2MSrcDataSize    = LL_DMA_PDATAALIGN_WORD;
  dmaInit.MemoryOrM2MDstAddress     = (uintptr_t)pdmRingBuf;
  dmaInit.MemoryOrM2MDstIncMode     = LL_DMA_MEMORY_INCREMENT;
  dmaInit.MemoryOrM2MDstDataSize    = LL_DMA_MDATAALIGN_WORD;
  dmaInit.NbData                    = PDM_RING_SAMPLES;
  dmaInit.Priority                  = LL_DMA_PRIORITY_HIGH;
  LL_DMA_Init(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM, &dmaInit);

  LL_DMA_EnableStream(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM);

  SET_BIT(filter->FLTCR1, DFSDM_FLTCR1_DFEN);
  SET_BIT(filter->FLTCR1, DFSDM_FLTCR1_RSWSTART);

  pdmRunning = true;
}

void pdmStop()
{
  if (!pdmRunning) return;

  DFSDM_Filter_TypeDef* filter = PDM_DFSDM_FILTER;

  // Drop RCONT first so the filter stops after the sample in flight.
  CLEAR_BIT(filter->FLTCR1, DFSDM_FLTCR1_RCONT);
  CLEAR_BIT(filter->FLTCR1, DFSDM_FLTCR1_DFEN | DFSDM_FLTCR1_RDMAEN);

  LL_DMA_DisableStream(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM);
  while (LL_DMA_IsEnabledStream(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM)) {}

  CLEAR_BIT(PDM_DFSDM_CHANNEL->CHCFGR1, DFSDM_CHCFGR1_CHEN);

  // Clearing DFSDMEN stops CKOUT, which puts the mic in low-power mode.
  CLEAR_BIT(DFSDM1_Channel0->CHCFGR1, DFSDM_CHCFGR1_DFSDMEN);

  pdmRunning = false;
}

static uint32_t pdmRingAvailable()
{
  const uint32_t ndtr =
      LL_DMA_GetDataLength(PDM_DFSDM_DMA, PDM_DFSDM_DMA_STREAM);
  const uint32_t writePos = PDM_RING_SAMPLES - ndtr;
  if (writePos >= pdmRingReadPos) return writePos - pdmRingReadPos;
  return PDM_RING_SAMPLES - pdmRingReadPos + writePos;
}

bool pdmCapture()
{
  uint32_t avail = pdmRingAvailable();
  if (avail < PDM_BURST_SAMPLES) return false;

  // Skip ahead if DMA is about to overwrite unread samples.
  if (avail > PDM_RING_SAMPLES - PDM_BURST_SAMPLES) {
    const uint32_t target = PDM_BURST_SAMPLES + PDM_BURST_SAMPLES / 2;
    pdmRingReadPos = (pdmRingReadPos + (avail - target)) % PDM_RING_SAMPLES;
  }

  uint32_t src = pdmRingReadPos;
  for (uint32_t i = 0; i < PDM_BURST_SAMPLES; ++i) {
    // RDATA sits in bits [31:8], sign-extended by the arithmetic shift.
    pdmBurstBuf[i] = (int32_t)pdmRingBuf[src] >> 8;
    if (++src >= PDM_RING_SAMPLES) src = 0;
  }

  pdmRingReadPos = src;
  return true;
}

bool pdmUpdateSoundLevel()
{
  if (!pdmCapture()) return false;

  int32_t peak = 0;
  for (uint32_t i = 0; i < PDM_BURST_SAMPLES; ++i) {
    int32_t v = pdmBurstBuf[i];
    if (v < 0) v = -v;
    if (v > peak) peak = v;
  }

  uint32_t level = (uint32_t)pdmScale(peak) * 100U / 32767U;
  if (level > 100U) level = 100U;
  lastSoundLevel = (uint8_t)level;
  return true;
}

uint8_t pdmGetSoundLevel()
{
  return lastSoundLevel;
}

uint32_t pdmConvertToPCM(int16_t* pcm, uint32_t max)
{
  if (!pcm || max == 0) return 0;

  const uint32_t n = max < PDM_BURST_SAMPLES ? max : PDM_BURST_SAMPLES;
  for (uint32_t i = 0; i < n; ++i) {
    pcm[i] = pdmScale(pdmBurstBuf[i]);
  }
  return n;
}

#endif // PDM_DFSDM_FILTER
