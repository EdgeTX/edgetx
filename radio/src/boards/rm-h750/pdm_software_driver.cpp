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

#if defined(PDM_CAPTURE_DMA)

static constexpr uint32_t PDM_SAI_MCKDIV =
    PDM_SAI_KER_FREQ / PDM_CLOCK_FREQ;

static_assert(PDM_SAI_MCKDIV >= 1 && PDM_SAI_MCKDIV <= 63,
              "PDM_SAI MCKDIV out of range for SAI_xCR1_MCKDIV (6 bits)");

// Burst size: ~6 ms of PDM at 1.6 MHz (multiple of 32 required for word packing).
static constexpr uint32_t PDM_BURST_BITS  = 10080;
static constexpr uint32_t PDM_BURST_WORDS = (PDM_BURST_BITS + 31) / 32;

static uint32_t pdmBurstBuf[PDM_BURST_WORDS];
static uint8_t  lastSoundLevel = 0;

// 40000 bytes at 1.6 MHz = ~25 ms ring; gives ~21 ms slack for audio-task stalls.
static constexpr uint32_t PDM_RING_BYTES = 40000;
static uint8_t  pdmRingBuf[PDM_RING_BYTES] __DMA_NO_CACHE;
static uint32_t pdmRingReadPos = 0;

// CIC integrator + comb state (used by pdmConvertToPCM, defined further down).
// Declared here so pdmStart() can zero them at session boundaries — without a
// reset the leftover integrator state from the previous session would emit a
// DC transient on the first decoded burst.
static int32_t cic_i1 = 0, cic_i2 = 0, cic_i3 = 0;
static int32_t cic_p1 = 0, cic_p2 = 0, cic_p3 = 0;
static uint32_t cic_bitsSeen = 0;

static bool pdmRunning = false;

void pdmStart()
{
  if (pdmRunning) return;

  // Per-session state reset. Must happen before the DMA starts producing data
  // so the audio-task path never sees a half-reset filter.
  cic_i1 = cic_i2 = cic_i3 = 0;
  cic_p1 = cic_p2 = cic_p3 = 0;
  cic_bitsSeen = 0;
  pdmRingReadPos = 0;
  lastSoundLevel = 0;

  gpio_init_af(PDM_CLOCK, PDM_CLOCK_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init(PDM_DATA, GPIO_IN, GPIO_PIN_SPEED_VERY_HIGH);

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN);
  (void)READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN);

  SAI_Block_TypeDef* block = PDM_SAI_BLOCK;

  CLEAR_BIT(block->CR1, SAI_xCR1_SAIEN);
  while (READ_BIT(block->CR1, SAI_xCR1_SAIEN)) {}

  block->CR1 = (0U << SAI_xCR1_MODE_Pos)       // Master TX
             | (0U << SAI_xCR1_PRTCFG_Pos)     // Free protocol
             | (4U << SAI_xCR1_DS_Pos)         // 16-bit data
             | SAI_xCR1_NODIV                  // BCLK = ker_ck / MCKDIV
             | (PDM_SAI_MCKDIV << SAI_xCR1_MCKDIV_Pos);

  block->CR2 = (1U << SAI_xCR2_FTH_Pos);       // FIFO threshold = 1/4 full

  block->FRCR = (15U << 0)                      // FRL = 15 (16-bit frame)
              | (7U  << 8);                     // FSALL = 7
  block->SLOTR = (0U << 8)                      // NBSLOT = 0 -> 1 slot
               | (1U << 16);                    // SLOTEN slot 0

  // Prime the FIFO so the block starts clocking immediately (no underrun).
  for (int i = 0; i < 4; ++i) block->DR = 0U;

  SET_BIT(block->CR1, SAI_xCR1_SAIEN);

  // TIM15 is free because FLYSKY_GIMBAL is OFF.
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN);
  (void)READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN);

  stm32_dma_enable_clock(PDM_CAPTURE_DMA);

  PDM_CAPTURE_TIMER->CR1 = 0;
  PDM_CAPTURE_TIMER->PSC = 0;
  PDM_CAPTURE_TIMER->ARR = (PDM_CAPTURE_TIMER_FREQ / PDM_CLOCK_FREQ) - 1U;
  PDM_CAPTURE_TIMER->CNT = 0;
  PDM_CAPTURE_TIMER->EGR = TIM_EGR_UG;   // load PSC/ARR
  PDM_CAPTURE_TIMER->SR  = 0;            // clear update flag
  PDM_CAPTURE_TIMER->DIER = TIM_DIER_UDE; // fire DMA request on every UEV

  LL_DMA_DeInit(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.PeriphRequest             = PDM_CAPTURE_DMA_REQUEST;
  dmaInit.Mode                      = LL_DMA_MODE_CIRCULAR;
  dmaInit.Direction                 = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dmaInit.PeriphOrM2MSrcAddress     = (uintptr_t)&PDM_DATA_GPIO_PORT->IDR;
  dmaInit.PeriphOrM2MSrcIncMode     = LL_DMA_PERIPH_NOINCREMENT;
  dmaInit.PeriphOrM2MSrcDataSize    = LL_DMA_PDATAALIGN_BYTE;
  dmaInit.MemoryOrM2MDstAddress     = (uintptr_t)pdmRingBuf;
  dmaInit.MemoryOrM2MDstIncMode     = LL_DMA_MEMORY_INCREMENT;
  dmaInit.MemoryOrM2MDstDataSize    = LL_DMA_MDATAALIGN_BYTE;
  dmaInit.NbData                    = PDM_RING_BYTES;
  dmaInit.Priority                  = LL_DMA_PRIORITY_HIGH;
  LL_DMA_Init(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM, &dmaInit);

  LL_DMA_EnableStream(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM);

  PDM_CAPTURE_TIMER->CR1 |= TIM_CR1_CEN;

  pdmRunning = true;
}

