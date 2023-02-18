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

#include "stm32_usart_driver.h"
#include "stm32_dma.h"

#include <string.h>

// WARNING:
//
// NVIC_GetEnableIRQ is stolen from "${THIRDPARTY_DIR}/CMSIS/Include/core_cm4.h".
//
// Until we can include the new CMSIS everywhere, this
// needs to be defined here, as the old CMSIS does not define it.
//
// The new CMSIS can be used when StdPeriph is gone, as it mandates
// the old version located in "${STM32LIB_DIR}/CMSIS/Include"
//
static inline uint32_t NVIC_GetEnableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0) {
    return ((uint32_t)(((NVIC->ISER[(((uint32_t)IRQn) >> 5UL)] &
                         (1UL << (((uint32_t)IRQn) & 0x1FUL))) != 0UL)
                           ? 1UL
                           : 0UL));
  } else {
    return (0U);
  }
}

static void _enable_usart_irq(const stm32_usart_t* usart)
{
  NVIC_SetPriority(usart->IRQn, usart->IRQ_Prio);
  NVIC_EnableIRQ(usart->IRQn);
}

static void _enable_tx_dma_irq(const stm32_usart_t* usart)
{
  NVIC_SetPriority(usart->txDMA_IRQn, usart->txDMA_IRQ_Prio);
  NVIC_EnableIRQ(usart->txDMA_IRQn);
}

static inline void _half_duplex_input(const stm32_usart_t* usart)
{
  if (usart->set_input) {
    usart->set_input(true);
    LL_USART_EnableDirectionRx(usart->USARTx);
  }
}

static inline void _half_duplex_output(const stm32_usart_t* usart)
{
  if (usart->set_input) {
    usart->set_input(false);
    LL_USART_DisableDirectionRx(usart->USARTx);
  }
}

void stm32_usart_enable_tx_irq(const stm32_usart_t* usart)
{
  if (LL_USART_IsEnabledDMAReq_TX(usart->USARTx)) {
    stm32_usart_wait_for_tx_dma(usart);
    LL_USART_DisableDMAReq_TX(usart->USARTx);
  }

  if (!NVIC_GetEnableIRQ(usart->IRQn)) {
    _enable_usart_irq(usart);
  }
  
  LL_USART_EnableIT_TXE(usart->USARTx);
}

void stm32_usart_set_idle_irq(const stm32_usart_t* usart, uint32_t enabled)
{
  if (enabled) {
    LL_USART_EnableIT_IDLE(usart->USARTx);
    _enable_usart_irq(usart);
  } else {
    LL_USART_DisableIT_IDLE(usart->USARTx);
    // Let's assume the port will be disabled soon
    // and we don't need to figure out whether or
    // not the IRQ needs to be disabled
  }
}

static void enable_usart_clock(USART_TypeDef* USARTx)
{
  if (USARTx == USART1) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  } else if (USARTx == USART2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  } else if (USARTx == USART3) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
  } else if (USARTx == UART4) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
  } else if (USARTx == USART6) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
  }
#if defined(UART7) // does not exist on F2
  else if (USARTx == UART7) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART7);
  }
#endif
}

static void disable_usart_clock(USART_TypeDef* USARTx)
{
  if (USARTx == USART1) {
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
  } else if (USARTx == USART2) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
  } else if (USARTx == USART3) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART3);
  } else if (USARTx == UART4) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART4);
  } else if (USARTx == USART6) {
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART6);
  }
#if defined(UART7) // does not exist on F2
  else if (USARTx == UART7) {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART7);
  }
#endif

}

static uint32_t _get_pin_speed(uint32_t baudrate)
{
  // 1 Mbps and above
  if (baudrate >= 1000000) {
    return LL_GPIO_SPEED_FREQ_VERY_HIGH;
  }
  // 400kbps and above
  else if (baudrate >= 400000) {
    return LL_GPIO_SPEED_FREQ_HIGH;
  }

  // under 400kbps
  return LL_GPIO_SPEED_FREQ_LOW;
}

