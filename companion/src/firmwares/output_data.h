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

#include "rawsource.h"
#include "curvereference.h"

#include <QtCore>

#define LIMITDATA_NAME_LEN  6

class RadioDataConversionState;
class AbstractStaticItemModel;

class LimitData {
  Q_DECLARE_TR_FUNCTIONS(LimitData)

  public:
    LimitData() { clear(); }

    RawSource min;
    RawSource max;
    bool revert;
    RawSource offset;
    int ppmCenter;
    bool symetrical;
    int failsafe;
    char name[LIMITDATA_NAME_LEN + 1];
    RawSource curve;

    void clear();
    bool isEmpty() const;
    QString revertToString() const;
    QString nameToString(int index) const;
    QString symetricalToString() const;
    static QString symetricalToString(bool value);
    void convert(RadioDataConversionState & cstate);
    static AbstractStaticItemModel *symetricalModel();
};
