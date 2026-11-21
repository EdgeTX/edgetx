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

#include <stdint.h>

// Futaba receiver telemetry (T-FHSS, FASSTest) from the MULTI-Module: the
// receiver's S.BUS2 slots as the sensors put them on the wire
// (MULTI_TELEMETRY_FUTABA in Multiprotocol.h)

// Sensor IDs: 0x1300 + n (0x13 = the MULTI telemetry type)
enum {
  FUTABA_RX_VOLTAGE = 0x1300,  // slot 0: the receiver's battery
  FUTABA_RX_LQ = 0x1301,       // link quality % the receiver reports
  FUTABA_RX_BARS = 0x1302,     // slot 0: link quality bars, 0..3
  FUTABA_TX_RSSI = 0x1303,     // the reply's RSSI at the module, dBm
};

void futabaSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);

// data: first slot, RSSI (dBm, signed), link quality % (0xFF: none in this
// reply), then 2 bytes per slot
void processFutabaPacket(const uint8_t* data, uint8_t len);
