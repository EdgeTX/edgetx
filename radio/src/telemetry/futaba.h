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

// The receiver's own values: sensor IDs 0x1300.., instance 0
enum {
  FUTABA_RX_VOLTAGE = 0x1300,  // slot 0: the receiver's battery
  FUTABA_RX_LQ = 0x1301,       // link quality % the receiver reports
  FUTABA_RX_BARS = 0x1302,     // slot 0: link quality bars, 0..3
  FUTABA_TX_RSSI = 0x1303,     // the reply's RSSI at the module, dBm
};

// S.BUS2 sensors: the sensor ID is the sensor type, the instance its first
// slot (1..31), subId the value for types with several (a sensor uses the
// slots from its first on). S.BUS2 sensors don't tell their type on the
// bus (their slot is set in the sensor): a sensor of one of these types
// claims its slots; a slot with data that no sensor claims shows up as a
// FUTABA_SLOT_RAW sensor (the 2 bytes as on the wire, first one high)
enum {
  FUTABA_SLOT_RAW = 0x1310,
  FUTABA_SBS_TEMP = 0x1311,     // SBS-01T: °C
  FUTABA_SBS_TEMP125 = 0x1312,  // SBS-01TE: °C
  FUTABA_SBS_RPM = 0x1313,      // SBS-01RM: rpm
  FUTABA_SBS_VOLT = 0x1314,     // SBS-01V, 2 slots: subId 0 / 1, V
  FUTABA_SBS_CURR = 0x1315,     // SBS-01C, 3 slots: subId 0 A, 1 V, 2 mAh
  FUTABA_SBS_LAST = FUTABA_SBS_CURR,
};

void futabaSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);

// data: first slot, RSSI (dBm, signed), link quality % (0xFF: none in this
// reply), then 2 bytes per slot
void processFutabaPacket(const uint8_t* data, uint8_t len);
