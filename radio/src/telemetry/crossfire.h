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

#include <inttypes.h>
#include "dataconstants.h"

// Device address
#define BROADCAST_ADDRESS              0x00
#define RADIO_ADDRESS                  0xEA
#define MODULE_ADDRESS                 0xEE
#define RECEIVER_ADDRESS               0xEC

// Frame id
#define GPS_ID                         0x02
#define CF_VARIO_ID                    0x07
#define BATTERY_ID                     0x08
#define BARO_ALT_ID                    0x09
#define LINK_ID                        0x14
#define CHANNELS_ID                    0x16
#define LINK_RX_ID                     0x1C
#define LINK_TX_ID                     0x1D
#define ATTITUDE_ID                    0x1E
#define FLIGHT_MODE_ID                 0x21
#define PING_DEVICES_ID                0x28
#define DEVICE_INFO_ID                 0x29
#define REQUEST_SETTINGS_ID            0x2A
#define COMMAND_ID                     0x32
#define RADIO_ID                       0x3A

#define UART_SYNC                      0xC8
#define SUBCOMMAND_CRSF                0x10
#define COMMAND_MODEL_SELECT_ID        0x05
#define SUBCOMMAND_CRSF_BIND           0x01

constexpr uint8_t CRSF_NAME_MAXSIZE = 16;

struct CrossfireSensor {
  const uint8_t id;
  const uint8_t subId;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

enum CrossfireSensorIndexes {
  RX_RSSI1_INDEX,
  RX_RSSI2_INDEX,
  RX_QUALITY_INDEX,
  RX_SNR_INDEX,
  RX_ANTENNA_INDEX,
  RF_MODE_INDEX,
  TX_POWER_INDEX,
  TX_RSSI_INDEX,
  TX_QUALITY_INDEX,
  TX_SNR_INDEX,
  RX_RSSI_PERC_INDEX,
  RX_RF_POWER_INDEX,
  TX_RSSI_PERC_INDEX,
  TX_RF_POWER_INDEX,
  TX_FPS_INDEX,
  BATT_VOLTAGE_INDEX,
  BATT_CURRENT_INDEX,
  BATT_CAPACITY_INDEX,
  BATT_REMAINING_INDEX,
  GPS_LATITUDE_INDEX,
  GPS_LONGITUDE_INDEX,
  GPS_GROUND_SPEED_INDEX,
  GPS_HEADING_INDEX,
  GPS_ALTITUDE_INDEX,
  GPS_SATELLITES_INDEX,
  ATTITUDE_PITCH_INDEX,
  ATTITUDE_ROLL_INDEX,
  ATTITUDE_YAW_INDEX,
  FLIGHT_MODE_INDEX,
  VERTICAL_SPEED_INDEX,
  BARO_ALTITUDE_INDEX,
  UNKNOWN_INDEX,
};

enum CrossfireFrames{
  CRSF_FRAME_CHANNEL,
  CRSF_FRAME_MODELID,
  CRSF_FRAME_MODELID_SENT
};

struct CrossfireModuleStatus
{
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    char name[CRSF_NAME_MAXSIZE];
    bool queryCompleted;
    bool isELRS;
};

extern CrossfireModuleStatus crossfireModuleStatus[2];

void processCrossfireTelemetryFrame(uint8_t module, uint8_t* rxBuffer,
                                    uint8_t rxBufferCount);
void crossfireSetDefault(int index, uint8_t id, uint8_t subId);

const uint32_t CROSSFIRE_BAUDRATES[] = {
  115200,
  400000,
  921600,
  1870000,
  3750000,
  5250000,
};

#if defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_T20)
#define CROSSFIRE_MAX_INTERNAL_BAUDRATE     DIM(CROSSFIRE_BAUDRATES) - 3
#else
#define CROSSFIRE_MAX_INTERNAL_BAUDRATE     DIM(CROSSFIRE_BAUDRATES) - 1
#endif

const uint8_t CROSSFIRE_FRAME_PERIODS[] = {
  16,
  4,
  2,
  2,
  2,
  2,
};
#if SPORT_MAX_BAUDRATE < 400000
  // index 0 (115200) is the default 0 value
  #define CROSSFIRE_STORE_TO_INDEX(v) v
  #define CROSSFIRE_INDEX_TO_STORE(i) i
#else
  // index 1 (400000) is the default 0 value
  #define CROSSFIRE_DEFAULT_INDEX 1
  #define CROSSFIRE_STORE_TO_INDEX(v) \
    (v + CROSSFIRE_DEFAULT_INDEX) % DIM(CROSSFIRE_BAUDRATES)
  #define CROSSFIRE_INDEX_TO_STORE(i)                          \
    (i + (DIM(CROSSFIRE_BAUDRATES) - CROSSFIRE_DEFAULT_INDEX)) \
        % DIM(CROSSFIRE_BAUDRATES)
#endif

#define CRSF_ELRS_MIN_VER(moduleIdx, maj, min) \
        (crossfireModuleStatus[moduleIdx].isELRS \
         && crossfireModuleStatus[moduleIdx].major >= maj \
         && crossfireModuleStatus[moduleIdx].minor >= min)

#if defined(HARDWARE_INTERNAL_MODULE)
#define INT_CROSSFIRE_BR_IDX   CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate)
#define INT_CROSSFIRE_BAUDRATE CROSSFIRE_BAUDRATES[INT_CROSSFIRE_BR_IDX]
#define INT_CROSSFIRE_PERIOD   (CROSSFIRE_FRAME_PERIODS[INT_CROSSFIRE_BR_IDX] * 1000)
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
#define EXT_CROSSFIRE_BR_IDX   CROSSFIRE_STORE_TO_INDEX(g_model.moduleData[EXTERNAL_MODULE].crsf.telemetryBaudrate)
#define EXT_CROSSFIRE_BAUDRATE CROSSFIRE_BAUDRATES[EXT_CROSSFIRE_BR_IDX]
#define EXT_CROSSFIRE_PERIOD   (CROSSFIRE_FRAME_PERIODS[EXT_CROSSFIRE_BR_IDX] * 1000)
#endif

#if defined(HARDWARE_INTERNAL_MODULE) && defined(HARDWARE_EXTERNAL_MODULE)
#define CROSSFIRE_PERIOD(module) \
  (module == INTERNAL_MODULE ? INT_CROSSFIRE_PERIOD : EXT_CROSSFIRE_PERIOD)
#elif defined(HARDWARE_INTERNAL_MODULE)
#define CROSSFIRE_PERIOD(module) INT_CROSSFIRE_PERIOD
#elif defined(HARDWARE_EXTERNAL_MODULE)
#define CROSSFIRE_PERIOD(module) EXT_CROSSFIRE_PERIOD
#else
#define CROSSFIRE_PERIOD(module) 4000
#endif

#define CROSSFIRE_TELEM_MIRROR_BAUDRATE   115200
