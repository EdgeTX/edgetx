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

//
// Structured source reference (replaces MixSources enum in storage)
//
// 32-bit word-aligned: type(8) + flags(8) + index(16)
//
// The old MixSources enum packed all source types into a single
// linear numbering space stored in 10-bit signed fields. This
// overflows on H7 radios with 99 telemetry sensors.
//
// SourceRef separates the type from the index, giving each type
// its own 0-65535 index range.
//

enum SourceType : uint8_t {
  SOURCE_TYPE_NONE = 0,
  SOURCE_TYPE_INPUT,          // Input lines (0..MAX_INPUTS-1)
  SOURCE_TYPE_LUA,            // Lua script outputs
  SOURCE_TYPE_STICK,          // Semantic sticks (0..MAX_STICKS-1)
  SOURCE_TYPE_POT,            // Pots and sliders (0..MAX_POTS-1)
  SOURCE_TYPE_IMU,            // Tilt X/Y
  SOURCE_TYPE_SPACEMOUSE,     // Spacemouse A-F
  SOURCE_TYPE_MIN,            // MIN constant
  SOURCE_TYPE_MAX,            // MAX constant
  SOURCE_TYPE_HELI,           // CYC1/2/3
  SOURCE_TYPE_TRIM,           // Trim values (0..MAX_TRIMS-1)
  SOURCE_TYPE_SWITCH,         // Physical switches (0..MAX_SWITCHES-1)
  SOURCE_TYPE_CUSTOM_SWITCH_GROUP, // Function switch groups
  SOURCE_TYPE_LOGICAL_SWITCH, // Logical switches (0..MAX_LOGICAL_SWITCHES-1)
  SOURCE_TYPE_TRAINER,        // Trainer channels
  SOURCE_TYPE_CHANNEL,        // Output channels (0..MAX_OUTPUT_CHANNELS-1)
  SOURCE_TYPE_GVAR,           // Global variables (0..MAX_GVARS-1)
  SOURCE_TYPE_TX_VOLTAGE,
  SOURCE_TYPE_TX_TIME,
  SOURCE_TYPE_TX_GPS,
  SOURCE_TYPE_TIMER,          // Timers (0..MAX_TIMERS-1)
  SOURCE_TYPE_TELEMETRY,      // Telemetry sensors (0..3*MAX_TELEMETRY_SENSORS-1)
  SOURCE_TYPE_LIGHT,          // Luminosity sensor
  SOURCE_TYPE_LAST
};

enum SourceFlags : uint8_t {
  SOURCE_FLAG_NONE     = 0,
  SOURCE_FLAG_INVERTED = (1 << 0),  // Negate the source value (replaces negative srcRaw)
};

struct SourceRef {
  uint8_t  type;   // SourceType
  uint8_t  flags;  // SourceFlags
  uint16_t index;  // Index within the type (0-based)

  bool isNone() const { return type == SOURCE_TYPE_NONE; }
  bool isInverted() const { return flags & SOURCE_FLAG_INVERTED; }

  void clear() { type = SOURCE_TYPE_NONE; flags = 0; index = 0; }

  bool operator==(const SourceRef& other) const {
    return type == other.type && flags == other.flags && index == other.index;
  }
  bool operator!=(const SourceRef& other) const { return !(*this == other); }
};

static_assert(sizeof(SourceRef) == 4, "SourceRef must be 32 bits");

//
// Structured switch reference (replaces SwitchSources enum in storage)
//

enum SwitchRefType : uint8_t {
  SWITCH_TYPE_NONE = 0,
  SWITCH_TYPE_SWITCH,         // Physical switches (index = position within switch array)
  SWITCH_TYPE_MULTIPOS,       // Multipos switch positions
  SWITCH_TYPE_TRIM,           // Trim switches
  SWITCH_TYPE_LOGICAL,        // Logical switches (0..MAX_LOGICAL_SWITCHES-1)
  SWITCH_TYPE_ON,             // Always ON
  SWITCH_TYPE_ONE,            // ON for one cycle
  SWITCH_TYPE_FLIGHT_MODE,    // Flight mode active
  SWITCH_TYPE_TELEMETRY,      // Telemetry streaming
  SWITCH_TYPE_SENSOR,         // Sensor active
  SWITCH_TYPE_RADIO_ACTIVITY, // Radio activity
  SWITCH_TYPE_TRAINER,        // Trainer connected
  SWITCH_TYPE_LAST
};

enum SwitchRefFlags : uint8_t {
  SWITCH_FLAG_NONE     = 0,
  SWITCH_FLAG_INVERTED = (1 << 0),  // Invert the switch state (replaces negative swtch)
};

struct SwitchRef {
  uint8_t  type;   // SwitchRefType
  uint8_t  flags;  // SwitchRefFlags
  uint16_t index;  // Index within the type (0-based)

  bool isNone() const { return type == SWITCH_TYPE_NONE; }
  bool isInverted() const { return flags & SWITCH_FLAG_INVERTED; }

  void clear() { type = SWITCH_TYPE_NONE; flags = 0; index = 0; }

  bool operator==(const SwitchRef& other) const {
    return type == other.type && flags == other.flags && index == other.index;
  }
  bool operator!=(const SwitchRef& other) const { return !(*this == other); }
};

static_assert(sizeof(SwitchRef) == 4, "SwitchRef must be 32 bits");

//
// Conversion functions (for gradual migration)
//
// These convert between the old enum-based representation (mixsrc_t / swsrc_t)
// and the new structured representation. During migration, both representations
// coexist: old code uses the enums, new code uses SourceRef/SwitchRef.
//

// Forward declarations - implemented in sourceref.cpp
// (require dataconstants.h which depends on board.h)
struct SourceRef sourceRefFromMixSrc(int32_t mixsrc);
int32_t mixSrcFromSourceRef(const struct SourceRef& ref);

struct SwitchRef switchRefFromSwSrc(int32_t swsrc);
int32_t swSrcFromSwitchRef(const struct SwitchRef& ref);
