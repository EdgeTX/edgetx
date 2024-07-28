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

#include <QtCore>

class AbstractStaticItemModel;

class CurvePoint {
  public:
    int8_t x;
    int8_t y;
};

#define CURVEDATA_NAME_LEN  3

class CurveData {
  Q_DECLARE_TR_FUNCTIONS(CurveData)

  public:
    enum CurveType {
      CURVE_TYPE_STANDARD,
      CURVE_TYPE_CUSTOM,
      CURVE_TYPE_LAST = CURVE_TYPE_CUSTOM
    };

    CurveData();

    CurveType type;
    bool smooth;
    int  count;
    CurvePoint points[CPN_MAX_POINTS];
    char name[CURVEDATA_NAME_LEN + 1];

    void clear(int count = 5);
    void init();
    bool isEmpty() const;
    QString nameToString(const int idx) const;
    QString typeToString() const;
    QString pointsToString() const;
    static QString typeToString(int value);
    static AbstractStaticItemModel * typeItemModel();
    static AbstractStaticItemModel * pointsItemModel();
};
