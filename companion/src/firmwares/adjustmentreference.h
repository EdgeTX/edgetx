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

#ifndef ADJUSTMENTREFERENCE_H
#define ADJUSTMENTREFERENCE_H

#include <QtCore>

class ModelData;

constexpr int ADJUST_REF_GVAR_BASE { 10000 };

class AdjustmentReference {
  Q_DECLARE_TR_FUNCTIONS(AdjustmentReference)

  public:
    enum AdjustRefType {
      ADJUST_REF_VALUE,
      ADJUST_REF_GVAR,
    };

    AdjustmentReference() { clear(); }
    explicit AdjustmentReference(int value);
    AdjustmentReference(const AdjustRefType type, const int value);

    void clear() { memset(this, 0, sizeof(AdjustmentReference)); }

    bool isSet() const { return type != ADJUST_REF_VALUE || value != 0; }
    bool isValid(const int value) const;
    QString toString(const ModelData * model = nullptr, const bool sign = false) const;
    const bool isAvailable() const;

    inline const int toValue() const
    {
      if (type == ADJUST_REF_GVAR)
        return value >= 0 ? value + ADJUST_REF_GVAR_BASE : value - ADJUST_REF_GVAR_BASE;
      else
        return value;
    }

    bool operator== ( const AdjustmentReference & other) const {
      return (this->type == other.type) && (this->value == other.value);
    }

    bool operator!= ( const AdjustmentReference & other) const {
      return (this->type != other.type) || (this->value != other.value);
    }

    AdjustRefType type;
    int value;
};

#endif // ADJUSTMENTREFERENCE_H
