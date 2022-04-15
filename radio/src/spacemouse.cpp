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

/*
 * This file is based on code from Cleanflight project
 * https://github.com/cleanflight/cleanflight
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "opentx.h"
#include <ctype.h>

#include <FreeRTOS.h>
#include <timers.h>

struct spacemousedata_t
{
  signed short axisValues[SPACEMOUSE_CHANNEL_COUNT];
};

struct spacemouseTelegram_t
{
  unsigned char head;
  unsigned char data[SPACEMOUSE_LENGTH_TELEGRAM];
  unsigned short checkSum;
  unsigned char startIndex;
  unsigned char endIndex;
  unsigned char dataIndex;
  unsigned char completeFlg;
  unsigned char parsestate;
  unsigned char recevied;
};

enum
{
  SM_START = 0,
  SM_DATA,
  SM_CHECKSUM,
  SM_FOOTER,
};

spacemouseTelegram_t spaceMouseTelegram = { 0 };
spacemousedata_t spacemouseData;

void spacemouseWakeup();

#if !defined(SIMU)
static TimerHandle_t spacemouseTimer = nullptr;
static StaticTimer_t spacemouseTimerBuffer;

static void spacemouseTimerCb(TimerHandle_t xTimer)
{
  if (!s_pulses_paused) {
    DEBUG_TIMER_START(debugTimerSpacemouseWakeup);
    spacemousePoll();
    spacemouseWakeup();
    DEBUG_TIMER_STOP(debugTimerSpacemouseWakeup);
  }
}

void spacemouseStart()
{
  if (!spacemouseTimer) {
    spacemouseTimer =
        xTimerCreateStatic("SpaceM", 10 / RTOS_MS_PER_TICK, pdTRUE, (void*)0,
                           spacemouseTimerCb, &spacemouseTimerBuffer);
  }

  if (spacemouseTimer) {
    if( xTimerStart( spacemouseTimer, 0 ) != pdPASS ) {
      /* The timer could not be set into the Active state. */
    }
  }
}

void spacemouseStop()
{
  if (spacemouseTimer) {
    if( xTimerStop( spacemouseTimer, 12 / RTOS_MS_PER_TICK ) != pdPASS ) {
      /* The timer could not be stopped. */
    }
  }
}
#endif

unsigned short calc_checksum(void *pBuffer,unsigned char BufferSize)
{
  unsigned short checksum = 0;
  while (BufferSize)
  {
      checksum += (*(unsigned char *)pBuffer);
      pBuffer = (void *)((unsigned char *)pBuffer + 1);
      BufferSize--;
  }
  return (checksum & 0x3FFF);
}

void spacemouseParseNewData(uint8_t c)
{
  static volatile bool parse_lock = false;
  if (parse_lock) return;
  parse_lock = true;

  switch (spaceMouseTelegram.parsestate) {
    case SM_START: {
      if (c == SPACEMOUSE_PROTO_HEADER) {
        spaceMouseTelegram.head = SPACEMOUSE_PROTO_HEADER;
        spaceMouseTelegram.parsestate = SM_DATA;
        spaceMouseTelegram.dataIndex = 0;
      }
      break;
    }
    case SM_DATA: {
      spaceMouseTelegram.data[spaceMouseTelegram.dataIndex++] = c;
      if (spaceMouseTelegram.dataIndex >= SPACEMOUSE_LENGTH_DATA) {
        spaceMouseTelegram.checkSum = 0;
        spaceMouseTelegram.dataIndex = 0;
        spaceMouseTelegram.parsestate = SM_CHECKSUM;
      }
      break;
    }
    case SM_CHECKSUM: {
      spaceMouseTelegram.checkSum |= c << ((spaceMouseTelegram.dataIndex++) * 7);
      if (spaceMouseTelegram.dataIndex >= 2) {
        spaceMouseTelegram.dataIndex = 0;
        spaceMouseTelegram.parsestate = SM_FOOTER;
      }
      break;
    }
    case SM_FOOTER: {
      if ((c == SPACEMOUSE_PROTO_FOOTER) ||
          (spaceMouseTelegram.checkSum ==
          calc_checksum((void *)&spaceMouseTelegram.head, SPACEMOUSE_LENGTH_HEADER + SPACEMOUSE_LENGTH_DATA))) {
          for ( uint8_t channel = 0; channel < SPACEMOUSE_CHANNEL_COUNT; channel++ )
          {
            // The values are 7-bit in LSByte and MSByte only. Set MSBit is reserved for start, stop & commands.
            spacemouseData.axisValues[channel] = ((spaceMouseTelegram.data[channel*2] << 7) + spaceMouseTelegram.data[(channel*2)+1]) - SPACEMOUSE_INPUT_OFFSET;
          }
        spaceMouseTelegram.parsestate = SM_START;
      } else {
        // Error handling
      }
      break;
    }
  }
  parse_lock = false;
}

static const etx_serial_driver_t* spacemouseSerialDrv = nullptr;
static void* spacemouseSerialCtx = nullptr;

#if defined(DEBUG)
uint8_t spacemouseTraceEnabled = false;
#endif

void spacemouseSetSerialDriver(void* ctx, const etx_serial_driver_t* drv)
{
  spacemouseSerialCtx = ctx;
  spacemouseSerialDrv = drv;
  spacemouseStart();
}

int16_t get_spacemouse_value(uint8_t ch)
{
  if (ch >= SPACEMOUSE_CHANNEL_COUNT) {
    return 0;
  }

  return (int16_t)(((float)spacemouseData.axisValues[ch])*SPACEMOUSE_OUTPUT_CONV_FACTOR);
}

void spacemouseWakeup()
{
  if (!spacemouseSerialDrv) return;
  
  auto _getByte = spacemouseSerialDrv->getByte;
  if (!_getByte) return;

  uint8_t byte;
  while (_getByte(spacemouseSerialCtx, &byte)) {
#if defined(DEBUG)
    if (spacemouseTraceEnabled) {
      dbgSerialPutc(byte);
    }
#endif
    spacemouseParseNewData(byte);
  }
}

void spacemouseSendCommand(const uint8_t cmd)
{
  if (!spacemouseSerialDrv) return;
  
  auto _sendByte = spacemouseSerialDrv->sendByte;
  if (!_sendByte) return;
  
  TRACE_NOCRLF("spacemouse> 0x%x", cmd);
  _sendByte(spacemouseSerialCtx, cmd);
}

void spacemousePoll()
{
  spacemouseSendCommand(SPACEMOUSE_CMD_REQUEST_DATA);
}

void spacemouseTare()
{
  spacemouseSendCommand(SPACEMOUSE_CMD_SET_ZERO_POSITION);
}

void spacemouseStartStreaming()
{
  spacemouseSendCommand(SPACEMOUSE_CMD_AUTO_DATA_ON);
}

void spacemouseStopStreaming()
{
  spacemouseSendCommand(SPACEMOUSE_CMD_AUTO_DATA_OFF);
}
