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

#include "hal/gpio.h"
#include "hal/audio_driver.h"

#include "os/sleep.h"
#include "stm32_gpio.h"
#include "stm32_timer.h"
#include "stm32_dma.h"

#include "edgetx.h"

#if defined(AUDIO_MUTE_GPIO)

static inline void set_mute_pin(bool enabled)
{
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  gpio_write(AUDIO_MUTE_GPIO, enabled);
}

static inline bool get_mute_pin(void)
{
  bool enabled = gpio_read(AUDIO_MUTE_GPIO) ? 1 : 0;
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  return enabled;
}

void audioMute()
{
  if (!g_eeGeneral.audioMuteEnable) return;
#if defined(AUDIO_UNMUTE_DELAY)
  tmr10ms_t now = get_tmr10ms();
  if (!audioQueue.lastAudioPlayTime) {
    // we start the mute delay now
    audioQueue.lastAudioPlayTime = now;
  }
  else if (now - audioQueue.lastAudioPlayTime > AUDIO_MUTE_DELAY / 10) {
    // delay expired, we may mute
    set_mute_pin(true);
  }
#else
  // mute
  set_mute_pin(true);
#endif
}

void audioUnmute()
{
  if (isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
    set_mute_pin(true);
    return;
  }

#if defined(AUDIO_UNMUTE_DELAY)
  // if muted
  if (get_mute_pin()) {
    // ..un-mute
    set_mute_pin(false);
    sleep_ms(AUDIO_UNMUTE_DELAY);
  }
  // reset the mute delay
  audioQueue.lastAudioPlayTime = 0;
#else
  set_mute_pin(false);
#endif
}
#endif

// DAC output channel; a board on DAC1_OUT2 (PA5) sets AUDIO_DAC_CH 2 in hal.h
#ifndef AUDIO_DAC_CH
  #define AUDIO_DAC_CH 1
#endif

#define _AUDIO_DAC_CAT(a, b)  a##b
#define _AUDIO_DAC_XCAT(a, b) _AUDIO_DAC_CAT(a, b)

#define AUDIO_DAC_CHANNEL        _AUDIO_DAC_XCAT(LL_DAC_CHANNEL_, AUDIO_DAC_CH)
#define AUDIO_DAC_DHR12L         _AUDIO_DAC_XCAT(DHR12L, AUDIO_DAC_CH)
#define AUDIO_DAC_CR_EN          _AUDIO_DAC_XCAT(DAC_CR_EN, AUDIO_DAC_CH)
#define AUDIO_DAC_CR_TEN         _AUDIO_DAC_XCAT(DAC_CR_TEN, AUDIO_DAC_CH)
#define AUDIO_DAC_CR_DMAEN       _AUDIO_DAC_XCAT(DAC_CR_DMAEN, AUDIO_DAC_CH)
#define AUDIO_DAC_SR_DMAUDR      _AUDIO_DAC_XCAT(DAC_SR_DMAUDR, AUDIO_DAC_CH)
#define AUDIO_DAC_CLEAR_DMAUDR() _AUDIO_DAC_XCAT(LL_DAC_ClearFlag_DMAUDR, AUDIO_DAC_CH)(AUDIO_DAC)

// Channel-2 control/trigger bits sit 16 bits above the channel-1 ones
#if AUDIO_DAC_CH == 2
  #define AUDIO_DAC_TRIGGER (DAC_TRIGGER << 16)
#else
  #define AUDIO_DAC_TRIGGER DAC_TRIGGER
#endif

#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)

#define DAC_TRIGGER LL_DAC_TRIG_EXT_TIM6_TRGO

#if defined(STM32H5) || defined(STM32H7RS)
LL_DMA_LinkNodeTypeDef dacDmaLinkNode;

// Registers re-loaded from the linked-list node on every fetch. CTR3/CBR2 exist
// only on the 2D channels (ch6/7); on a linear channel their update bits are
// reserved and must stay 0, or the node fetch goes out of sync.
#define DAC_DMA_NODE_UPDATE_REGS                                     \
  (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 |    \
   LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR)
