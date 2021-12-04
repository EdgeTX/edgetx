/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include <QtCore>
#include <string>
#include <vector>

typedef std::pair<int, std::string> Int2StringMapping;
typedef std::vector<Int2StringMapping> Int2StringMappingTable;

class FieldRange
{
  public:
    FieldRange():
      decimals(0),
      min(0.0),
      max(0.0),
      step(1.0),
      offset(0.0),
      prefix(""),
      unit("")
    {
    }

    float getValue(int value) { return float(value) * step; }

    int decimals;
    double min;
    double max;
    double step;
    double offset;
    QString prefix;
    QString unit;
};


constexpr unsigned int TIMESTR_MASK_HRSMINS   { 1 << 1 };
constexpr unsigned int TIMESTR_MASK_ZEROHRS   { 1 << 2 };
constexpr unsigned int TIMESTR_MASK_PADSIGN   { 1 << 3 };

namespace DataHelpers
{
  enum BoolFormat {
    BOOL_FMT_ENABLEDISABLE,
    BOOL_FMT_ONOFF,
    BOOL_FMT_TRUEFALSE,
    BOOL_FMT_YN,
    BOOL_FMT_YESNO
  };

  QString boolToString(const bool value, const BoolFormat format);
  QString getElementName(const QString & prefix, const unsigned int index, const char * name = 0, const bool padding = false);
  QString timeToString(const int value, const unsigned int mask);

}