static uint32_t _get_usart_af(USART_TypeDef* USARTx)
{
  if (USARTx == USART1 || USARTx == USART2 || USARTx == USART3) {
    return LL_GPIO_AF_7;
  } else {
    return LL_GPIO_AF_8;
  }
}

void stm32_usart_init_rx_dma(const stm32_usart_t* usart, const void* buffer, uint32_t length)
{
  if (!usart->rxDMA) return;

  // Disable IRQ based RX
  LL_USART_DisableIT_RXNE(usart->USARTx);

  // In case TX DMA is used and IDLE IRQ is not, disable the ISR completely
  if (usart->txDMA && !LL_USART_IsEnabledIT_IDLE(usart->USARTx)) {
    NVIC_DisableIRQ(usart->IRQn);
  }

  LL_DMA_DeInit(usart->rxDMA, usart->rxDMA_Stream);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.Channel = usart->rxDMA_Channel;
  dmaInit.PeriphOrM2MSrcAddress = (uint32_t)&usart->USARTx->DR;
  dmaInit.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dmaInit.MemoryOrM2MDstAddress = (uint32_t)buffer;
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.Mode = LL_DMA_MODE_CIRCULAR;
  dmaInit.NbData = length;
  dmaInit.Priority = LL_DMA_PRIORITY_LOW; // TODO: make it configurable
  LL_DMA_Init(usart->rxDMA, usart->rxDMA_Stream, &dmaInit);
  LL_USART_EnableDMAReq_RX(usart->USARTx);

  // Stream can be enable as the USART has alread been enabled
  LL_DMA_EnableStream(usart->rxDMA, usart->rxDMA_Stream);
}

void stm32_usart_deinit_rx_dma(const stm32_usart_t* usart)
{
  if (!usart->rxDMA) return;

  LL_DMA_DeInit(usart->rxDMA, usart->rxDMA_Stream);

  if ((int32_t)(usart->IRQn) >= 0) {

    // Enable IRQ based RX
    LL_USART_EnableIT_RXNE(usart->USARTx);
    _enable_usart_irq(usart);
  }
}

// TODO: probably needs to be a separate API on the serial driver
//
// - USART_OneBitMethodCmd(TELEMETRY_USART, ENABLE);

// OBSOLETE:
// - asymmetric bitrates for half-duplex (GHOST)
// - ??? ability to switch RX DMA OFF ??? (-> X12S)
//
void stm32_usart_init(const stm32_usart_t* usart, const etx_serial_init* params)
{
  enable_usart_clock(usart->USARTx);
  LL_USART_DeInit(usart->USARTx);

  // TODO: enable GPIO clock
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = usart->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Speed = _get_pin_speed(params->baudrate);
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP;
  pinInit.Alternate = _get_usart_af(usart->USARTx);
  LL_GPIO_Init(usart->GPIOx, &pinInit);
  
  LL_USART_InitTypeDef usartInit;
  LL_USART_StructInit(&usartInit);

  usartInit.BaudRate = params->baudrate;
  usartInit.Parity = LL_USART_PARITY_NONE;
  usartInit.DataWidth = LL_USART_DATAWIDTH_8B;

  switch(params->encoding){

  case ETX_Encoding_8E2:
    usartInit.Parity = LL_USART_PARITY_EVEN;
    usartInit.StopBits = LL_USART_STOPBITS_2;
    usartInit.DataWidth = LL_USART_DATAWIDTH_9B;
    break;

  case ETX_Encoding_8N1:
    break;

  default:
    // TODO: return some error
    return;
  }

  usartInit.HardwareFlowControl = LL_USART_HWCONTROL_NONE;

  usartInit.TransferDirection = LL_USART_DIRECTION_NONE;

  if (params->direction & ETX_Dir_RX)
    usartInit.TransferDirection |= LL_USART_DIRECTION_RX;

  if (params->direction & ETX_Dir_TX)
    usartInit.TransferDirection |= LL_USART_DIRECTION_TX;

  LL_USART_Init(usart->USARTx, &usartInit);
  LL_USART_Enable(usart->USARTx);

  if (params->direction & ETX_Dir_TX) {
    // Enable TX DMA request
    if (usart->txDMA) {
      LL_USART_EnableDMAReq_TX(usart->USARTx);

      // 2-wire half-duplex: setup TX DMA IRQ
      if (usart->set_input && (int32_t)(usart->txDMA_IRQn) >= 0) {
        _enable_tx_dma_irq(usart);
      }
    }
  }

  if (params->direction & ETX_Dir_RX) {
    // IRQ based RX
    LL_USART_EnableIT_RXNE(usart->USARTx);

    // half-duplex: start in input mode
    if (usart->set_input)
      _half_duplex_input(usart);
  }

  if (((params->direction & ETX_Dir_TX) && !usart->txDMA) ||
      LL_USART_IsEnabledIT_RXNE(usart->USARTx)) {
    _enable_usart_irq(usart);
  }
}

