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

#include "boards.h"
#include "constants.h"
#include "datahelpers.h"

#include <QtCore>

class Firmware;
class ModelData;
class GeneralSettings;
class RadioDataConversionState;

// TODO remove and refactor to use dynamic board switches
enum Switches {
  SWITCH_NONE,

  SWITCH_THR = SWITCH_NONE+1,
  SWITCH_RUD,
  SWITCH_ELE,
  SWITCH_ID0,
  SWITCH_ID1,
  SWITCH_ID2,
  SWITCH_AIL,
  SWITCH_GEA,
  SWITCH_TRN,

  SWITCH_SA0 = SWITCH_NONE+1,
  SWITCH_SA1,
  SWITCH_SA2,
  SWITCH_SB0,
  SWITCH_SB1,
  SWITCH_SB2,
  SWITCH_SC0,
  SWITCH_SC1,
  SWITCH_SC2,
  SWITCH_SD0,
  SWITCH_SD1,
  SWITCH_SD2,
  SWITCH_SE0,
  SWITCH_SE1,
  SWITCH_SE2,
  SWITCH_SF0,
  SWITCH_SF1,
  SWITCH_SG0,
  SWITCH_SG1,
  SWITCH_SG2,
  SWITCH_SH0,
  SWITCH_SH2,
  SWITCH_SI0,
  SWITCH_SI2,
  SWITCH_SJ0,
  SWITCH_SJ2,
  SWITCH_SK0,
  SWITCH_SK2,
};

enum TimerModes {
  TMRMODE_NONE,
  TMRMODE_ABS,
  TMRMODE_THR,
  TMRMODE_THR_REL,
  TMRMODE_THR_TRG,
  TMRMODE_FIRST_SWITCH
};

enum FailsafeModes {
  FAILSAFE_NOT_SET,
  FAILSAFE_HOLD,
  FAILSAFE_CUSTOM,
  FAILSAFE_NOPULSES,
  FAILSAFE_RECEIVER,
  FAILSAFE_LAST = FAILSAFE_RECEIVER
};

enum HeliSwashTypes {
  HELI_SWASH_TYPE_NONE=0,
  HELI_SWASH_TYPE_120,
  HELI_SWASH_TYPE_120X,
  HELI_SWASH_TYPE_140,
  HELI_SWASH_TYPE_90
};

enum TelemetrySource {
  TELEMETRY_SOURCE_NONE,
  TELEMETRY_SOURCE_TX_BATT,
  TELEMETRY_SOURCE_TX_TIME,
  TELEMETRY_SOURCE_TIMER1,
  TELEMETRY_SOURCE_TIMER2,
  TELEMETRY_SOURCE_TIMER3,
  TELEMETRY_SOURCE_RAS,
  TELEMETRY_SOURCE_RSSI_TX,
  TELEMETRY_SOURCE_RSSI_RX,
  TELEMETRY_SOURCE_A1,
  TELEMETRY_SOURCE_A2,
  TELEMETRY_SOURCE_A3,
  TELEMETRY_SOURCE_A4,
  TELEMETRY_SOURCE_ALT,
  TELEMETRY_SOURCE_RPM,
  TELEMETRY_SOURCE_FUEL,
  TELEMETRY_SOURCE_T1,
  TELEMETRY_SOURCE_T2,
  TELEMETRY_SOURCE_SPEED,
  TELEMETRY_SOURCE_DIST,
  TELEMETRY_SOURCE_GPS_ALT,
  TELEMETRY_SOURCE_CELL,
  TELEMETRY_SOURCE_CELLS_SUM,
  TELEMETRY_SOURCE_VFAS,
  TELEMETRY_SOURCE_CURRENT,
  TELEMETRY_SOURCE_CONSUMPTION,
  TELEMETRY_SOURCE_POWER,
  TELEMETRY_SOURCE_ACCX,
  TELEMETRY_SOURCE_ACCY,
  TELEMETRY_SOURCE_ACCZ,
  TELEMETRY_SOURCE_HDG,
  TELEMETRY_SOURCE_VERTICAL_SPEED,
  TELEMETRY_SOURCE_ASPEED,
  TELEMETRY_SOURCE_DTE,
  TELEMETRY_SOURCE_A1_MIN,
  TELEMETRY_SOURCE_A2_MIN,
  TELEMETRY_SOURCE_A3_MIN,
  TELEMETRY_SOURCE_A4_MIN,
  TELEMETRY_SOURCE_ALT_MIN,
  TELEMETRY_SOURCE_ALT_MAX,
  TELEMETRY_SOURCE_RPM_MAX,
  TELEMETRY_SOURCE_T1_MAX,
  TELEMETRY_SOURCE_T2_MAX,
  TELEMETRY_SOURCE_SPEED_MAX,
  TELEMETRY_SOURCE_DIST_MAX,
  TELEMETRY_SOURCE_ASPEED_MAX,
  TELEMETRY_SOURCE_CELL_MIN,
  TELEMETRY_SOURCE_CELLS_MIN,
  TELEMETRY_SOURCE_VFAS_MIN,
  TELEMETRY_SOURCE_CURRENT_MAX,
  TELEMETRY_SOURCE_POWER_MAX,
  TELEMETRY_SOURCE_ACC,
  TELEMETRY_SOURCE_GPS_TIME,
  TELEMETRY_SOURCES_STATUS_COUNT = TELEMETRY_SOURCE_GPS_TIME+1,
  TELEMETRY_SOURCES_DISPLAY_COUNT = TELEMETRY_SOURCE_POWER_MAX+1,
  TELEMETRY_SOURCES_COUNT = TELEMETRY_SOURCE_POWER_MAX+1,
  TELEMETRY_SOURCE_RESERVE = -1
};

