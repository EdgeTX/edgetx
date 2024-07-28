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

#include "curvereference.h"
#include "rawsource.h"
#include "rawswitch.h"

#include <QtCore>

// identiying names of static abstract item models
constexpr char AIM_EXPO_CARRYTRIM[]        {"expo.carrytrim"};

class RadioDataConversionState;
class AbstractStaticItemModel;

enum InputMode {
  INPUT_MODE_NONE,
  INPUT_MODE_POS,
  INPUT_MODE_NEG,
  INPUT_MODE_BOTH
};

constexpr int CARRYTRIM_DEFAULT = {0};
constexpr int CARRYTRIM_STICK_OFF = {1};

enum CarryTrimGroups {
  CarryTrimSticksGroup      = 0x001,
  CarryTrimNotSticksGroup   = 0x002
};

#define EXPODATA_NAME_LEN  10

class ExpoData {
  Q_DECLARE_TR_FUNCTIONS(ExpoData)

  public:
    ExpoData() { clear(); }

    RawSource srcRaw;
    unsigned int scale;
    unsigned int mode;   // InputMode
    unsigned int chn;
    RawSwitch swtch;
    unsigned int flightModes;        // -5=!FP4, 0=normal, 5=FP4
    int  weight;
    int offset;
    CurveReference curve;
    int carryTrim;
    char name[EXPODATA_NAME_LEN + 1];

    void convert(RadioDataConversionState & cstate);
    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(ExpoData)); }
    bool isEmpty() const;
    QString carryTrimToString() const;

    static AbstractStaticItemModel * carryTrimItemModel();
};