void stm32_usart_deinit(const stm32_usart_t* usart)
{
  if (usart->rxDMA) {
    LL_DMA_DeInit(usart->rxDMA, usart->rxDMA_Stream);
  }
  if (usart->txDMA) {
    LL_DMA_DeInit(usart->txDMA, usart->txDMA_Stream);
  }

  if ((int32_t)(usart->IRQn) >= 0) {
    NVIC_DisableIRQ(usart->IRQn);
  }
  LL_USART_DeInit(usart->USARTx);
  disable_usart_clock(usart->USARTx);

  // Reconfigure pin as output
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = usart->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_NO;
  pinInit.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(usart->GPIOx, &pinInit);
  LL_GPIO_ResetOutputPin(usart->GPIOx, pinInit.Pin);
}

void stm32_usart_send_byte(const stm32_usart_t* usart, uint8_t byte)
{
  // TODO: split into 2 steps to avoid blocking on send
  while (!LL_USART_IsActiveFlag_TXE(usart->USARTx));

  if (usart->set_input) {
    _half_duplex_output(usart);

    // switch back to input after TC
    LL_USART_ClearFlag_TC(usart->USARTx);
    LL_USART_EnableIT_TC(usart->USARTx);
  }

  LL_USART_TransmitData8(usart->USARTx, byte);
}

void stm32_usart_send_buffer(const stm32_usart_t* usart, const uint8_t * data, uint32_t size)
{
  _half_duplex_output(usart);

  if (usart->txDMA) {
    LL_DMA_DeInit(usart->txDMA, usart->txDMA_Stream);

    LL_DMA_InitTypeDef dmaInit;
    LL_DMA_StructInit(&dmaInit);

    dmaInit.Channel = usart->txDMA_Channel;
    dmaInit.PeriphOrM2MSrcAddress = (uint32_t)&usart->USARTx->DR;
    dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dmaInit.MemoryOrM2MDstAddress = (uint32_t)data;
    dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dmaInit.NbData = size;
    dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH; // TODO: make it configurable

    LL_DMA_Init(usart->txDMA, usart->txDMA_Stream, &dmaInit);

    if (usart->set_input && (int32_t)(usart->txDMA_IRQn) >= 0) {
      LL_DMA_EnableIT_TC(usart->txDMA, usart->txDMA_Stream);
    }
    LL_DMA_EnableStream(usart->txDMA, usart->txDMA_Stream);

    return;
  } else {
    // Please note that we don't use the buffer:
    // it should be set internally by the driver user
    // and each byte is returned individually by on_send()
    if ((int32_t)(usart->IRQn) >= 0) LL_USART_EnableIT_TXE(usart->USARTx);
  }
}

uint8_t stm32_usart_tx_completed(const stm32_usart_t* usart)
{
  if (LL_USART_IsEnabledDMAReq_TX(usart->USARTx)) {
    // TX DMA is configured, let's check if the stream is currently enabled
    if (LL_DMA_IsEnabledStream(usart->txDMA, usart->txDMA_Stream) ||
        !LL_USART_IsActiveFlag_TXE(usart->USARTx))
      return 0;
  } else if (LL_USART_IsEnabledIT_TXE(usart->USARTx)) {
    return 0;
  }

  return LL_USART_IsActiveFlag_TXE(usart->USARTx);
}

