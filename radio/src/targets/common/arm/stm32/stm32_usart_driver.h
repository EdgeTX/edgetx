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

#pragma once

#include <stdint.h>
#include "hal/serial_driver.h"
#include "stm32_hal_ll.h"

struct stm32_usart_t {

    // USART defs
    USART_TypeDef*             USARTx;
    GPIO_TypeDef*              GPIOx;
    const LL_GPIO_InitTypeDef* pinInit;

    // USART IRQ
    IRQn_Type                  IRQn;
    uint8_t                    IRQ_Prio;

    // TX DMA defs
    DMA_TypeDef*               DMAx;
    uint32_t                   DMA_Stream;
    uint32_t                   DMA_Channel;
};

void stm32_usart_init(const stm32_usart_t* usart, const etx_serial_init* params);
void stm32_usart_deinit(const stm32_usart_t* usart);
void stm32_usart_send_byte(const stm32_usart_t* usart, uint8_t byte);
void stm32_usart_send_buffer(const stm32_usart_t* usart, const uint8_t * data, uint32_t size);
void stm32_usart_wait_for_tx_dma(const stm32_usart_t* usart);
void stm32_usart_isr(const stm32_usart_t* usart, etx_serial_callbacks_t* cb);