void pdmStop()
{
  if (!pdmRunning) return;

  // Stop DMA pacing first so no further requests fire.
  PDM_CAPTURE_TIMER->CR1 &= ~TIM_CR1_CEN;
  PDM_CAPTURE_TIMER->DIER = 0;

  // Disable the DMA stream and wait for it to actually stop. A subsequent
  // pdmStart() calls LL_DMA_DeInit which expects the stream idle.
  LL_DMA_DisableStream(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM);
  while (LL_DMA_IsEnabledStream(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM)) {}

  // Disable the SAI block — this stops the PDM clock to the mic, putting it
  // into low-power mode.
  SAI_Block_TypeDef* block = PDM_SAI_BLOCK;
  CLEAR_BIT(block->CR1, SAI_xCR1_SAIEN);
  while (READ_BIT(block->CR1, SAI_xCR1_SAIEN)) {}

  pdmRunning = false;
}

static uint32_t pdmRingAvailable()
{
  const uint32_t ndtr =
      LL_DMA_GetDataLength(PDM_CAPTURE_DMA, PDM_CAPTURE_DMA_STREAM);
  const uint32_t writePos = PDM_RING_BYTES - ndtr;
  if (writePos >= pdmRingReadPos) return writePos - pdmRingReadPos;
  return PDM_RING_BYTES - pdmRingReadPos + writePos;
}

bool pdmCapture()
{
  uint32_t avail = pdmRingAvailable();
  if (avail < PDM_BURST_BITS) return false;

  // Skip ahead if DMA is about to overwrite unread data.
  if (avail > PDM_RING_BYTES - PDM_BURST_BITS) {
    const uint32_t target_avail = PDM_BURST_BITS + PDM_BURST_BITS / 2;
    const uint32_t skip = avail - target_avail;
    pdmRingReadPos = (pdmRingReadPos + skip) % PDM_RING_BYTES;
  }

  static_assert(PDM_BURST_BITS % 32U == 0U,
                "PDM_BURST_BITS must be a multiple of 32");

  uint32_t src = pdmRingReadPos;
  for (uint32_t w = 0; w < PDM_BURST_WORDS; ++w) {
    uint32_t acc = 0;
    for (uint32_t b = 0; b < 32; ++b) {
      const uint8_t s = pdmRingBuf[src];
      if (++src >= PDM_RING_BYTES) src = 0;
      acc = (acc << 1) | ((s >> PDM_DATA_GPIO_PIN) & 1U);
    }
    pdmBurstBuf[w] = acc;
  }

  pdmRingReadPos = src;
  return true;
}

