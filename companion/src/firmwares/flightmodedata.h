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

#include "constants.h"
#include "rawswitch.h"

#include <QtCore>
#include <QComboBox>

class RadioDataConversionState;
class AbstractStaticItemModel;
class Firmware;

#define FLIGHTMODE_NAME_LEN  10

constexpr char AIM_FLIGHTMODEDATA_GVARFM[]  {"flightmodedata.gvarfm"};

class FlightModeData {
  Q_DECLARE_TR_FUNCTIONS(FlightModeData)

  public:
    FlightModeData() { clear(0); }

    int trimMode[CPN_MAX_TRIMS];
    int trimRef[CPN_MAX_TRIMS];
    int trim[CPN_MAX_TRIMS];
    RawSwitch swtch;
    char name[FLIGHTMODE_NAME_LEN + 1];
    unsigned int fadeIn;
    unsigned int fadeOut;
    int gvars[CPN_MAX_GVARS];

    void convert(RadioDataConversionState & cstate);
    void clear(const int phaseIdx);
    bool isEmpty(int phaseIdx) const;
    QString nameToString(int phaseIdx) const;
    bool isGVarEmpty(int phaseIdx, int gvIdx) const;
    int linkedFlightModeZero(int phaseIdx, int maxOwnValue) const;
    int linkedGVarFlightModeZero(int phaseIdx) const;
    static AbstractStaticItemModel * gvarModesItemModel(int excludeMode);
    static QString modeAbbrev();
    static QString modeName();
};
