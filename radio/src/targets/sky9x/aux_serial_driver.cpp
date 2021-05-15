/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include <stdarg.h>

/** Usart Hw interface used by the console (UART0). */
#define SECOND_SERIAL_UART        UART0
/** Usart Hw ID used by the console (UART0). */
#define SECOND_SERIAL_ID          ID_UART0
/** Pins description corresponding to Rxd,Txd, (UART pins) */
#define SECOND_SERIAL_PINS        {PINS_UART}

Fifo<uint8_t, 512> auxSerialRxFifo;

#if !defined(SIMU)
/*
 * Outputs a character on the UART line.
 *
 * This function is synchronous (i.e. uses polling).
 * c  Character to send.
 */
void auxSerialPutc(const unsigned char c)
{
  Uart *pUart = SECOND_SERIAL_UART;

  /* Wait for the transmitter to be ready */
  while ( (pUart->UART_SR & UART_SR_TXRDY) == 0 ) ;

  /* Send character */
  pUart->UART_THR = c;
}

uint8_t auxSerialTracesEnabled()
{
  return false;
}

/**
 * Configures a UART peripheral with the specified parameters.
 *
 * baudrate  Baudrate at which the UART should operate (in Hz).
 * masterClock  Frequency of the system master clock (in Hz).
 * uses PA9 and PA10, RXD2 and TXD2
 */
void SECOND_UART_Configure(uint32_t baudrate, uint32_t masterClock)
{
  Uart *pUart = SECOND_SERIAL_UART;

  /* Configure PIO */
  configure_pins( (PIO_PA9 | PIO_PA10), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;

  /* Configure PMC */
  PMC->PMC_PCER0 = 1 << SECOND_SERIAL_ID;

  /* Reset and disable receiver & transmitter */
  pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX
                 | UART_CR_RXDIS | UART_CR_TXDIS;

  /* Configure mode */
  pUart->UART_MR = 0x800 ;  // NORMAL, No Parity

  /* Configure baudrate */
  /* Asynchronous, no oversampling */
  pUart->UART_BRGR = (masterClock / baudrate) / 16;

  /* Disable PDC channel */
  pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  /* Enable receiver and transmitter */
  pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;

  pUart->UART_IER = UART_IER_RXRDY;
  NVIC_EnableIRQ(UART0_IRQn);
}

void SECOND_UART_Stop()
{
  SECOND_SERIAL_UART->UART_IDR = UART_IDR_RXRDY ;
  NVIC_DisableIRQ(UART0_IRQn) ;
}

extern "C" void UART0_IRQHandler()
{
  if (SECOND_SERIAL_UART->UART_SR & UART_SR_RXRDY) {
    auxSerialRxFifo.push(SECOND_SERIAL_UART->UART_RHR);
  }
}

#else
#define SECOND_UART_Configure(...)
#endif

void auxSerialTelemetryInit(unsigned int /*protocol*/)
{
  SECOND_UART_Configure(FRSKY_D_BAUDRATE, Master_frequency);
}

bool telemetrySecondPortReceive(uint8_t & data)
{
  return auxSerialRxFifo.pop(data);
}

#if defined(DEBUG) && !defined(SIMU)
void serialPrintf(const char*, ... ) {}
#endif