bool pdmUpdateSoundLevel()
{
  if (!pdmCapture()) return false;

  // Ones-density of the PDM bitstream approximates signal amplitude.
  uint32_t ones = 0;
  for (uint32_t w = 0; w < PDM_BURST_WORDS; ++w) {
    ones += __builtin_popcount(pdmBurstBuf[w]);
  }

  const uint32_t total = PDM_BURST_WORDS * 32U;
  const int32_t  mid   = (int32_t)(total / 2U);
  int32_t dev = (int32_t)ones - mid;
  if (dev < 0) dev = -dev;

  uint32_t level = (uint32_t)dev * 200U / total;
  if (level > 100U) level = 100U;
  lastSoundLevel = (uint8_t)level;
  return true;
}

uint8_t pdmGetSoundLevel()
{
  return lastSoundLevel;
}

// ---------------------------------------------------------------------------
// PDM -> PCM conversion: 3rd-order CIC decimator (R = PDM_PCM_DECIMATION).
//
// Structure (Hogenauer):
//   integrator ×3 at 1 MHz  ->  down-sample by R  ->  comb ×3 at 1 MHz / R
//
// Integrators run on 1-bit input (0 or 1), so for R=64 and N=3 the worst-case
// magnitude that has to be representable is R^N = 262 144 — fits easily in
// int32_t. 2's-complement wrap-around through the integrators is intentional
// and cancels in the comb stage.
//
// DC gain of the filter is R^N. Silence (50/50 bitstream) therefore sits at
// R^N / 2, and the useful signal swing is ±R^N / 2. We subtract the DC offset
// and scale so that full swing maps to the int16 range.
// ---------------------------------------------------------------------------

// PDM_POST_GAIN_SHIFT is defined in pdm_software_driver.h so trimSilence
// can scale its threshold from the same value.

// CIC integrator + comb state is declared near the top of the file (so
// pdmStart() can reset it on session boundaries).

uint32_t pdmConvertToPCM(int16_t* pcm, uint32_t max)
{
  if (!pcm || max == 0) return 0;

  constexpr uint32_t R  = PDM_PCM_DECIMATION;
  constexpr int32_t  G  = (int32_t)(R * R * R);
  constexpr int32_t  DC = G / 2;

  // floor_log2(DC) - 14: ensures (c3 - DC) >> SCALE_SHIFT fits in int16.
  // Works for any R, not just powers of two.
  constexpr int SCALE_SHIFT = (31 - __builtin_clz((unsigned)(DC))) - 14;
  static_assert(SCALE_SHIFT >= 0, "Decimation too small to fit PCM range");

  uint32_t produced = 0;
  const uint32_t totalBits = PDM_BURST_WORDS * 32U;

  for (uint32_t idx = 0; idx < totalBits && produced < max; ++idx) {
    const uint32_t word = pdmBurstBuf[idx >> 5];
    const int32_t  bit  = (int32_t)((word >> (31U - (idx & 31U))) & 1U);

    cic_i1 += bit;
    cic_i2 += cic_i1;
    cic_i3 += cic_i2;

    if (++cic_bitsSeen == R) {
      cic_bitsSeen = 0;

      const int32_t c1 = cic_i3 - cic_p1; cic_p1 = cic_i3;
      const int32_t c2 = c1     - cic_p2; cic_p2 = c1;
      const int32_t c3 = c2     - cic_p3; cic_p3 = c2;

      int32_t s = (c3 - DC) >> SCALE_SHIFT;
      s <<= PDM_POST_GAIN_SHIFT;
      if (s >  32767) s =  32767;
      if (s < -32768) s = -32768;
      pcm[produced++] = (int16_t)s;
    }
  }

  return produced;
}

#endif // PDM_CAPTURE_DMA
