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

#include "sbus.h"

#include <string.h>

#include "edgetx.h"
#include "timers_driver.h"

#define SBUS_FRAME_SIZE 25
#define SBUS_START_BYTE 0x0F
#define SBUS_END_BYTE 0x00
#define SBUS_FLAGS_IDX 23
#define SBUS_FRAMELOST_BIT 2
#define SBUS_FAILSAFE_BIT 3

#define SBUS_CH_BITS 11
#define SBUS_CH_MASK ((1 << SBUS_CH_BITS) - 1)

#define SBUS_CH_CENTER 0x3E0

static const etx_serial_driver_t* _sbus_drv = nullptr;
static void* _sbus_ctx = nullptr;
static bool _sbus_aux_enabled = false;

static void sbusProcessFrame(int16_t* pulses, uint8_t* sbus, uint32_t size);

// The last byte of a frame is 0x00 for Futaba / FrSky, but some
// implementations use it to carry frame flags (0x04 / 0x14 / 0x24).
static inline bool sbusIsEndByte(uint8_t b)
{
  return b == SBUS_END_BYTE || b == 0x04 || b == 0x14 || b == 0x24;
}

void sbusSetReceiveCtx(void* ctx, const etx_serial_driver_t* drv)
{
  _sbus_ctx = ctx;
  _sbus_drv = drv;
}

void sbusAuxFrameReceived(void*)
{
  if (!_sbus_aux_enabled) return;
  sbusFrameReceived(nullptr);
}

void sbusAuxSetEnabled(bool enabled) { _sbus_aux_enabled = enabled; }

void sbusFrameReceived(void*)
{
  if (!_sbus_drv || !_sbus_ctx || !_sbus_drv->copyRxBuffer ||
      !_sbus_drv->getBufferedBytes)
    return;

  if (_sbus_drv->getBufferedBytes(_sbus_ctx) != SBUS_FRAME_SIZE) {
    _sbus_drv->clearRxBuffer(_sbus_ctx);
    return;
  }

  uint8_t frame[SBUS_FRAME_SIZE];
  int received = _sbus_drv->copyRxBuffer(_sbus_ctx, frame, SBUS_FRAME_SIZE);
  if (received < 0) return;

  sbusProcessFrame(trainerInput, frame, received);
}

//
// Byte-stream framer, for ports with no idle-line detection (USB-VCP).
//
// USB CDC gives no frame boundaries: a 25 byte frame may be split over several
// packets, and several frames may arrive in one packet. So frames have to be
// recovered from the stream itself.
//
// Invariant: _sbus_stream_len == 0, or _sbus_stream_buf[0] == SBUS_START_BYTE.
//
static uint8_t _sbus_stream_buf[SBUS_FRAME_SIZE];
static uint8_t _sbus_stream_len = 0;

static const etx_serial_driver_t* _sbus_stream_drv = nullptr;
static void* _sbus_stream_ctx = nullptr;

// Drop the leading byte of a rejected frame and re-sync on the next start byte
// found in what is left. Never drops the whole buffer: a valid frame may well
// have started inside it.
static void sbusStreamResync()
{
  uint8_t i = 1;
  while (i < _sbus_stream_len && _sbus_stream_buf[i] != SBUS_START_BYTE) i++;

  _sbus_stream_len -= i;
  if (_sbus_stream_len > 0) {
    memmove(_sbus_stream_buf, _sbus_stream_buf + i, _sbus_stream_len);
  }
}

void sbusStreamReceiveData(uint8_t* data, uint32_t len)
{
  // Trainer mode is not asking for serial input: stay out of the way.
  if (!_sbus_aux_enabled) {
    _sbus_stream_len = 0;
    return;
  }

  while (len > 0) {
    // Hunt for a start byte while no frame is being assembled
    if (_sbus_stream_len == 0 && *data != SBUS_START_BYTE) {
      data++; len--;
      continue;
    }

    _sbus_stream_buf[_sbus_stream_len++] = *data++;
    len--;

    if (_sbus_stream_len < SBUS_FRAME_SIZE) continue;

    if (sbusIsEndByte(_sbus_stream_buf[SBUS_FRAME_SIZE - 1])) {
      // Complete frame. sbusProcessFrame() re-checks it, and resets the
      // trainer validity timer if it is accepted.
      sbusProcessFrame(trainerInput, _sbus_stream_buf, SBUS_FRAME_SIZE);
      _sbus_stream_len = 0;
    } else {
      sbusStreamResync();
    }
  }
}

void sbusStreamStart(void* ctx, const etx_serial_driver_t* drv)
{
  if (!drv || !drv->setReceiveCb) return;

  _sbus_stream_len = 0;
  _sbus_stream_drv = drv;
  _sbus_stream_ctx = ctx;

  drv->setReceiveCb(ctx, sbusStreamReceiveData);
}

void sbusStreamStop()
{
  auto drv = _sbus_stream_drv;
  auto ctx = _sbus_stream_ctx;

  _sbus_stream_drv = nullptr;
  _sbus_stream_ctx = nullptr;
  _sbus_stream_len = 0;

  // Release the RX stream, so the next user of the port gets it
  if (drv && drv->setReceiveCb) drv->setReceiveCb(ctx, nullptr);
}

// Range for pulses (ppm input) is [-512:+512]
static void sbusProcessFrame(int16_t* pulses, uint8_t* sbus, uint32_t size)
{
  if (size != SBUS_FRAME_SIZE || sbus[0] != SBUS_START_BYTE ||
      !sbusIsEndByte(sbus[SBUS_FRAME_SIZE - 1])) {
    return;  // not a valid SBUS frame
  }
  if ((sbus[SBUS_FLAGS_IDX] & (1 << SBUS_FAILSAFE_BIT)) ||
      (sbus[SBUS_FLAGS_IDX] & (1 << SBUS_FRAMELOST_BIT))) {
    return;  // SBUS invalid frame or failsafe mode
  }

  sbus++;  // skip start byte

  uint32_t inputbitsavailable = 0;
  uint32_t inputbits = 0;
  for (uint32_t i = 0; i < MAX_TRAINER_CHANNELS; i++) {
    while (inputbitsavailable < SBUS_CH_BITS) {
      inputbits |= *sbus++ << inputbitsavailable;
      inputbitsavailable += 8;
    }
    *pulses++ = ((int32_t)(inputbits & SBUS_CH_MASK) - SBUS_CH_CENTER) * 5 / 8;
    inputbitsavailable -= SBUS_CH_BITS;
    inputbits >>= SBUS_CH_BITS;
  }

  trainerResetTimer();
}
