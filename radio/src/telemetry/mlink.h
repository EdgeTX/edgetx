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
  MLINK_GRATE = 14,
  MLINK_RX_VOLTAGE = 16,   // out of range ID for specific RxBt treatment
  MLINK_LOSS = 17,         // out of range ID for handling number of loss
  MLINK_TX_RSSI = 18,      // out of range ID for handling Telemetry RSSi reported by multi
  MLINK_TX_LQI = 19,       // out of range ID for handling Telemetry LQI reported by multi
  MLINK_SPECIAL = 20,      // out of raneg ID fro handling special value class
};

#define MLINK_SVC 0         // ID speial value class - can't use in enum as indexes have to start at 0

void mlinkSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);

// Used by multi protocol
void processMLinkPacket(const uint8_t *packet, bool multi);

// used by telemetry driver
#define MSB_EXT_MODULE_PACKET_LEN  18   // packet length minus start and stop byte
#define MSB_STX             0x02        // start byte
#define MSB_ETX             0x03        // stop byte
#define MSB_STUFF_ESC       0x1B        // ESC symbol
#define MSB_STUFF_OFFSET    0x20        // byte stuffing offset
#define MSB_VALID_TELEMETRY 0x13
#define MSB_NORMAL          0x06
#define MSB_NORMAL_FAST     0x04
#define MSB_RANGE           0x46       
#define MSB_RANGE_FAST      0x44

// used by external MLink module driver
#define PPM_MSB_BAUDRATE  115200

void processExternalMLinkSerialData(uint8_t module, uint8_t data,
                                    uint8_t* buffer, uint8_t* len);