#endif

// 16 bit, 1 channels
#define DMA_BUFFER_HALF_LEN AUDIO_BUFFER_SIZE

// 2 buffers
#define DMA_BUFFER_LEN (DMA_BUFFER_HALF_LEN * 2)

static uint16_t _dma_buffer[DMA_BUFFER_LEN] __DMA_NO_CACHE;

static volatile uint32_t _dma_buffer_offset = 0;
static volatile uint8_t _empty_dma_halves = 0;

// Require sustained silence before stopping DMA to avoid start/stop thrashing
// when the producer briefly lags behind the consumer.
constexpr uint8_t DMA_EMPTY_HALVES_STOP_THRESHOLD = 6;

static inline uint32_t _calc_offset(uint8_t tc)
{
  return tc * DMA_BUFFER_HALF_LEN;
}

// return true if no more data available
static bool audio_update_dma_buffer(uint8_t tc)
{
  _dma_buffer_offset = _calc_offset(tc);

  auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
  if (!buffer) {
    unsigned idx = 0;
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = AUDIO_DATA_SILENCE;
    }
    return true;
  } else {
    unsigned idx = 0;
    for (; idx < buffer->size; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = (uint16_t)buffer->data[idx];
    }
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = AUDIO_DATA_SILENCE;
    }
    audioQueue.buffersFifo.freeNextFilledBuffer();
    return false;
  }
}

// _dma_buffer lives in a NOLOAD section, so it holds random SRAM contents at
// power-on. Prime it with silence so the DAC never clocks out garbage (heard
// as a crackle) on the very first transfer.
//
// This is a one-shot boot-time step: it must NOT run from dac_dma_init() on the
// GPDMA re-arm path, which happens *after* audioConsumeCurrentBuffer() has
// already loaded both halves from the FIFO.
static void dac_prime_silence()
{
  for (unsigned i = 0; i < DMA_BUFFER_LEN; i++)
    _dma_buffer[i] = AUDIO_DATA_SILENCE;
}

