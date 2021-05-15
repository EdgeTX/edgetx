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

void btSetBaudrate(uint32_t index)
{
  uint32_t baudrate = (index==0 ? 115200 : (index==1 ? 9600 : 19200));
  UART3_Configure(baudrate, Master_frequency);
}

void btInit()
{
  btSetBaudrate(g_eeGeneral.bluetoothBaudrate);
}

OS_FlagID btFlag;
Fifo<32> btTxFifo;
Fifo<32> btRxFifo;

struct t_serial_tx btTx ;
uint8_t btTxBuffer[32] ;

struct t_serial_tx * Current_bt ;

uint32_t txPdcBt(struct t_serial_tx *data)
{
#ifndef SIMU
  Uart *pUart = BT_USART ;

  if (pUart->UART_TNCR == 0) {
    Current_bt = data;
    data->ready = 1;
    pUart->UART_TPR = (uint32_t) data->buffer;
    pUart->UART_TCR = data->size;
    pUart->UART_PTCR = US_PTCR_TXTEN;
    pUart->UART_IER = UART_IER_TXBUFE;
    NVIC_EnableIRQ(UART1_IRQn);
    return 1; // Sent OK
  }
#endif
  return 0 ;  // Busy
}

extern "C" void UART1_IRQHandler()
{
  Uart *pUart = BT_USART ;
  if ( pUart->UART_SR & UART_SR_TXBUFE ) {
    pUart->UART_IDR = UART_IDR_TXBUFE ;
    pUart->UART_PTCR = US_PTCR_TXTDIS ;
    Current_bt->ready = 0 ;
  }
  if ( pUart->UART_SR & UART_SR_RXRDY ) {
    btRxFifo.push(pUart->UART_RHR);
  }
}

uint16_t rxBtuart()
{
  uint8_t result;
  if (!btRxFifo.pop(result))
    return 0xffff;

  return result;
}

void btSendBuffer()
{
  btTx.buffer = btTxBuffer;
  txPdcBt(&btTx);
  while (btTx.ready == 1) {
    RTOS_WAIT_TICKS(1);
  }
  btTx.size = 0;
}

#define BT_POLL_TIMEOUT         500

uint32_t btChangeBaudrate( uint32_t baudIndex )
{
  uint16_t x ;
  uint32_t y ;
  uint16_t rxchar ;

  x = 4 ;         // 9600
  if ( baudIndex == 0 )
  {
    x = 8 ;         // 115200
  }
  else if ( baudIndex == 2 )
  {
    x = 5 ;         // 19200
  }
  btTxBuffer[0] = 'A' ;
  btTxBuffer[1] = 'T' ;
  btTxBuffer[2] = '+' ;
  btTxBuffer[3] = 'B' ;
  btTxBuffer[4] = 'A' ;
  btTxBuffer[5] = 'U' ;
  btTxBuffer[6] = 'D' ;
  btTxBuffer[7] = '0' + x ;
  btTx.size = 8 ;
  btSendBuffer() ;
  x = 'O' ;
  for( y = 0 ; y < BT_POLL_TIMEOUT ;  y += 1 ) {
    if ( ( rxchar = rxBtuart() ) != 0xFFFF ) {
      if ( rxchar == x ) {
        if ( x == 'O' ) {
          x = 'K' ;
        }
        else {
          break ;                 // Found "OK"
        }
      }
    }
    else {
      RTOS_WAIT_TICKS(1);
    }
  }

  if ( y < BT_POLL_TIMEOUT ) {
    return 1 ;
  }
  else {
    return 0 ;
  }
}

uint32_t btPollDevice()
{
  uint16_t x ;
  uint32_t y ;
  uint16_t rxchar ;

  x = 'O' ;
  btTxBuffer[0] = 'A' ;
  btTxBuffer[1] = 'T' ;
  btTx.size = 2 ;
  btSendBuffer() ;

  for( y = 0 ; y < BT_POLL_TIMEOUT ;  y++) {
    if ( ( rxchar = rxBtuart() ) != 0xFFFF ) {
      if ( rxchar == x ) {
        if ( x == 'O' ) {
          x = 'K' ;
        }
        else {
          break ;                 // Found "OK"
        }
      }
    }
    else {
      RTOS_WAIT_TICKS(1) ;                                        // 2mS
    }
  }

  if ( y < BT_POLL_TIMEOUT ) {
    return 1 ;
  }
  else {
    return 0 ;
  }
}

/*
Commands to BT module
AT+VERSION      Returns the software version of the module
AT+BAUDx        Sets the baud rate of the module:
1       1200
2       2400
3       4800
4       9600 (Default)
5       19200
6       38400
7       57600
8       115200
9       230400
AT+NAME<name here>      Sets the name of the module

Any name can be specified up to 20 characters
AT+PINxxxx      Sets the pairing password of the device

Any 4 digit number can be used, the default pincode is 1234
AT+PN   Sets the parity of the module

So we could send AT+VERSION at different baudrates until we get a response
Then we can change the baudrate to the required value.
Or maybe just AT and get OK back
*/

uint32_t btStatus;

void bluetoothTask(void * pdata)
{
  uint8_t byte;

  btFlag = CoCreateFlag(true, false);
  btTx.size = 0;

  // Look for BT module baudrate, try 115200, and 9600
  // Already initialised to g_eeGeneral.bt_baudrate
  // 0 : 115200, 1 : 9600, 2 : 19200

  uint32_t x = g_eeGeneral.bluetoothBaudrate;

  btStatus = btPollDevice() ;              // Do we get a response?

  for (int y=0; y<2; y++) {
    if (btStatus == 0) {
      x += 1 ;
      if (x > 2) {
        x = 0 ;
      }
      btSetBaudrate(x) ;
      RTOS_WAIT_TICKS(1) ;                                        // 2mS
      btStatus = btPollDevice() ;              // Do we get a response?
    }
  }

  if (btStatus) {
    btStatus = x + 1 ;
    if ( x != g_eeGeneral.bluetoothBaudrate ) {
      x = g_eeGeneral.bluetoothBaudrate ;
      // Need to change Bt Baudrate
      btChangeBaudrate( x ) ;
      btStatus += (x+1) * 10 ;
      btSetBaudrate( x ) ;
    }
  }
  else {
    btInit();
  }

  RTOS_WAIT_TICKS(1) ;

  btPollDevice(); // Do we get a response?

  while (1) {
    uint32_t x = CoWaitForSingleFlag(btFlag, 10); // Wait for data in Fifo
    if (x == E_OK) {
      // We have some data in the Fifo
      while (btTxFifo.pop(byte)) {
        btTxBuffer[btTx.size++] = byte;
        if (btTx.size > 31) {
          btSendBuffer();
        }
      }
    }
    else if (btTx.size) {
      btSendBuffer();
    }
  }
}

void btPushByte(uint8_t data)
{
  btTxFifo.push(data);
  CoSetFlag(btFlag); // Tell the Bt task something to do
}
