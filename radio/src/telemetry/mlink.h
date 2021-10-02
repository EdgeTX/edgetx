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

#pragma once

enum {
  MLINK_VOLTAGE = 1,
  MLINK_CURRENT = 2,
  MLINK_VARIO = 3,
  MLINK_SPEED = 4,
  MLINK_RPM = 5,
  MLINK_TEMP = 6,
  MLINK_HEADING = 7,
  MLINK_ALT = 8,
  MLINK_FUEL = 9,
  MLINK_LQI = 10,
  MLINK_CAPACITY = 11,
  MLINK_FLOW = 12,
  MLINK_DISTANCE = 13,
  MLINK_RX_VOLTAGE = 16,   // out of range ID for specific RxBt treatment
  MLINK_LOSS = 17,         // out of range ID for handling number of loss
  MLINK_TX_RSSI = 18,      // out of range ID for handling Telemetry RSSi reported by multi
  MLINK_TX_LQI = 19,       // out of range ID for handling Telemetry LQI reported by multi
};

void processMLinkTelemetryData(uint8_t data, uint8_t* rxBuffer, uint8_t& rxBufferCount);
void mlinkSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);

// Used by multi protocol
void processMLinkPacket(const uint8_t *packet);
