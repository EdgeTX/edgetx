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

#include "autosource.h"

#include <QDoubleSpinBox>

class AutoSourceDSB : public QDoubleSpinBox, virtual public AutoSource {

  Q_OBJECT

  public:
    explicit AutoSourceDSB(QWidget * parent = nullptr);
    virtual ~AutoSourceDSB();

    virtual void updateValue() override;

    void setField(RawSource * field, RawSource dflt = RawSource(),
                  QString typeLabel = "",
                  int min = -1024, int max = 1024,
                  int precision = 0,
                  QString prefix = "", QString suffix = "", GenericPanel * panel = nullptr);

    void setField(RawSource dflt = RawSource(),
                  QString typeLabel = "",
                  int min = -1024, int max = 1024,
                  int precision = 0,
                  QString prefix = "", QString suffix = "");

    void setMinMax(const int min, const int max);
    void setValueDefault();

  signals:
    void dataChanged();

  protected slots:
    void on_editingFinished();

  private:
    RawSource m_dflt;
    QString m_typeLabel;
    int m_min;
    int m_max;
    int m_precision;
    QString m_prefix;
    QString m_suffix;
    QDoubleSpinBox *m_dsbValue;
    double m_scalingFactor;
};
