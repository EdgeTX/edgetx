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

#include <QtCore>

constexpr char AIM_GVARDATA_PREC[]  {"gvardata.prec"};
constexpr char AIM_GVARDATA_UNIT[]  {"gvardata.unit"};

#define GVAR_NAME_LEN       3
#define GVAR_MAX_VALUE      1024
#define GVAR_MIN_VALUE      -GVAR_MAX_VALUE

class AbstractStaticItemModel;

class GVarData {
  Q_DECLARE_TR_FUNCTIONS(GVarData)

  public:
    GVarData() { clear(); }

    enum {
      GVAR_UNIT_NUMBER,
      GVAR_UNIT_PERCENT,
      GVAR_UNIT_COUNT
    };

    enum {
      GVAR_PREC_MUL10,
      GVAR_PREC_MUL1,
      GVAR_PREC_COUNT
    };

    char name[GVAR_NAME_LEN + 1];
    int min;
    int max;
    bool popup;
    unsigned int prec;     // 0 0._  1 0.0
    unsigned int unit;     // 0 _    1 %

    void clear() {memset(this, 0, sizeof(GVarData)); }
    QString unitToString() const;
    static QString unitToString(int val);
    QString precToString() const;
    static QString precToString(int val);
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
    static AbstractStaticItemModel * unitItemModel();
    static AbstractStaticItemModel * precItemModel();
};

QDataStream &operator<<(QDataStream &out, const GVarData &obj);
QDataStream &operator>>(QDataStream &in, GVarData &obj);
