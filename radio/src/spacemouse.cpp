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

#include "edgetx.h"
#include "tasks/mixer_task.h"
#include "os/timer.h"

struct spacemousedata_t
{
  signed short axisValues[SPACEMOUSE_CHANNEL_COUNT];
};

enum class sm_bytetype
{
  SM_START,
  SM_DATA,
  SM_CHECKSUMHB,
  SM_CHECKSUMLB,
  SM_FOOTER,
};

struct spacemouseTelegram_t
{
  unsigned char data[SPACEMOUSE_LENGTH_TELEGRAM];
  unsigned short checkSumReceived;
  unsigned short checkSumCalc;
  unsigned char dataIndex;
  sm_bytetype parsestate;
  unsigned char recevied;
};

static spacemouseTelegram_t spaceMouseTelegram = { {0} };
spacemousedata_t spacemouseData;

void spacemouseWakeup();

static timer_handle_t spacemouseTimer = TIMER_INITIALIZER;

static void spacemouseTimerCb(timer_handle_t* timer)
{
  if (mixerTaskRunning()) {
    DEBUG_TIMER_START(debugTimerSpacemouseWakeup);
    spacemousePoll();
    spacemouseWakeup();
    DEBUG_TIMER_STOP(debugTimerSpacemouseWakeup);
  }
}

void spacemouseStart()
{
  if (!timer_is_created(&spacemouseTimer)) {
    timer_create(&spacemouseTimer, spacemouseTimerCb, "spacem", 10, true);
  }

  timer_start(&spacemouseTimer);
}

void spacemouseStop()
{
  timer_stop(&spacemouseTimer);
}

void spacemouseParseNewData(uint8_t c)
{
  switch (spaceMouseTelegram.parsestate) {
    case sm_bytetype::SM_START: {
      if (c == SPACEMOUSE_PROTO_HEADER) {
        spaceMouseTelegram.checkSumCalc = c;
        spaceMouseTelegram.parsestate = sm_bytetype::SM_DATA;
        spaceMouseTelegram.dataIndex = 0;
      }
      break;
    }
    case sm_bytetype::SM_DATA: {
      spaceMouseTelegram.data[spaceMouseTelegram.dataIndex++] = c;
      spaceMouseTelegram.checkSumCalc += c;
      if (spaceMouseTelegram.dataIndex >= SPACEMOUSE_LENGTH_DATA) {
        spaceMouseTelegram.checkSumReceived = 0;
        spaceMouseTelegram.dataIndex = 0;
        spaceMouseTelegram.parsestate = sm_bytetype::SM_CHECKSUMHB;
      }
      break;
    }
    case sm_bytetype::SM_CHECKSUMHB: {
      spaceMouseTelegram.checkSumReceived = c << 7;
      spaceMouseTelegram.parsestate = sm_bytetype::SM_CHECKSUMLB;
      break;
    }
    case sm_bytetype::SM_CHECKSUMLB: {
      spaceMouseTelegram.checkSumReceived += c;
      spaceMouseTelegram.parsestate = sm_bytetype::SM_FOOTER;
      break;
    }
    case sm_bytetype::SM_FOOTER: {
      if ((c == SPACEMOUSE_PROTO_FOOTER) &&
          (spaceMouseTelegram.checkSumReceived == (spaceMouseTelegram.checkSumCalc & 0x3FFF))) {
          for ( uint8_t channel = 0; channel < SPACEMOUSE_CHANNEL_COUNT; channel++ )
          {
            // The values are 7-bit in LSByte and MSByte only. Set MSBit is reserved for start, stop & commands.
            spacemouseData.axisValues[channel] = ((spaceMouseTelegram.data[channel*2] << 7) + spaceMouseTelegram.data[(channel*2)+1]) - SPACEMOUSE_INPUT_OFFSET;
          }
      }
      spaceMouseTelegram.checkSumCalc = 0;
      spaceMouseTelegram.parsestate = sm_bytetype::SM_START;
      break;
    }
  }
}

static const etx_serial_driver_t* spacemouseSerialDrv = nullptr;
static void* spacemouseSerialCtx = nullptr;

#if defined(DEBUG)
bool spacemouseTraceEnabled = false;
#endif

void spacemouseSetSerialDriver(void* ctx, const etx_serial_driver_t* drv)
{
  spacemouseSerialCtx = ctx;
  spacemouseSerialDrv = drv;
  spaceMouseTelegram.parsestate = sm_bytetype::SM_START;
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
