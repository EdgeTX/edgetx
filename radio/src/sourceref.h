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
  bool operator<(const SourceRef& other) const {
    if (type != other.type) return type < other.type;
    return index < other.index;
  }
};

static_assert(sizeof(SourceRef) == 4, "SourceRef must be 32 bits");

//
// Value-or-source union (replaces SourceNumVal 11-bit packed field)
//
// Used for MixData.weight, MixData.offset, ExpoData.weight, ExpoData.offset,
// and CurveRef.value — fields that can hold either a numeric value or a
// source reference.
//
// 4 bytes, 32-bit aligned. When isSource=1, `value` is reinterpreted as the
// source index and `srcType` identifies the source type.
//
struct ValueOrSource {
  int16_t value;     // numeric value, or source index when isSource=1
  uint8_t isSource;  // 0 = numeric value, 1 = source reference
  uint8_t srcType;   // SourceType (only valid when isSource=1)

  // Access as numeric value
  int16_t numericValue() const { return value; }
  void setNumeric(int16_t v) { value = v; isSource = 0; srcType = 0; }

  // Access as source reference
  SourceRef toSourceRef() const {
    SourceRef ref = {};
    if (isSource) {
      ref.type = srcType;
      ref.index = static_cast<uint16_t>(value);
    }
    return ref;
  }

  void setSource(const SourceRef& ref) {
    isSource = 1;
    srcType = ref.type;
    value = static_cast<int16_t>(ref.index);
    // Note: flags (inversion) not stored here — weight/offset inversion
    // is handled separately via negative numeric values
  }

  void clear() { value = 0; isSource = 0; srcType = 0; }
};

static_assert(sizeof(ValueOrSource) == 4, "ValueOrSource must be 32 bits");

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

