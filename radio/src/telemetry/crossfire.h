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

#include <memory>
#include <optional>
#include <array>

#include <inttypes.h>
#include "dataconstants.h"
#include "debug.h"

#if defined(LUA)
#include "fifo.h"
#if __cpp_lib_span
#include <span>
namespace etx {
  template<typename T>
  using span = std::span<T>;
}
#else
  namespace etx {
    template<typename T>
    struct span {
      using size_type = size_t;
      using value_type = T;
      size_t size() const {
        return mSize;
      }
      const T& operator[](size_t index) const {
        return mData[index];
      }
      const T* begin() const {
        return &mData[0];
      } 
      const T* end() const {
        return &mData[mSize];
      } 
      T* mData;
      size_t mSize;
    };
  }
#endif
#endif

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
         && (crossfireModuleStatus[moduleIdx].major > maj \
          || (crossfireModuleStatus[moduleIdx].major == maj \
           && crossfireModuleStatus[moduleIdx].minor >= min)))

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

#if defined(LUA)

template<auto N>
struct LuaTelemetryQueueManager {
  static inline constexpr uint8_t  numberOfQueues = N;
  static inline constexpr uint16_t fifoSize = 256;
  using fifo_t = Fifo<uint8_t, fifoSize>;
  using ptr_t = std::unique_ptr<fifo_t>;
  struct Entry {
    size_t id = 0;
    ptr_t fifo = nullptr;
    uint8_t filterLength = 0;
    std::array<uint8_t, 8> filter = {};
  };
  inline std::pair<uint8_t, Entry*> create(const size_t id) {
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (fifos[i].id == id) { // already there
        return {i, &fifos[i]};
      }
    }
    // if not there, create one if possible
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (!fifos[i].fifo) {
        fifos[i].id = id;
        fifos[i].fifo = std::make_unique<fifo_t>();
        return {i, &fifos[i]};
      }
    }
    return {-1, nullptr};
  }
  inline void remove(const size_t id) {
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (fifos[i].id == id) {
        fifos[i].id = 0;
        fifos[i].fifo = nullptr;
        fifos[i].filterLength = 0;
      }
    }
  }
  inline const Entry* get(const size_t id) const {
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (fifos[i].id == id) {
        return &fifos[i];
      }
      return nullptr;
    }
  }
  template<typename F>
  inline int pop(const size_t id, const F f){
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (fifos[i].id == id) {
        return f(fifos[i].fifo.get());
      }
    }
    return 0;
  }
  template<typename F>
  inline bool contains(const uint8_t length, const F getFilterValue){
    bool oneEqual = false;
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      bool equal = true;
      if (fifos[i].fifo && (fifos[i].filterLength == length)) {
        for(uint8_t n = 0; n < length; ++n) {
          const uint8_t a = getFilterValue(n);
          const uint8_t b = fifos[i].filter[n];
          if ((a != 0) && (b != 0) && (a != b)) {
            equal = false;
            break;
          }
        }
        if (equal) {
          oneEqual = true;
        }
      }
    }
    return oneEqual;
  }
  inline bool push(const etx::span<const uint8_t>& values) { // the packet values do not contain the start byte and the CRC
    TRACE("push");
    for(uint8_t i = 0; i < fifos.size(); ++i) {
      if (fifos[i].fifo) {
        TRACE("try: %d", i);
        bool match = true;
        for(uint8_t n = 0; n < fifos[i].filterLength; ++n) {
          TRACE("cmp: %d %d %d ", i, values[n], fifos[i].filter[n]);
          if ((values[n] != fifos[i].filter[n]) && (fifos[i].filter[n] != 0)) {
            match = false;
            break;
          }
        }
        if (match) {
          TRACE("match: %d", fifos[i].id);
          if (fifos[i].fifo->hasSpace(std::size(values))) {
            for(const uint8_t b : values) {
              fifos[i].fifo->push(b);
            }   
          }
          return true;
        }
      }
    }
    return false;
  }
  private:
  std::array<Entry, numberOfQueues> fifos = {};
};

using luaTelemetryQueueManager_t = LuaTelemetryQueueManager<8>;
#endif