static void dac_dma_init()
{
  stm32_dma_enable_clock(AUDIO_DMA);

  LL_DMA_DeInit(AUDIO_DMA, AUDIO_DMA_Stream);


#if defined(STM32H7)
  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.Mode = LL_DMA_MODE_CIRCULAR;
  dmaInit.PeriphRequest = AUDIO_DMA_Channel;
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  dmaInit.PeriphOrM2MSrcAddress = LL_DAC_DMA_GetRegAddr(
      AUDIO_DAC, AUDIO_DAC_CHANNEL, LL_DAC_DMA_REG_DATA_12BITS_LEFT_ALIGNED);
  dmaInit.MemoryOrM2MDstAddress = (uintptr_t)_dma_buffer;
  dmaInit.NbData = DMA_BUFFER_LEN;

  LL_DMA_Init(AUDIO_DMA, AUDIO_DMA_Stream, &dmaInit);

  NVIC_EnableIRQ(AUDIO_DMA_Stream_IRQn);
  NVIC_SetPriority(AUDIO_DMA_Stream_IRQn, 7);

#elif defined(STM32H7RS) || defined(STM32H5)
  LL_DMA_InitNodeTypeDef nodeInit;
  LL_DMA_NodeStructInit(&nodeInit);

  nodeInit.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  nodeInit.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  nodeInit.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
  nodeInit.DestBurstLength = 1;
  nodeInit.DestIncMode = LL_DMA_DEST_FIXED;
  nodeInit.DestDataWidth = LL_DMA_DEST_DATAWIDTH_WORD;
  nodeInit.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
  nodeInit.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;
  nodeInit.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  nodeInit.SrcBurstLength = 1;
  nodeInit.SrcIncMode = LL_DMA_SRC_INCREMENT;
  nodeInit.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_HALFWORD;
  nodeInit.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  nodeInit.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  nodeInit.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  nodeInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  nodeInit.Request = AUDIO_DMA_REQUEST;
  nodeInit.UpdateRegisters = DAC_DMA_NODE_UPDATE_REGS;
  nodeInit.NodeType = LL_DMA_GPDMA_LINEAR_NODE;
  /* Additional settings */
  nodeInit.SrcAddress = (uintptr_t)_dma_buffer;
  nodeInit.DestAddress = LL_DAC_DMA_GetRegAddr(AUDIO_DAC, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_LEFT_ALIGNED);
  /* Size is always in bytes! Width is determined by source and destination numbers */
  nodeInit.BlkDataLength = DMA_BUFFER_LEN*2;
  LL_DMA_CreateLinkNode(&nodeInit, &dacDmaLinkNode);

  /* Connect node to next node = to itself to achieve circular mode with one configuration */
  LL_DMA_ConnectLinkNode(&dacDmaLinkNode, LL_DMA_CLLR_OFFSET5, &dacDmaLinkNode, LL_DMA_CLLR_OFFSET5);

  /*
   * Set first linked list address to DMA channel
   *
   * Set link update mechanism - DMA fetches first configuration from first node
   * on the start of DMA operation
   */
  LL_DMA_SetLinkedListBaseAddr(AUDIO_DMA, AUDIO_DMA_Stream, (intptr_t)&dacDmaLinkNode);
  LL_DMA_ConfigLinkUpdate(AUDIO_DMA, AUDIO_DMA_Stream, DAC_DMA_NODE_UPDATE_REGS,
			   (intptr_t)&dacDmaLinkNode);

  LL_DMA_InitLinkedListTypeDef DMA_InitLinkedListStruct = {0};
  /* Initialize linked list general setup for GPDMA CH0 - the way transfers are done */
  DMA_InitLinkedListStruct.Priority = LL_DMA_HIGH_PRIORITY;
  DMA_InitLinkedListStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitLinkedListStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitLinkedListStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(AUDIO_DMA, AUDIO_DMA_Stream, &DMA_InitLinkedListStruct);


  LL_DMA_EnableIT_HT(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_EnableIT_TC(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_EnableIT_USE(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_EnableIT_ULE(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_EnableIT_DTE(AUDIO_DMA, AUDIO_DMA_Stream);

  NVIC_EnableIRQ(AUDIO_DMA_Stream_IRQn);
  NVIC_SetPriority(AUDIO_DMA_Stream_IRQn, 7);
//  LL_DAC_EnableIT_DMAUDR1(AUDIO_DAC);
  //LL_DAC_EnableDMAReq(AUDIO_DAC, LL_DAC_CHANNEL_1);
#endif
}

static inline void dac_clear_dma_flags()
{
  // Drain stale half/complete flags so a new transfer starts from a clean state.
  stm32_dma_check_ht_flag(AUDIO_DMA, AUDIO_DMA_Stream);
  stm32_dma_check_tc_flag(AUDIO_DMA, AUDIO_DMA_Stream);
}

static void dac_close_dma_xfer()
{
  LL_DMA_DisableIT_TC(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_DisableIT_HT(AUDIO_DMA, AUDIO_DMA_Stream);
#if defined(STM32H5) || defined(STM32H7RS)
  // GPDMA: a running channel must be SUSPENDED (and the suspend must take
  // effect) before RESET is honoured. LL_DMA_DisableChannel() writes
  // SUSP|RESET in one go, which does NOT abort the self-linked circular
  // channel - it keeps cycling and the ISR refills silence forever.
  LL_DMA_SuspendChannel(AUDIO_DMA, AUDIO_DMA_Stream);
  uint32_t timeout = 10000;
  while (!LL_DMA_IsActiveFlag_SUSP(AUDIO_DMA, AUDIO_DMA_Stream) && --timeout) {
  }
  LL_DMA_ResetChannel(AUDIO_DMA, AUDIO_DMA_Stream);
  // drop any pending transfer flags so no stale IRQ re-arms the refill
  LL_DMA_ClearFlag_HT(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_ClearFlag_TC(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_ClearFlag_SUSP(AUDIO_DMA, AUDIO_DMA_Stream);

  // Disable DAC DMA to prevent underrun while DMA is stopped
  AUDIO_DAC->CR &= ~DAC_CR_DMAEN1;
  LL_DAC_ClearFlag_DMAUDR1(AUDIO_DAC);
#else
  LL_DMA_DisableStream(AUDIO_DMA, AUDIO_DMA_Stream);

  // Wait until DMA EN bit is actually cleared by hardware.
  uint32_t timeout = 1000;
  while (LL_DMA_IsEnabledStream(AUDIO_DMA, AUDIO_DMA_Stream) && timeout--) {
    __NOP();
  }

  dac_clear_dma_flags();
#endif
}

static void dac_start_dma()
{
#if defined(STM32H5) || defined(STM32H7RS)
  // On GPDMA, dac_close_dma_xfer() suspends and resets the channel, which tears
  // it down: the linked-list pointer (CxLLR) and the source/block-length
  // registers are lost, so rebuild the descriptor before re-arming.
  dac_dma_init();
#else
  // re-arm from the start of the buffer: a mid-transfer stop leaves NDTR and the
  // memory address partway, which desyncs the HT/TC half tracking
  LL_DMA_DisableStream(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_SetMemoryAddress(AUDIO_DMA, AUDIO_DMA_Stream, (uintptr_t)_dma_buffer);
  LL_DMA_SetDataLength(AUDIO_DMA, AUDIO_DMA_Stream, DMA_BUFFER_LEN);

  dac_clear_dma_flags();
#endif

  // enable DMA stream and transfer complete interrupt
  LL_DMA_EnableIT_HT(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_EnableIT_TC(AUDIO_DMA, AUDIO_DMA_Stream);
#if defined(STM32H5) || defined(STM32H7RS)
  LL_DMA_EnableChannel(AUDIO_DMA, AUDIO_DMA_Stream);
#else
  LL_DMA_EnableStream(AUDIO_DMA, AUDIO_DMA_Stream);
#endif

  // clear underrun flag
  AUDIO_DAC_CLEAR_DMAUDR();

  // enable DAC
  AUDIO_DAC->CR |= AUDIO_DAC_CR_EN | AUDIO_DAC_CR_DMAEN;
}

void audioConsumeCurrentBuffer()
{
#if defined(STM32H5) || defined(STM32H7RS)
  // An underrun latches DMAUDR1 and the DAC stops taking DMA requests while
  // the channel stays enabled, so the re-arm below would never fire.
  if (LL_DAC_IsActiveFlag_DMAUDR1(AUDIO_DAC)) {
    LL_DAC_ClearFlag_DMAUDR1(AUDIO_DAC);
    dac_close_dma_xfer();
    _empty_dma_halves = 0;
  }

  if (!LL_DMA_IsEnabledChannel(AUDIO_DMA, AUDIO_DMA_Stream)) {
#else
  if (!LL_DMA_IsEnabledStream(AUDIO_DMA, AUDIO_DMA_Stream)) {
#endif
    // Prime both halves before starting the circular DMA. The interrupt only
    // refills one half at a time, so if we left the second half untouched it
    // would play stale/garbage data for the first lap (a crackle), then a gap.
    bool empty0 = audio_update_dma_buffer(0);
    bool empty1 = audio_update_dma_buffer(1);
    if (!empty0 || !empty1) {
      _empty_dma_halves = 0;
      // prime the second half as well so the first full DMA cycle plays valid
      // data and the half tracking starts aligned (ignore the result: if no
      // more data is available it is filled with silence)
      audio_update_dma_buffer(1);
#if defined(AUDIO_MUTE_GPIO)
      audioUnmute();
#endif
      dac_start_dma();
    } else {
#if defined(AUDIO_MUTE_GPIO)
      audioMute();
#endif
    }
  }
}

extern "C" void AUDIO_DMA_Stream_IRQHandler()
{
#if defined(STM32H5) || defined(STM32H7RS)
  // Clear error flags to prevent infinite ISR loop
  LL_DMA_ClearFlag_DTE(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_ClearFlag_ULE(AUDIO_DMA, AUDIO_DMA_Stream);
  LL_DMA_ClearFlag_USE(AUDIO_DMA, AUDIO_DMA_Stream);
#endif

  bool hasData = false;
  if(stm32_dma_check_ht_flag(AUDIO_DMA, AUDIO_DMA_Stream)) {
    if (audio_update_dma_buffer(0)) {
      if (_empty_dma_halves < 0xFF) _empty_dma_halves++;
    } else {
      hasData = true;
    }
  }

  if(stm32_dma_check_tc_flag(AUDIO_DMA, AUDIO_DMA_Stream)) {
    if (audio_update_dma_buffer(1)) {
      if (_empty_dma_halves < 0xFF) _empty_dma_halves++;
    } else {
      hasData = true;
    }
  }

  if (hasData) {
    _empty_dma_halves = 0;
  }

  if(_empty_dma_halves >= DMA_EMPTY_HALVES_STOP_THRESHOLD) {
    _empty_dma_halves = 0;
    dac_close_dma_xfer();
  }
}

#else // STM32H5 || STM32H7 || STM32H7RS

#define AUDIO_DAC DAC
#define DAC_TRIGGER 0 // TIM6

const AudioBuffer * nextBuffer = nullptr;

// no private DMA buffer on this path: the DMA plays straight out of the FIFO
static inline void dac_prime_silence() {}

static void dac_dma_init()
{
  stm32_dma_enable_clock(AUDIO_DMA);

  // Disable DMA stream
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN;

  // clear DMA stream flags
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 |
                     DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

  // Chan 7, 16-bit wide, Medium priority, memory increments
  AUDIO_DMA_Stream->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 |
                         DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0 |
                         DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0;

  // write to DAC channel 1 (12 bits, left-aligned)
  AUDIO_DMA_Stream->PAR = CONVERT_PTR_UINT(&AUDIO_DAC->DHR12L1);

  // disable direct mode and set FIFO threshold to half
  AUDIO_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

  NVIC_EnableIRQ(AUDIO_DMA_Stream_IRQn);
  NVIC_SetPriority(AUDIO_DMA_Stream_IRQn, 7);
}

static void dac_close_dma_xfer()
{
  // disable transfer complete interrupt
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_TCIE;

  // reset DMA flags
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 |
                     DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

  // Disable DMA stream
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN;
}

static void dac_start_dma_request(const audio_data_t* data, uint16_t length)
{
  // set DMA buffer
  AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(data);
  AUDIO_DMA_Stream->NDTR = length;

  // enable DMA stream and transfer complete interrupt
  AUDIO_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;

  // clear underrun flag
  AUDIO_DAC->SR = DAC_SR_DMAUDR1;

  // enable DAC
  AUDIO_DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1;
}

void audioConsumeCurrentBuffer()
{
  if (!nextBuffer) {
    nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (nextBuffer) {

#if defined(AUDIO_MUTE_GPIO)
      audioUnmute();
#endif
      dac_close_dma_xfer();
      dac_start_dma_request(nextBuffer->data, nextBuffer->size);
    } else {
#if defined(AUDIO_MUTE_GPIO)
      audioMute();
#endif
    }
  }
}

extern "C" void AUDIO_DMA_Stream_IRQHandler()
{
  dac_close_dma_xfer();

  // get next buffer if any
  if (nextBuffer) audioQueue.buffersFifo.freeNextFilledBuffer();
  nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();

  if (nextBuffer) {
    dac_start_dma_request(nextBuffer->data, nextBuffer->size);
  }
}

#endif

// Init timer that triggers the DAC conversion
// at sampling frequency
static void dac_trigger_init()
{
  stm32_timer_enable_clock(AUDIO_TIMER);

  // A basic timer's TRGO is the update event, one timer clock wide - 4ns at
  // 250MHz. Prescaling widens that pulse without moving the sample rate.
#if !defined(AUDIO_TIMER_PSC)
  #define AUDIO_TIMER_PSC 1
#endif
  AUDIO_TIMER->PSC = AUDIO_TIMER_PSC - 1;

  // reset counter
  AUDIO_TIMER->CNT = 0;

  // set expiry to sample period (1 / sample_rate)
  AUDIO_TIMER->ARR =
      ((PERI1_FREQUENCY * TIMER_MULT_APB1) / AUDIO_TIMER_PSC) / AUDIO_SAMPLE_RATE - 1;

  // Master mode selection
  //
  //  000: Reset
  AUDIO_TIMER->CR2 = 0;
  //  010: The update event is selected as trigger output (TRGO)
  //       (used to trigger DAC conversion)
  AUDIO_TIMER->CR2 = TIM_CR2_MMS_1;

  // enable timer
  AUDIO_TIMER->CR1 |= TIM_CR1_CEN;
}

static void dac_periph_init()
{

#if defined(LL_APB1_GRP1_PERIPH_DAC12)
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC12);
#elif defined(LL_AHB2_GRP1_PERIPH_DAC1)
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_DAC1);
#else
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
#endif

  gpio_init_analog(AUDIO_OUTPUT_GPIO);

#if defined(LL_DAC_HIGH_FREQ_MODE_DISABLE)
  // STM32H5/H7: the DAC sample/hold timing must be told the bus clock band via
  // HFSEL, otherwise the analog output is distorted (and marginal => varies
  // boot-to-boot). Must be written while the channel is disabled (before EN1).
  // The band is on the AHB clock, NOT the ADCDACSEL kernel clock: see
  // HAL_DAC_ConfigChannel()'s AUTOMATIC mode, which reads HAL_RCC_GetHCLKFreq().
#if defined(AUDIO_DAC_HFSEL)
  LL_DAC_SetHighFrequencyMode(AUDIO_DAC, AUDIO_DAC_HFSEL);
#elif defined(STM32H5)
  // SystemCoreClock is HCLK here (SystemCoreClockUpdate() applies HPRE)
  uint32_t hclk = SystemCoreClock;
  LL_DAC_SetHighFrequencyMode(
      AUDIO_DAC, hclk > 160000000 ? LL_DAC_HIGH_FREQ_MODE_ABOVE_160MHZ
               : hclk >  80000000 ? LL_DAC_HIGH_FREQ_MODE_ABOVE_80MHZ
                                  : LL_DAC_HIGH_FREQ_MODE_DISABLE);
#elif (PERI1_FREQUENCY > 160000000)
  LL_DAC_SetHighFrequencyMode(AUDIO_DAC, LL_DAC_HIGH_FREQ_MODE_ABOVE_160MHZ);
#elif (PERI1_FREQUENCY > 80000000)
  LL_DAC_SetHighFrequencyMode(AUDIO_DAC, LL_DAC_HIGH_FREQ_MODE_ABOVE_80MHZ);
#else
  LL_DAC_SetHighFrequencyMode(AUDIO_DAC, LL_DAC_HIGH_FREQ_MODE_DISABLE);
#endif
#endif

  // set data registre to silence
  AUDIO_DAC->AUDIO_DAC_DHR12L = AUDIO_DATA_SILENCE;

  // clear underrun flag
  AUDIO_DAC->SR = AUDIO_DAC_SR_DMAUDR;

  // use TIM6 TRGO as trigger
  // enable DAC & channel trigger
  AUDIO_DAC->CR = AUDIO_DAC_CR_TEN | AUDIO_DAC_CR_EN | AUDIO_DAC_TRIGGER;
}

// Configure DAC0
// Not sure why PB14 has not be allocated to the DAC, although it is an EXTRA function
// So maybe it is automatically done
void dacInit()
{
#if defined(AUDIO_MUTE_GPIO)
  // Mute before init anything
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  set_mute_pin(true);
#endif

  dac_trigger_init();
  dac_prime_silence();
  dac_dma_init();
  dac_periph_init();
}

// Sound routines
void audioInit()
{
  dacInit();
}
