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

enum RawSwitchType {
  SWITCH_TYPE_NONE,
  SWITCH_TYPE_SWITCH,
  SWITCH_TYPE_FUNCTIONSWITCH, // depreciated kept to preserve following enums
  SWITCH_TYPE_VIRTUAL,
  SWITCH_TYPE_MULTIPOS_POT,
  SWITCH_TYPE_TRIM,
  SWITCH_TYPE_ROTARY_ENCODER,
  SWITCH_TYPE_ON,
  SWITCH_TYPE_OFF,
  SWITCH_TYPE_ONE,
  SWITCH_TYPE_FLIGHT_MODE,
  SWITCH_TYPE_TIMER_MODE, // depreciated kept to preserve following enums
  SWITCH_TYPE_TELEMETRY,
  SWITCH_TYPE_SENSOR,
  SWITCH_TYPE_ACT,
  SWITCH_TYPE_TRAINER,
  MAX_SWITCH_TYPE
};

class RawSwitch {
  Q_DECLARE_TR_FUNCTIONS(RawSwitch)

  public:
    enum SwitchContext
    {
      LogicalSwitchesContext  = 0x01,
      SpecialFunctionsContext = 0x02,
      GlobalFunctionsContext  = 0x04,
      MixesContext            = 0x08,

      AllModelContexts        = SpecialFunctionsContext | LogicalSwitchesContext | MixesContext,
      AllSwitchContexts       = AllModelContexts | GlobalFunctionsContext
    };

    RawSwitch() { clear(); }

    explicit RawSwitch(int value):
      type(RawSwitchType(abs(value)/256)),
      index(value >= 0 ? abs(value)%256 : -(abs(value)%256))
    {
    }

    RawSwitch(RawSwitchType type, int index=0):
      type(type),
      index(index)
    {
    }

    inline const int toValue() const
    {
      return index >= 0 ? (type * 256 + index) : -(type * 256 - index);
    }

    RawSwitch convert(RadioDataConversionState & cstate);
    QString toString(Board::Type board = Board::BOARD_UNKNOWN, const GeneralSettings * const generalSettings = nullptr, const ModelData * const modelData = nullptr, bool prefixCustomName = true) const;
    bool isAvailable(const ModelData * const model = nullptr, const GeneralSettings * const gs = nullptr, Board::Type board = Board::BOARD_UNKNOWN) const;
    bool isSet() const { return type != SWITCH_TYPE_NONE || index != 0; }
    void clear() { type = SWITCH_TYPE_NONE; index = 0; }
    static StringTagMappingTable getRawSwitchTypesLookupTable();

    bool operator== ( const RawSwitch& other) const {
      return (this->type == other.type) && (this->index == other.index);
    }

    bool operator!= ( const RawSwitch& other) const {
      return (this->type != other.type) || (this->index != other.index);
    }

    RawSwitchType type;
    int index;
};
