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
#include "edgetx_types.h"

void processFlySkySensor(const uint8_t *packet, uint8_t type);

void processFlySkyTelemetryData(uint8_t data, uint8_t * rxBuffer, uint8_t &rxBufferCount);

void flySkySetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance);

// Used by multi protocol
void processFlySkyPacket(const uint8_t * packet);

void processFlySkyPacketAC(const uint8_t * packet);

int32_t getALT(uint32_t pressure);

// Vertical speed derived from barometric altitude samples, for pressure-only
// sensors such as the FS-CAT01 which do not report VSpd. Every sample is used:
// the raw estimate is the least-squares slope of altitude over the last
// WINDOW, followed by a light exponential filter. This tolerates the 0.5m
// altitude steps and sensor wander at rest, at the cost of ~1s of lag.
class FlySkyVSpeed
{
 public:
  static constexpr tmr10ms_t WINDOW = 100;        // 1s regression window
  static constexpr tmr10ms_t MIN_SPAN = 20;       // need 200ms of data
  static constexpr tmr10ms_t MAX_INTERVAL = 200;  // restart after 2s gap
  static constexpr uint8_t MAX_SAMPLES = 24;
  // Output filter weight of each new estimate, in tenths
  static constexpr int32_t FILTER_WEIGHT = 3;

  // alt in cm, now in 10ms ticks
  void update(int32_t alt, tmr10ms_t now);

  // Vertical speed in cm/s (i.e. m/s with precision 2)
  int32_t value() const { return vspdQ4 / 16; }

 private:
  struct Sample {
    tmr10ms_t time;
    int32_t alt;
  };
  Sample samples[MAX_SAMPLES];
  uint8_t first = 0;
  uint8_t count = 0;
  int32_t vspdQ4 = 0;  // filter state, cm/s * 16 to avoid integer stall

  const Sample& at(uint8_t i) const
  {
    return samples[(first + i) % MAX_SAMPLES];
  }
};