void stm32_usart_wait_for_tx_dma(const stm32_usart_t* usart)
{
  // TODO: check if everything is properly initialised, this seems to block when
  //       the port has been initialised with a zero baudrate
  if (LL_DMA_IsEnabledStream(usart->txDMA, usart->txDMA_Stream)) {

    switch(usart->txDMA_Stream) {
    case LL_DMA_STREAM_1:
      while (!LL_DMA_IsActiveFlag_TC1(usart->txDMA));
      LL_DMA_ClearFlag_TC1(usart->txDMA);
      break;
    case LL_DMA_STREAM_3:
      while (!LL_DMA_IsActiveFlag_TC3(usart->txDMA));
      LL_DMA_ClearFlag_TC3(usart->txDMA);
      break;
    case LL_DMA_STREAM_5:
      while (!LL_DMA_IsActiveFlag_TC5(usart->txDMA));
      LL_DMA_ClearFlag_TC5(usart->txDMA);
      break;
    case LL_DMA_STREAM_6:
      while (!LL_DMA_IsActiveFlag_TC6(usart->txDMA));
      LL_DMA_ClearFlag_TC6(usart->txDMA);
      break;
    case LL_DMA_STREAM_7:
      while (!LL_DMA_IsActiveFlag_TC7(usart->txDMA));
      LL_DMA_ClearFlag_TC7(usart->txDMA);
      break;
    }
  }
}

void stm32_usart_enable_rx(const stm32_usart_t* usart)
{
  _half_duplex_input(usart);
}

// from stm32f4xx_ll_usart.c
static uint32_t _get_usart_periph_clock(USART_TypeDef* USARTx)
{
  uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
  LL_RCC_ClocksTypeDef rcc_clocks;

  LL_RCC_GetSystemClocksFreq(&rcc_clocks);
  if (USARTx == USART1) {
    periphclk = rcc_clocks.PCLK2_Frequency;
  } else if (USARTx == USART2) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#if defined(USART3)
  else if (USARTx == USART3) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#endif /* USART3 */
#if defined(USART6)
  else if (USARTx == USART6) {
    periphclk = rcc_clocks.PCLK2_Frequency;
  }
#endif /* USART6 */
#if defined(UART4)
  else if (USARTx == UART4) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#endif /* UART4 */
#if defined(UART5)
  else if (USARTx == UART5) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#endif /* UART5 */
#if defined(UART7)
  else if (USARTx == UART7) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#endif /* UART7 */
#if defined(UART8)
  else if (USARTx == UART8) {
    periphclk = rcc_clocks.PCLK1_Frequency;
  }
#endif /* UART8 */
#if defined(UART9)
  else if (USARTx == UART9) {
    periphclk = rcc_clocks.PCLK2_Frequency;
  }
#endif /* UART9 */
#if defined(UART10)
  else if (USARTx == UART10) {
    periphclk = rcc_clocks.PCLK2_Frequency;
  }
#endif /* UART10 */

  return periphclk;
}

uint32_t stm32_usart_get_baudrate(const stm32_usart_t* usart)
{
  auto periphclk = _get_usart_periph_clock(usart->USARTx);
  auto oversampling = LL_USART_GetOverSampling(usart->USARTx);
  return LL_USART_GetBaudRate(usart->USARTx, periphclk, oversampling);
}

void stm32_usart_set_baudrate(const stm32_usart_t* usart, uint32_t baudrate)
{
  auto periphclk = _get_usart_periph_clock(usart->USARTx);
  auto oversampling = LL_USART_GetOverSampling(usart->USARTx);
  LL_USART_SetBaudRate(usart->USARTx, periphclk, oversampling, baudrate);

  // update pin speed accordingly (must be done one-by-one)
  uint32_t pin_speed = _get_pin_speed(baudrate);
  uint32_t pindef = usart->GPIO_Pin;

  // loop borrowed from LL_GPIO_init()
  uint32_t pinpos = POSITION_VAL(pindef);
  uint32_t currentpin = 0;
  while ((pindef >> pinpos) != 0) {
    currentpin = pindef & (1 << pinpos);
    if (currentpin) LL_GPIO_SetPinSpeed(usart->GPIOx, currentpin, pin_speed);
    pinpos++;
  }
}

