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

#include <QtCore>

class GeneralSettings;
class ModelData;
class RadioDataConversionState;
class AbstractItemModel;

class CurveReference {

  Q_DECLARE_TR_FUNCTIONS(CurveReference)

  public:

    enum CurveRefType {
      CURVE_REF_DIFF,
      CURVE_REF_EXPO,
      CURVE_REF_FUNC,
      CURVE_REF_CUSTOM,
      MAX_CURVE_REF_COUNT
    };

    CurveReference();

    CurveReference(CurveRefType type, RawSource source) :
      type(type),
      source(source)
    {}

    void clear();
    const bool isEmpty() const { return type == CURVE_REF_DIFF && source.index == 0; }
    const bool isSet() const { return !isEmpty(); }
    const QString toString(const ModelData * model = nullptr, bool verbose = true, const GeneralSettings * const generalSettings = nullptr,
                           Board::Type board = Board::BOARD_UNKNOWN, bool prefixCustomName = true) const;
    CurveReference convert(RadioDataConversionState & cstate);

    CurveRefType type;
    RawSource source;

    bool operator == ( const CurveReference & other) const {
      return (this->type == other.type) && (this->source == other.source);
    }

    bool operator != ( const CurveReference & other) const {
      return (this->type != other.type) || (this->source != other.source);
    }

    static CurveReference getDefaultValue(const CurveRefType type);
    static QString typeToString(const int type);
    static AbstractItemModel *typeItemModel();
};
