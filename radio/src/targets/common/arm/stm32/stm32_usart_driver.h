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
#include "hal/gpio.h"
#include "stm32_hal_ll.h"

typedef void (*pin_setter_t)(uint8_t enable);

struct stm32_usart_t {

    // USART defs
    USART_TypeDef*             USARTx;
    gpio_t                     txGPIO;
    gpio_t                     rxGPIO;

    // USART IRQ
    IRQn_Type                  IRQn;
    uint8_t                    IRQ_Prio;

    // TX DMA defs
    DMA_TypeDef*               txDMA;
    uint32_t                   txDMA_Stream;
    uint32_t                   txDMA_Channel;

    // RX DMA defs
    DMA_TypeDef*               rxDMA;
    uint32_t                   rxDMA_Stream;
    uint32_t                   rxDMA_Channel;

    // 2-wire Half-duplex settings (uses direction pin)
    pin_setter_t               set_input;
    IRQn_Type                  txDMA_IRQn;
    uint8_t                    txDMA_IRQ_Prio;
};

bool stm32_usart_init(const stm32_usart_t* usart, const etx_serial_init* params);
void stm32_usart_init_rx_dma(const stm32_usart_t* usart, const void* buffer, uint32_t length);
void stm32_usart_enable_tx_irq(const stm32_usart_t* usart);
void stm32_usart_set_idle_irq(const stm32_usart_t* usart, uint32_t enabled);
void stm32_usart_deinit(const stm32_usart_t* usart);
void stm32_usart_deinit_rx_dma(const stm32_usart_t* usart);
void stm32_usart_send_byte(const stm32_usart_t* usart, uint8_t byte);
void stm32_usart_send_buffer(const stm32_usart_t* usart, const uint8_t * data, uint32_t size);
uint8_t stm32_usart_tx_completed(const stm32_usart_t* usart);
void stm32_usart_wait_for_tx_dma(const stm32_usart_t* usart);
void stm32_usart_enable_rx(const stm32_usart_t* usart);
uint32_t stm32_usart_get_baudrate(const stm32_usart_t* usart);
void stm32_usart_set_baudrate(const stm32_usart_t* usart, uint32_t baudrate);
void stm32_usart_set_hw_option(const stm32_usart_t* usart, uint32_t option);
void stm32_usart_isr(const stm32_usart_t* usart, etx_serial_callbacks_t* cb);
void stm32_usart_tx_dma_isr(const stm32_usart_t* usart);
