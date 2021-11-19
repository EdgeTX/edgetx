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

void stm32_usart_init(USART_TypeDef *USARTx, const etx_serial_init* params)
{
  USART_DeInit(USARTx);

  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate = params->baudrate;

  uint16_t parity = USART_Parity_No;
  switch(params->parity){
  case ETX_Parity_None:
      parity = USART_Parity_No;
      break;
  case ETX_Parity_Even:
      parity = USART_Parity_Even;
      break;
  case ETX_Parity_Odd:
      parity = USART_Parity_Odd;
      break;
  }
  USART_InitStructure.USART_Parity = parity;

  uint16_t stop_bits = USART_StopBits_1;
  switch(params->stop_bits) {
  case ETX_StopBits_One:
      stop_bits = USART_StopBits_1;
      break;
  case ETX_StopBits_OneAndHalf:
      stop_bits = USART_StopBits_1_5;
      break;
  case ETX_StopBits_Two:
      stop_bits = USART_StopBits_2;
      break;
  }
  USART_InitStructure.USART_StopBits = stop_bits;

  if (params->word_length == ETX_WordLength_9) {
      USART_InitStructure.USART_WordLength = USART_WordLength_9b;
  } else {
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  }

  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USARTx, &USART_InitStructure);
  USART_Cmd(USARTx, ENABLE);
}
