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

#ifndef GVARDATA_H
#define GVARDATA_H

#include <QtCore>

#define GVAR_NAME_LEN       3
#define GVAR_MAX_VALUE      1024
#define GVAR_MIN_VALUE      -GVAR_MAX_VALUE

class GVarData {
  Q_DECLARE_TR_FUNCTIONS(GVarData)

  public:
    GVarData() { clear(); }

    enum {
      GVAR_UNIT_NUMBER,
      GVAR_UNIT_PERCENT
    };

    enum {
      GVAR_PREC_MUL10,
      GVAR_PREC_MUL1
    };

    char name[GVAR_NAME_LEN + 1];
    int min;
    int max;
    bool popup;
    unsigned int prec;     // 0 0._  1 0.0
    unsigned int unit;     // 0 _    1 %

    void clear() {memset(this, 0, sizeof(GVarData)); }
    QString unitToString() const;
    QString precToString() const;
    QString nameToString(int index) const;
    int multiplierSet();
    float multiplierGet() const;
    void setMin(float val);
    void setMax(float val);
    int getMin() const;
    int getMax() const;
    float getMinPrec() const;
    float getMaxPrec() const;
    bool isEmpty() const;
};


#endif // GVARDATA_H
