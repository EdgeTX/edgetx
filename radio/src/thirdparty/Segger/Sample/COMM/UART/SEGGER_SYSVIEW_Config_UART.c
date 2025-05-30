/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the SystemView and RTT protocol, and J-Link.       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: 3.60e                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

Purpose : The sample shows how to send/receive SysView via UART.
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include "BSP_UART.h"
#include "SEGGER_SYSVIEW_REC.h"

/*********************************************************************
*
*       Constants
*
**********************************************************************
*/

#define _SYSVIEW_UART_PORT          0             // use 0 for RS232 adapter on ExpIf 0 header.
#define _SYSVIEW_UART_BAUDRATE      700000
#define _SYSVIEW_UART_DATABITS      8
#define _SYSVIEW_UART_PARITY        BSP_UART_PARITY_NONE
#define _SYSVIEW_UART_STOPBITS      1

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static volatile int _UART_Busy = 0;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _SysView_TrySend()
*
*  Function description
*    Try to send queued SysView data over the SysView output channel
*
*  Return value
*     > 0: Amount of data that was transmitted successfully
*    == 0: No data available to be transmitted
*     < 0: Transmit channel was busy, no data sent
*/
static int _SysView_TrySend(void) {
  int Amount;
  U8 Byte;

  if (_UART_Busy) {
    return -1;
  } else {
    Amount = SYSVIEW_REC_GetOutgoing(&Byte, 1);
    if (Amount > 0) {
      _UART_Busy = 1;
      BSP_UART_Write1(_SYSVIEW_UART_PORT, Byte);
      return Amount;
    }
    return 0;
  }
}

/*********************************************************************
*
*       _SysView_OnRxComplete()
*
*  Function description
*    RX-complete callback for SysView input channel
*/
static void _SysView_OnRxComplete(unsigned int Unit, unsigned char c) {
  int ResponseReady;

  if (Unit == _SYSVIEW_UART_PORT) {
    ResponseReady = SYSVIEW_REC_ProcessIncoming(&c, 1);
    if (ResponseReady > 0) {
      _SysView_TrySend();
    }
  }
}

/*********************************************************************
*
*       _SysView_OnTxComplete()
*
*  Function description
*    TX-complete callback for SysView output channel
*
*  Return value
*    == 0: There are more bytes to be sent
*    != 0: Buffer empty, no more bytes to be sent.
*/
static int _SysView_OnTxComplete(unsigned int Unit) {
  if (Unit == _SYSVIEW_UART_PORT) {
    _UART_Busy = 0;
    if (_SysView_TrySend() > 0) {
      return 0;
    }
  }
  return 1;
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_OnEventRecorded()
*
*  Function description
*    Callback for SysView when it has some output available
*/
void SEGGER_SYSVIEW_X_OnEventRecorded(unsigned NumBytes) {
  (void)NumBytes;
  _SysView_TrySend();
}

/*********************************************************************
*
*       SYSVIEW_UART_Config
*/
void SYSVIEW_UART_Config(void);
void SYSVIEW_UART_Config(void) {
  //
  // Init SystemView REC over UART.
  //
  BSP_UART_Init(_SYSVIEW_UART_PORT, _SYSVIEW_UART_BAUDRATE, _SYSVIEW_UART_DATABITS, _SYSVIEW_UART_PARITY, _SYSVIEW_UART_STOPBITS);
  BSP_UART_SetReadCallback(_SYSVIEW_UART_PORT, _SysView_OnRxComplete);
  BSP_UART_SetWriteCallback(_SYSVIEW_UART_PORT, _SysView_OnTxComplete);
}

/**************************** end of file ***************************/
