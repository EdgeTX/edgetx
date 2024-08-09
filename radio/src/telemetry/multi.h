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

#include "timers_driver.h"

#define MULTI_BUFFER_SIZE 177
extern uint8_t * Multi_Buffer;

/*
  Multiprotocol telemetry definition

  Serial: 100000 Baud 8e2 (same as input)

  TLV Protocol (type, length, value), allows a TX to ignore unknown messages

  Format: header (4 byte) + data (variable)

   [0] = 'M' (0x4d)
   [1] = 'P' (0x50)


   The first byte is deliberatly chosen to be different from other telemetry protocols
   (e.g. 0xAA for DSM/Multi, 0xAA for FlySky and 0x7e for Frsky) to allow a TX to detect
   the telemetry format of older versions

   [2] Type (see below)
   [3] Length (excluding the 4 header bytes)

   [4-xx] data


Type = 0x01 Multimodule Status:
   [4] Flags
   0x01 = Input signal detected
   0x02 = Serial mode enabled
   0x04 = protocol is valid
   0x08 = module is in binding mode
   [5] major
   [6] minor
   [7] revision
   [8] patchlevel,
   version of multi code, should be displayed as major.minor.revision.patchlevel

   more information can be added by specifying a longer length of the type, the TX will just ignore these bytes


Type 0x02 Frksy S.port telemetry
Type 0x03 Frsky Hub telemetry

	*No* usual frsky byte stuffing and without start/stop byte (0x7e)


Type 0x04 Spektrum telemetry data
   data[0] RSSI
   data[1-15] telemetry data

Type 0x05 DSM bind data
	data[0-16] DSM bind data

    technically DSM bind data is only 10 bytes but multi send 16
    like with telemtry, check length field)

Type 0x06 Flysky AFHDS2 telemetry data
   length: 29
   data[0] = RSSI value
   data[1-28] telemetry data

Type 0x0B Spectrum Scanner telemetry data
   length: 6
   data[0] = start channel (2400 + x*0.333 Mhz)
   data[1-5] power levels

*/

void processMultiTelemetryData(uint8_t data, uint8_t module);

#define MULTI_SCANNER_MAX_CHANNEL 249

struct MultiModuleStatus {

  uint8_t major;
  uint8_t minor;
  uint8_t revision;
  uint8_t patch;
  uint8_t ch_order;

  uint8_t flags;
  uint8_t requiresFailsafeCheck;
  uint8_t failsafeChecked;
  uint8_t isRXProto;
  tmr10ms_t lastUpdate;

  uint8_t protocolPrev = 0;
  uint8_t protocolNext = 0;
  char protocolName[8] = {0};
  uint8_t protocolSubNbr = 0;
  char protocolSubName[9] = {0};
  uint8_t optionDisp = 0;

  void getStatusString(char * statusText) const;

  uint8_t getModuleIndex() const;

  inline bool isValid() const { return (bool)(get_tmr10ms() - lastUpdate < 200); }
  inline bool isBufferFull() const { return (bool) (flags & 0x80); }
  inline bool supportsDisableMapping() const { return (bool) (flags & 0x40); }
  inline bool supportsFailsafe() const { return (bool) (flags & 0x20); }
  inline bool isWaitingforBind() const { return (bool) (flags & 0x10); }
  inline bool isBinding() const { return (bool) (flags & 0x08); }
  inline bool protocolValid() const { return (bool) (flags & 0x04); }
  inline bool serialMode() const { return (bool) (flags & 0x02); }
  inline bool inputDetected() const { return (bool) (flags & 0x01); }

  inline void invalidate() {
    lastUpdate = get_tmr10ms() - 500;
    protocolSubNbr = 0;
    protocolName[0] = '\0';
    protocolSubName[0] = '\0';
  }
};

MultiModuleStatus& getMultiModuleStatus(uint8_t module);

enum MultiBindStatus : uint8_t {
  MULTI_BIND_NONE,
  MULTI_BIND_INITIATED,
  MULTI_BIND_FINISHED,
};

uint8_t getMultiBindStatus(uint8_t module);
void setMultiBindStatus(uint8_t module, uint8_t bindStatus);

bool isMultiModeScanning(uint8_t module);
bool isMultiTelemReceiving(uint8_t module);

void checkFailsafeMulti();
