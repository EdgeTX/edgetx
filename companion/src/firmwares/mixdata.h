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

class RadioDataConversionState;

enum MltpxValue {
  MLTPX_ADD,
  MLTPX_MUL,
  MLTPX_REP
};

#define MIXDATA_NAME_LEN  10

class MixData {
  Q_DECLARE_TR_FUNCTIONS(MixData)

  public:
    MixData() { clear(); }

    unsigned int destCh;            //        1..CPN_MAX_CHNOUT
    RawSource srcRaw;
    int     weight;
    RawSwitch swtch;
    CurveReference curve;
    unsigned int delayPrec;
    unsigned int delayUp;
    unsigned int delayDown;
    unsigned int speedPrec;
    unsigned int speedUp;
    unsigned int speedDown;
    int  carryTrim;
    bool noExpo;
    MltpxValue mltpx;
    unsigned int mixWarn;
    unsigned int flightModes;       // -5=!FP4, 0=normal, 5=FP4
    int    sOffset;
    char   name[MIXDATA_NAME_LEN + 1];

    void convert(RadioDataConversionState & cstate);
    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(MixData)); }
    bool isEmpty() const;
};