enum RawSourceType {
  SOURCE_TYPE_NONE,
  SOURCE_TYPE_VIRTUAL_INPUT,
  SOURCE_TYPE_LUA_OUTPUT,
  SOURCE_TYPE_INPUT,
  SOURCE_TYPE_ROTARY_ENCODER,
  SOURCE_TYPE_TRIM,
  SOURCE_TYPE_MIN,
  SOURCE_TYPE_MAX,
  SOURCE_TYPE_SWITCH,
  SOURCE_TYPE_FUNCTIONSWITCH, // v2.10 only used for reading binary files
  SOURCE_TYPE_CUSTOM_SWITCH,
  SOURCE_TYPE_CYC,
  SOURCE_TYPE_PPM,
  SOURCE_TYPE_CH,
  SOURCE_TYPE_GVAR,
  SOURCE_TYPE_SPECIAL,
  SOURCE_TYPE_TELEMETRY,
  SOURCE_TYPE_SPACEMOUSE,
  SOURCE_TYPE_TIMER,
  SOURCE_TYPE_FUNCTIONSWITCH_GROUP,
  MAX_SOURCE_TYPE
};

enum RawSourceTypeSpecial {
  SOURCE_TYPE_SPECIAL_NONE,
  SOURCE_TYPE_SPECIAL_TX_BATT,
  SOURCE_TYPE_SPECIAL_TX_TIME,
  SOURCE_TYPE_SPECIAL_TX_GPS,
  SOURCE_TYPE_SPECIAL_FIRST_RESERVED,
  SOURCE_TYPE_SPECIAL_RESERVED1 = SOURCE_TYPE_SPECIAL_FIRST_RESERVED,
  SOURCE_TYPE_SPECIAL_RESERVED2,
  SOURCE_TYPE_SPECIAL_RESERVED3,
  SOURCE_TYPE_SPECIAL_RESERVED4,
  SOURCE_TYPE_SPECIAL_LAST_RESERVED = SOURCE_TYPE_SPECIAL_RESERVED4,
  SOURCE_TYPE_SPECIAL_COUNT
};

constexpr int SOURCE_TYPE_INPUT_THR_IDX { 3 };      //  TODO is there a function to determine index?

class RawSourceRange
{
  Q_DECLARE_TR_FUNCTIONS(RawSourceRange)

  public:
    RawSourceRange():
      decimals(0),
      min(0.0),
      max(0.0),
      step(1.0),
      offset(0.0)
    {
    }

    float getValue(int value);

    int decimals;
    double min;
    double max;
    double step;
    double offset;
    QString unit;
};

#define RANGE_DELTA_FUNCTION      1
#define RANGE_ABS_FUNCTION        2

class RawSource {
  Q_DECLARE_TR_FUNCTIONS(RawSource)

  public:
    enum SourceGroups {
      NoneGroup      = 0x001,
      SourcesGroup   = 0x002,
      TrimsGroup     = 0x004,
      SwitchesGroup  = 0x008,
      GVarsGroup     = 0x010,
      TelemGroup     = 0x020,
      InputsGroup    = 0x040,
      ScriptsGroup   = 0x080,
      NegativeGroup  = 0x100,
      PositiveGroup  = 0x200,

      InputSourceGroups = NoneGroup | SourcesGroup | TrimsGroup | SwitchesGroup | InputsGroup,
      AllSourceGroups   = InputSourceGroups | GVarsGroup | TelemGroup | ScriptsGroup
    };

    RawSource() { clear(); }

    explicit RawSource(int value):
      type(RawSourceType(abs(value) / 65536)),
      index(value >= 0 ? abs(value) % 65536 : -(abs(value) % 65536))
    {
    }

    RawSource(RawSourceType type, int index = 0):
      type(type),
      index(type != SOURCE_TYPE_NONE && index == 0 ? 1 : index)
    {
    }

    inline const int toValue() const
    {
      return index >= 0 ? (type * 65536 + index) : -(type * 65536 - index);
    }

    RawSource convert(RadioDataConversionState & cstate);
    QString toString(const ModelData * model = nullptr, const GeneralSettings * const generalSettings = nullptr, Board::Type board = Board::BOARD_UNKNOWN, bool prefixCustomName = true) const;
    RawSourceRange getRange(const ModelData * model, const GeneralSettings & settings, unsigned int flags=0) const;
    bool isStick(Board::Type board = Board::BOARD_UNKNOWN) const;
    bool isTimeBased(Board::Type board = Board::BOARD_UNKNOWN) const;
    bool isAvailable(const ModelData * const model = nullptr, const GeneralSettings * const gs = nullptr, Board::Type board = Board::BOARD_UNKNOWN) const;
    bool isSet() const { return type != SOURCE_TYPE_NONE || index != 0; }
    void clear() { type = SOURCE_TYPE_NONE; index = 0; }
    static StringTagMappingTable getSpecialTypesLookupTable();
    static StringTagMappingTable getCyclicLookupTable();

    bool operator == ( const RawSource & other) const {
      return (this->type == other.type) && (this->index == other.index);
    }

    bool operator != ( const RawSource & other) const {
      return (this->type != other.type) || (this->index != other.index);
    }

    RawSourceType type;
    int index;

  private:
};
