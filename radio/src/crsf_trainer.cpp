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

#include "crsf_trainer.h"

#include <string.h>

#include "crc.h"
#include "edgetx.h"
#include "telemetry/crossfire.h"
#include "telemetry/telemetry.h"

//
// CRSF frame: [addr][len][type][payload...][crc8]
//
// len counts type + payload + crc, so the frame occupies len + 2 bytes.
// The CRC covers type + payload, i.e. bytes [2 .. len].
//
// A frame may be split over several USB packets and several frames may arrive
// in one, so the assembler keeps its partial-frame state across chunks.
//
// Invariant: _crsf_len == 0, or _crsf_buf[0] is a valid address byte.
//
#define CRSF_MIN_FRAME_LEN 3

static uint8_t _crsf_buf[TELEMETRY_RX_PACKET_SIZE];
static uint8_t _crsf_len = 0;

static const etx_serial_driver_t* _crsf_drv = nullptr;
static void* _crsf_ctx = nullptr;

static inline bool _validAddr(uint8_t b)
{
  return b == RADIO_ADDRESS || b == UART_SYNC;
}

static inline bool _lenIsSane(uint32_t len)
{
  // at least type + payload + crc, and must fit the buffer
  return len > 2 && len < TELEMETRY_RX_PACKET_SIZE - 1;
}

static bool _checkCRC(const uint8_t* frame)
{
  uint8_t len = frame[1];
  return crc8(&frame[2], len - 1) == frame[len + 1];
}

// Drop the leading byte of a rejected frame and re-sync on the next valid
// address byte held in what is left. Never discards the whole buffer: a valid
// frame may well have started inside it.
static void _resync()
{
  uint8_t i = 1;
  while (i < _crsf_len && !_validAddr(_crsf_buf[i])) i++;

  _crsf_len -= i;
  if (_crsf_len > 0) {
    memmove(_crsf_buf, _crsf_buf + i, _crsf_len);
  }
}

void crsfTrainerReceiveData(uint8_t* data, uint32_t len)
{
  // The model is not asking for CRSF trainer input: stay out of the way.
  if (g_model.trainerData.mode != TRAINER_MODE_CRSF) {
    _crsf_len = 0;
    return;
  }

  while (len > 0) {
    // Hunt for an address byte while no frame is being assembled
    if (_crsf_len == 0 && !_validAddr(*data)) {
      data++; len--;
      continue;
    }

    _crsf_buf[_crsf_len++] = *data++;
    len--;

    // Need the length byte before the frame size is known
    if (_crsf_len < 2) continue;

    uint32_t pkt_len = (uint32_t)_crsf_buf[1] + 2;
    if (!_lenIsSane(pkt_len)) {
      // Bogus length: this was not a frame start after all
      _resync();
      continue;
    }

    if (_crsf_len < pkt_len) continue;  // incomplete, wait for more bytes

    if (_checkCRC(_crsf_buf) && _crsf_buf[2] == CHANNELS_ID) {
      // Fills trainerInput[] and resets the trainer validity timer
      crossfireProcessChannelsFrame(_crsf_buf);
    }

    // Consume the whole frame either way, as _processFrames() does for the
    // module path. With a valid address and a sane length, a CRC failure is
    // far more likely to be corruption inside a real frame than a false lock,
    // so dropping the frame keeps the stream aligned; byte-wise re-sync would
    // instead risk latching onto a payload byte that happens to look like an
    // address. Frame types other than CHANNELS_ID are consumed and ignored.
    _crsf_len = 0;
  }
}

void crsfTrainerStart(void* ctx, const etx_serial_driver_t* drv)
{
  if (!drv || !drv->setReceiveCb) return;

  _crsf_len = 0;
  _crsf_drv = drv;
  _crsf_ctx = ctx;

  drv->setReceiveCb(ctx, crsfTrainerReceiveData);
}

void crsfTrainerStop()
{
  auto drv = _crsf_drv;
  auto ctx = _crsf_ctx;

  _crsf_drv = nullptr;
  _crsf_ctx = nullptr;
  _crsf_len = 0;

  // Release the RX stream, so the next user of the port gets it
  if (drv && drv->setReceiveCb) drv->setReceiveCb(ctx, nullptr);
}
