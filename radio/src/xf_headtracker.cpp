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

#include "crc.h"
#include "edgetx.h"
#include "os/timer.h"
#include "tasks/mixer_task.h"

static const etx_serial_driver_t* xfhtSerialDrv = nullptr;
static void* xfhtSerialCtx = nullptr;

static uint8_t xfhtFrame[XFHT_FRAME_LENGTH];
static uint8_t xfhtFrameIndex = 0;
static int16_t xfhtValues[XFHT_CHANNEL_COUNT];
static tmr10ms_t xfhtLastByte = 0;
static tmr10ms_t xfhtLastFrame = 0;
static bool xfhtValid = false;

static timer_handle_t xfhtTimer = TIMER_INITIALIZER;

static void xfhtDecodeFrame()
{
  uint16_t crc = ((uint16_t)xfhtFrame[XFHT_FRAME_LENGTH - 2] << 8) |
                 xfhtFrame[XFHT_FRAME_LENGTH - 1];
  if (crc16(CRC_1021, xfhtFrame, XFHT_FRAME_LENGTH - 2) != crc) return;

  for (uint8_t ch = 0; ch < XFHT_CHANNEL_COUNT; ch++) {
    uint16_t ticks =
        xfhtFrame[2 + ch * 2] | ((uint16_t)xfhtFrame[3 + ch * 2] << 8);
    // 1000..2000 us -> -1024..1024
    int32_t value = ((int32_t)ticks - 1500 * XFHT_TICKS_PER_US) * 1024 /
                    (500 * XFHT_TICKS_PER_US);
    xfhtValues[ch] = limit<int32_t>(-1024, value, 1024);
  }
  xfhtLastFrame = get_tmr10ms();
  xfhtValid = true;
}

void xfhtParseByte(uint8_t c)
{
  // a header-like byte pair inside the data could otherwise keep the parser out
  // of step for good
  tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - xfhtLastByte) >= XFHT_FRAME_GAP_MS / 10)
    xfhtFrameIndex = 0;
  xfhtLastByte = now;

  if (xfhtFrameIndex == 0 && c != XFHT_HEADER1) return;
  if (xfhtFrameIndex == 1 && c != XFHT_HEADER2) {
    xfhtFrameIndex = (c == XFHT_HEADER1) ? 1 : 0;
    return;
  }
  xfhtFrame[xfhtFrameIndex++] = c;
  if (xfhtFrameIndex >= XFHT_FRAME_LENGTH) {
    xfhtDecodeFrame();
    xfhtFrameIndex = 0;
  }
}

static void xfhtWakeup()
{
  auto drv = xfhtSerialDrv;
  auto ctx = xfhtSerialCtx;
  if (!drv || !ctx || !drv->getByte) return;

  uint8_t byte;
  while (drv->getByte(ctx, &byte) > 0) {
    xfhtParseByte(byte);
  }
}

static void xfhtTimerCb(timer_handle_t* timer)
{
  if (mixerTaskRunning()) {
    xfhtWakeup();
  }
}

void xfhtSetSerialDriver(void* ctx, const etx_serial_driver_t* drv)
{
  xfhtSerialCtx = ctx;
  xfhtSerialDrv = drv;
  xfhtFrameIndex = 0;
  xfhtValid = false;

  if (!timer_is_created(&xfhtTimer)) {
    timer_create(&xfhtTimer, xfhtTimerCb, "xfht", 10, true);
  }
  if (ctx && drv) {
    timer_start(&xfhtTimer);
  } else {
    timer_stop(&xfhtTimer);
  }
}

int16_t get_xfht_value(uint8_t axis)
{
  // a lost link centres the outputs rather than freezing the last head position
  if (axis >= XFHT_AXIS_COUNT || !xfhtValid ||
      (tmr10ms_t)(get_tmr10ms() - xfhtLastFrame) > XFHT_TIMEOUT_MS / 10) {
    return 0;
  }
  return xfhtValues[XFHT_FIRST_AXIS + axis];
}
