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

static inline bool pushFrskyTelemetryData(bool is_sport, uint8_t data,
                                          uint8_t* buffer, uint8_t& len)
{
  static uint8_t dataState = STATE_DATA_IDLE;

  switch (dataState) {
    case STATE_DATA_START:
      if (data == START_STOP) {
        if (is_sport) {
          dataState = STATE_DATA_IN_FRAME ;
          len = 0;
        }
      }
      else {
        if (len < TELEMETRY_RX_PACKET_SIZE) {
          buffer[len++] = data;
        }
        dataState = STATE_DATA_IN_FRAME;
      }
      break;

    case STATE_DATA_IN_FRAME:
      if (data == BYTE_STUFF) {
        dataState = STATE_DATA_XOR; // XOR next byte
      }
      else if (data == START_STOP) {
        if (is_sport) {
          dataState = STATE_DATA_IN_FRAME ;
          len = 0;
        } else {
          // end of frame detected
          dataState = STATE_DATA_IDLE;
          return true;
        }
        break;
      }
      else if (len < TELEMETRY_RX_PACKET_SIZE) {
        buffer[len++] = data;
      }
      break;

    case STATE_DATA_XOR:
      if (len < TELEMETRY_RX_PACKET_SIZE) {
        buffer[len++] = data ^ STUFF_MASK;
      }
      dataState = STATE_DATA_IN_FRAME;
      break;

    case STATE_DATA_IDLE:
      if (data == START_STOP) {
        len = 0;
        dataState = STATE_DATA_START;
      }
      break;

  } // switch

  if (is_sport && len >= FRSKY_SPORT_PACKET_SIZE) {
    // end of frame detected
    dataState = STATE_DATA_IDLE;
    return true;
  }

  return false;
}

void processFrskyDTelemetryData(uint8_t module, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  if (pushFrskyTelemetryData(false, data, buffer, *len)) {
    frskyDProcessPacket(module, buffer, *len);
  }
}

void processFrskySportTelemetryData(uint8_t module, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  if (pushFrskyTelemetryData(true, data, buffer, *len)) {
    sportProcessTelemetryPacket(module, buffer, *len);
  }
}
