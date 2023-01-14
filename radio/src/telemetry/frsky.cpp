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

#include "opentx.h"
#include "aux_serial_driver.h"

void processFrskyTelemetryData(uint8_t data)
{
  if (pushFrskyTelemetryData(data)) {
    // if (IS_FRSKY_SPORT_PROTOCOL()) {
    sportProcessTelemetryPacket(telemetryRxBuffer);
    // }
    // else {
    //   frskyDProcessPacket(telemetryRxBuffer);
    // }
  }
}

bool pushFrskyTelemetryData(uint8_t data)
{
  static uint8_t dataState = STATE_DATA_IDLE;

  switch (dataState) {
    case STATE_DATA_START:
      if (data == START_STOP) {
        // if (IS_FRSKY_SPORT_PROTOCOL()) {
        dataState = STATE_DATA_IN_FRAME ;
        telemetryRxBufferCount = 0;
        // }
      }
      else {
        if (telemetryRxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
          telemetryRxBuffer[telemetryRxBufferCount++] = data;
        }
        dataState = STATE_DATA_IN_FRAME;
      }
      break;

    case STATE_DATA_IN_FRAME:
      if (data == BYTE_STUFF) {
        dataState = STATE_DATA_XOR; // XOR next byte
      }
      else if (data == START_STOP) {
        // if (IS_FRSKY_SPORT_PROTOCOL()) {
        dataState = STATE_DATA_IN_FRAME ;
        telemetryRxBufferCount = 0;
        // }
        // else {
        //   // end of frame detected
        //   dataState = STATE_DATA_IDLE;
        //   return true;
        // }
        break;
      }
      else if (telemetryRxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
        telemetryRxBuffer[telemetryRxBufferCount++] = data;
      }
      break;

    case STATE_DATA_XOR:
      if (telemetryRxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
        telemetryRxBuffer[telemetryRxBufferCount++] = data ^ STUFF_MASK;
      }
      dataState = STATE_DATA_IN_FRAME;
      break;

    case STATE_DATA_IDLE:
      if (data == START_STOP) {
        telemetryRxBufferCount = 0;
        dataState = STATE_DATA_START;
      }
      break;

  } // switch

  if (/*IS_FRSKY_SPORT_PROTOCOL() &&*/ telemetryRxBufferCount >= FRSKY_SPORT_PACKET_SIZE) {
    // end of frame detected
    dataState = STATE_DATA_IDLE;
    return true;
  }

  return false;
}


