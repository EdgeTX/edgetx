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
#include <string.h>

void stm32_usart_init(const stm32_usart_t* usart, const etx_serial_init* params)
{
  LL_USART_DeInit(usart->USARTx);
  LL_GPIO_Init(usart->GPIOx, (LL_GPIO_InitTypeDef*)usart->pinInit);
  
  LL_USART_InitTypeDef usartInit;
  usartInit.BaudRate = params->baudrate;

  uint32_t parity = LL_USART_PARITY_NONE;
  switch(params->parity){
  case ETX_Parity_None:
      parity = LL_USART_PARITY_NONE;
      break;
  case ETX_Parity_Even:
      parity = LL_USART_PARITY_EVEN;
      break;
  case ETX_Parity_Odd:
      parity = LL_USART_PARITY_ODD;
      break;
  }
  usartInit.Parity = parity;

  uint16_t stop_bits = LL_USART_STOPBITS_1;
  switch(params->stop_bits) {
  case ETX_StopBits_One:
      stop_bits = LL_USART_STOPBITS_1;
      break;
  case ETX_StopBits_OneAndHalf:
      stop_bits = LL_USART_STOPBITS_1_5;
      break;
  case ETX_StopBits_Two:
      stop_bits = LL_USART_STOPBITS_2;
      break;
  }
  usartInit.StopBits = stop_bits;

  if (params->word_length == ETX_WordLength_9) {
      usartInit.DataWidth = LL_USART_DATAWIDTH_9B;
  } else {
      usartInit.DataWidth = LL_USART_DATAWIDTH_8B;
  }

  usartInit.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  usartInit.TransferDirection = LL_USART_DIRECTION_TX_RX;

  LL_USART_Init(usart->USARTx, &usartInit);
  LL_USART_Enable(usart->USARTx);

  // Enable TX DMA request
  if (usart->DMAx) {
    LL_USART_EnableDMAReq_TX(usart->USARTx);
  }

  // Enable RX IRQ
  if (params->rx_enable) {
    LL_USART_EnableIT_RXNE(usart->USARTx);
  }

  if (!usart->DMAx || params->rx_enable) {
    // TODO: configurable priority ???
    NVIC_SetPriority(usart->IRQn, 6);
    NVIC_EnableIRQ(usart->IRQn);
  }
}

void stm32_usart_deinit(const stm32_usart_t* usart)
{
  if (usart->DMAx) {
    LL_DMA_DeInit(usart->DMAx, usart->DMA_Stream);
  }
  NVIC_DisableIRQ(usart->IRQn);
  LL_USART_DeInit(usart->USARTx);

  // Reconfigure pin as output
  LL_GPIO_InitTypeDef pinInit;
  memcpy(&pinInit, usart->pinInit, sizeof(LL_GPIO_InitTypeDef));
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_NO;
  pinInit.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(usart->GPIOx, &pinInit);
  LL_GPIO_ResetOutputPin(usart->GPIOx, pinInit.Pin);
}

void stm32_usart_send_byte(const stm32_usart_t* usart, uint8_t byte)
{
  while (!LL_USART_IsActiveFlag_TXE(usart->USARTx));
  LL_USART_TransmitData8(usart->USARTx, byte);
}

void stm32_usart_send_buffer(const stm32_usart_t* usart, const uint8_t * data, uint32_t size)
{
  if (usart->DMAx) {
    LL_DMA_DeInit(usart->DMAx, usart->DMA_Stream);

    LL_DMA_InitTypeDef dmaInit;
    LL_DMA_StructInit(&dmaInit);

    dmaInit.Channel = usart->DMA_Channel;
    dmaInit.PeriphOrM2MSrcAddress = (uint32_t)&usart->USARTx->DR;
    dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dmaInit.MemoryOrM2MDstAddress = (uint32_t)data;
    dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dmaInit.NbData = size;
    dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;

    LL_DMA_Init(usart->DMAx, usart->DMA_Stream, &dmaInit);
    LL_DMA_EnableStream(usart->DMAx, usart->DMA_Stream);

    return;
  } else {
    // Please note that we don't use the buffer:
    // it should set internally by the driver user
    // and each byte is returned individually by on_send()
    LL_USART_EnableIT_TXE(usart->USARTx);
  }
}

void stm32_usart_wait_for_tx_dma(const stm32_usart_t* usart)
{
  // TODO: check if everything is properly initialised, this seems to block when
  //       the port has been initialised with a zero baudrate
  if (LL_DMA_IsEnabledStream(usart->DMAx, usart->DMA_Stream)) {

    switch(usart->DMA_Stream) {
    case LL_DMA_STREAM_1:
      while (LL_DMA_IsActiveFlag_TC1(usart->DMAx));
      LL_DMA_ClearFlag_TC1(usart->DMAx);
      break;
    case LL_DMA_STREAM_3:
      while (LL_DMA_IsActiveFlag_TC3(usart->DMAx));
      LL_DMA_ClearFlag_TC3(usart->DMAx);
      break;
    case LL_DMA_STREAM_5:
      while (LL_DMA_IsActiveFlag_TC5(usart->DMAx));
      LL_DMA_ClearFlag_TC5(usart->DMAx);
      break;
    case LL_DMA_STREAM_6:
      while (LL_DMA_IsActiveFlag_TC6(usart->DMAx));
      LL_DMA_ClearFlag_TC6(usart->DMAx);
      break;
    case LL_DMA_STREAM_7:
      while (LL_DMA_IsActiveFlag_TC7(usart->DMAx));
      LL_DMA_ClearFlag_TC7(usart->DMAx);
      break;
    }
  }
}

#define USART_FLAG_ERRORS \
  (LL_USART_SR_ORE | LL_USART_SR_NE | LL_USART_SR_FE | LL_USART_SR_PE)

void stm32_usart_isr(const stm32_usart_t* usart, etx_serial_callbacks_t* cb)
{
  uint32_t status = LL_USART_ReadReg(usart->USARTx, SR);

  // IRQ based send: TXE IRQ is enabled only during transfer
  if (LL_USART_IsEnabledIT_TXE(usart->USARTx) && (status & LL_USART_SR_TXE)) {

    uint8_t data;
    if (cb->on_send && cb->on_send(data)) {
      LL_USART_TransmitData8(usart->USARTx, data);
    } else {
      LL_USART_DisableIT_TXE(usart->USARTx);
    }
  }

  // Receive
  if (LL_USART_IsEnabledIT_TXE(usart->USARTx)) {

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
}
