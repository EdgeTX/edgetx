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

static uint8_t xfhtFrame[XFHT_MAX_LENGTH];
static uint8_t xfhtFrameIndex = 0;
static int16_t xfhtValues[XFHT_SOURCE_COUNT];
static tmr10ms_t xfhtLastByte = 0;
static tmr10ms_t xfhtLastFrame = 0;
static bool xfhtValid = false;

static timer_handle_t xfhtTimer = TIMER_INITIALIZER;

static uint8_t xfhtFrameLength(uint8_t header1, uint8_t header2)
{
  if (header1 == XFHT_PPM_HEADER1 && header2 == XFHT_PPM_HEADER2)
    return XFHT_PPM_LENGTH;
  if (header1 == XFHT_CSERIES_HEADER1 && header2 == XFHT_CSERIES_HEADER2)
    return XFHT_CSERIES_LENGTH;
  return 0;
}

static int16_t xfhtSigned(uint16_t value, uint8_t bits)
{
  return value >= (1u << (bits - 1)) ? (int16_t)(value - (1u << bits))
                                     : (int16_t)value;
}

static bool xfhtDecodeFrame(uint8_t length)
{
  const uint8_t* f = xfhtFrame;
  uint16_t crc = ((uint16_t)f[length - 2] << 8) | f[length - 1];
  if (crc16(CRC_1021, f, length - 2) != crc) return false;

  if (length == XFHT_PPM_LENGTH) {
    for (uint8_t i = 0; i < XFHT_SOURCE_COUNT; i++) {
      uint8_t ch = XFHT_PPM_FIRST_CHANNEL + i;
      uint16_t ticks = f[2 + ch * 2] | ((uint16_t)f[3 + ch * 2] << 8);
      // 1000..2000 us -> -1024..1024
      int32_t value = ((int32_t)ticks - 1500 * XFHT_PPM_TICKS_PER_US) * 1024 /
                      (500 * XFHT_PPM_TICKS_PER_US);
      xfhtValues[i] = limit<int32_t>(-1024, value, 1024);
    }
  } else {
    // +-180 deg on +-1024, as the PPM output with CwHeadTracker's default
    // ratio; mode 0..2 and sensitivity -16..15 as PPM carries them on CH7, CH8
    xfhtValues[0] = xfhtSigned((f[3] >> 4) | (f[4] << 4), 12) / 2;
    xfhtValues[1] = xfhtSigned(f[5] | ((f[6] & 0x0F) << 8), 12) / 2;
    xfhtValues[2] = xfhtSigned((f[6] >> 4) | (f[7] << 4), 12) / 2;
    xfhtValues[3] = limit<int32_t>(-1024, ((f[2] & 0x07) - 1) * 1024, 1024);
    xfhtValues[4] = xfhtSigned(f[2] >> 3, 5) * 64;
  }
  xfhtLastFrame = get_tmr10ms();
  xfhtValid = true;
  return true;
}

void xfhtParseByte(uint8_t c)
{
  tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - xfhtLastByte) >= XFHT_FRAME_GAP_MS / 10)
    xfhtFrameIndex = 0;
  xfhtLastByte = now;

  xfhtFrame[xfhtFrameIndex++] = c;
  while (xfhtFrameIndex > 0) {
    bool header1 = xfhtFrame[0] == XFHT_PPM_HEADER1 ||
                   xfhtFrame[0] == XFHT_CSERIES_HEADER1;
    if (header1 && xfhtFrameIndex == 1) return;
    uint8_t length = header1 ? xfhtFrameLength(xfhtFrame[0], xfhtFrame[1]) : 0;
    if (length && xfhtFrameIndex < length) return;
    uint8_t used = (length && xfhtDecodeFrame(length)) ? length : 1;
    // after a bad CRC (a header-like pair inside the data) step one byte only:
    // C Series frames come too close together for the gap to resync
    xfhtFrameIndex -= used;
    memmove(xfhtFrame, xfhtFrame + used, xfhtFrameIndex);
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

int16_t get_xfht_value(uint8_t source)
{
  // a lost link centres the outputs rather than freezing the last head position
  if (source >= XFHT_SOURCE_COUNT || !xfhtValid ||
      (tmr10ms_t)(get_tmr10ms() - xfhtLastFrame) > XFHT_TIMEOUT_MS / 10) {
    return 0;
  }
  return xfhtValues[source];
}
