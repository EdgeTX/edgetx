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
----------------------------------------------------------------------
Purpose     : Implementation of the synchronous SysView recorder state machine
---------------------------END-OF-HEADER------------------------------
*/

#include "SEGGER_SYSVIEW_REC.h"

#include <SEGGER_RTT.h>
#include <SEGGER_SYSVIEW.h>

/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/

static enum {
  WAIT_FOR_HOST_HELLO1_S,
  WAIT_FOR_HOST_HELLO2_V,
  WAIT_FOR_HOST_HELLO3_VERSION1,
  WAIT_FOR_HOST_HELLO4_VERSION2,
  SEND_CLIENT_HELLO1_S,
  SEND_CLIENT_HELLO2_V,
  SEND_CLIENT_HELLO3_VERSION1,
  SEND_CLIENT_HELLO4_VERSION2,
  RECORDING
} _State = WAIT_FOR_HOST_HELLO1_S;

static int _ChannelId = -1;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       SYSVIEW_REC_GetOutgoing()
*
*  Function description
*    Get outgoing data from SystemView buffer to be transmitted.
*
*  Parameters
*    pBuf     - Buffer to store data to.
*    BufSize  - Size of buffer.
*
*  Return value
*    > 0: Number of bytes stored to the buffer.
*    = 0: No data available.
*    < 0: Error (e.g. connection was not opened yet).
*/
int SYSVIEW_REC_GetOutgoing(U8 *pBuf, unsigned BufSize) {
  static const U8 HelloResponse[4] = {'S', 'V', SEND_CLIENT_HELLO3_VERSION1, SEND_CLIENT_HELLO4_VERSION2};
  int RetVal;

  if (BufSize <= 0) {
    return 0;
  }
  switch (_State) {
  case RECORDING:
    RetVal = (int)SEGGER_RTT_ReadUpBufferNoLock(_ChannelId, pBuf, BufSize);
    break;
  case SEND_CLIENT_HELLO4_VERSION2:
    *pBuf = HelloResponse[3];
    RetVal = 1;
    _State = RECORDING;
    break;
  case SEND_CLIENT_HELLO3_VERSION1:
    *pBuf = HelloResponse[2];
    RetVal = 1;
    _State = SEND_CLIENT_HELLO4_VERSION2;
    break;
  case SEND_CLIENT_HELLO2_V:
    *pBuf = HelloResponse[1];
    RetVal = 1;
    _State = SEND_CLIENT_HELLO3_VERSION1;
    break;
  case SEND_CLIENT_HELLO1_S:
    if (BufSize >= 4) {  // check if we can answer efficiently (e.g. via IP)
      pBuf[0] = HelloResponse[0];
      pBuf[1] = HelloResponse[1];
      pBuf[2] = HelloResponse[2];
      pBuf[3] = HelloResponse[3];
      RetVal = 4;
      _State = RECORDING;
    } else {
      *pBuf = HelloResponse[0];
      RetVal = 1;
      _State = SEND_CLIENT_HELLO2_V;
    }
    break;
  case WAIT_FOR_HOST_HELLO4_VERSION2: // fall through!
  case WAIT_FOR_HOST_HELLO3_VERSION1: // fall through!
  case WAIT_FOR_HOST_HELLO2_V:        // fall through!
  case WAIT_FOR_HOST_HELLO1_S:        // fall through from above!
    RetVal = -1;  // connection not yet established
    break;
  }
  return RetVal;
}

/*********************************************************************
*
*       SYSVIEW_REC_ProcessIncoming()
*
*  Function description
*    Event handler to be called upon reveiving incoming data.
*
*  Parameters
*    pBytes   - Buffer containing received data.
*    NumBytes - Number of bytes in the buffer.
*
*  Return value
*    > 0: OK. Response ready to be sent.
*    = 0: OK.
*    < 0: Error.
*/
int SYSVIEW_REC_ProcessIncoming(const U8 *pBytes, unsigned NumBytes) {
  int RetVal;
  U8 Byte;

  RetVal = 0;
  while (NumBytes > 0) {  // Process incoming data one byte at a time.
    Byte = *pBytes;
    pBytes++;
    NumBytes--;
    switch (_State) {
      case RECORDING:
        RetVal = SEGGER_RTT_WriteDownBuffer(_ChannelId, &Byte, 1);
        break;
      case SEND_CLIENT_HELLO4_VERSION2: // fall through!
      case SEND_CLIENT_HELLO3_VERSION1: // fall through!
      case SEND_CLIENT_HELLO2_V:        // fall through!
      case SEND_CLIENT_HELLO1_S:        // fall through from above!
        RetVal = -1;  // need to send response first
        break;
      case WAIT_FOR_HOST_HELLO4_VERSION2:
        _State = SEND_CLIENT_HELLO1_S;          // we don't care about the clients version
        RetVal = 1;
        break;
      case WAIT_FOR_HOST_HELLO3_VERSION1:
        _State = WAIT_FOR_HOST_HELLO4_VERSION2; // we don't care about the clients version
        RetVal = 0;
        break;
      case WAIT_FOR_HOST_HELLO2_V:
        if ('V' == Byte) {
          _State = WAIT_FOR_HOST_HELLO3_VERSION1;
          RetVal = 0;
        } else {
          _State = WAIT_FOR_HOST_HELLO1_S;
          RetVal = -1;
        }
        break;
      case WAIT_FOR_HOST_HELLO1_S:
        if ('S' == Byte) {
          _ChannelId = SEGGER_SYSVIEW_GetChannelID();
          _State = WAIT_FOR_HOST_HELLO2_V;
          RetVal = 0;
        } else {
          _State = WAIT_FOR_HOST_HELLO1_S;
          RetVal = -1;
        }
        break;
    }
    if (RetVal < 0) {
      break;
    }
  }
  return RetVal;
}

/*************************** End of file ****************************/