void stm32_usart_set_hw_option(const stm32_usart_t* usart, uint32_t option)
{
  switch(option) {
  case ETX_HWOption_OVER8: {
    // BRR depends on oversampling as well, so we must
    // re-compute the baudrate as well.
    auto baudrate = stm32_usart_get_baudrate(usart);
    LL_USART_SetOverSampling(usart->USARTx, LL_USART_OVERSAMPLING_8);
    stm32_usart_set_baudrate(usart, baudrate);
  } break;
  case ETX_HWOption_ONEBIT:
    LL_USART_EnableOneBitSamp(usart->USARTx);
    break;
  }
}

#define USART_FLAG_ERRORS \
  (LL_USART_SR_ORE | LL_USART_SR_NE | LL_USART_SR_FE | LL_USART_SR_PE)

void stm32_usart_isr(const stm32_usart_t* usart, etx_serial_callbacks_t* cb)
{
  uint32_t status = LL_USART_ReadReg(usart->USARTx, SR);

  // cache these first, as RXNE might clear SR
  uint32_t idle = (status & LL_USART_SR_IDLE);
  uint32_t txe = (status & LL_USART_SR_TXE);

  // TC is only enabled with 2-wire half-duplex when TX DMA was in use
  if (LL_USART_IsEnabledIT_TC(usart->USARTx) && (status & LL_USART_SR_TC)) {

    // disable TC IRQ
    LL_USART_DisableIT_TC(usart->USARTx);

    // switch to input
    _half_duplex_input(usart);

    // and drain RX side first
    while (status & LL_USART_SR_RXNE) {
      status = LL_USART_ReadReg(usart->USARTx, DR);
      status = LL_USART_ReadReg(usart->USARTx, SR);
    }
  }
  
  // Receive: do it first as it is more time critical
  if (LL_USART_IsEnabledIT_RXNE(usart->USARTx)) {

    // Drain RX
    while (status & (LL_USART_SR_RXNE | USART_FLAG_ERRORS)) {

      // This will clear the RXNE bit in USART_DR register
      uint8_t data = LL_USART_ReadReg(usart->USARTx, DR);

      if (status & USART_FLAG_ERRORS) {
        if (cb->on_error)
          cb->on_error();
      }
      else {
        if (cb->on_receive)
          cb->on_receive(data);
      }

      status = LL_USART_ReadReg(usart->USARTx, SR);
    }
  }

  // IRQ based send: TXE IRQ is enabled only during transfer
  if (LL_USART_IsEnabledIT_TXE(usart->USARTx) && txe) {

    uint8_t data;
    if (cb->on_send && cb->on_send(&data)) {
      LL_USART_TransmitData8(usart->USARTx, data);
    } else {
      LL_USART_DisableIT_TXE(usart->USARTx);
    }
  }

  if (LL_USART_IsEnabledIT_IDLE(usart->USARTx) && idle) {
    // SR clear sequence
    status = LL_USART_ReadReg(usart->USARTx, DR);
    if (cb->on_idle) cb->on_idle();
  }
}

void stm32_usart_tx_dma_isr(const stm32_usart_t* usart)
{
  if (!stm32_dma_check_tc_flag(usart->txDMA, usart->txDMA_Stream))
    return;

  auto USARTx = usart->USARTx;

  // clear TC flag before enabling USART TC interrupt:
  //  -> TC flag will be re-triggered once the last byte which has
  //     just been transfered from DMA to USART will be transmitted,
  //     thus triggering TELEMETRY_USART_IRQHandler(), which will
  //     switch from output to input mode.
  //
  LL_USART_ClearFlag_TC(USARTx);
  LL_USART_EnableIT_TC(USARTx);
}
