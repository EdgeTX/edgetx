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

#include "rawsourcewidget.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

constexpr int UI_FLAG_USE        { 1 << 2 };
constexpr int UI_FLAG_SPINBOX    { 1 << 3 };
constexpr int UI_FLAG_DBLSPINBOX { 1 << 4 };
class RawSourceExtWidget : public RawSourceWidget {

  Q_OBJECT

  public:
    explicit RawSourceExtWidget(QWidget * parent,
                                ModelData * modelData = nullptr,
                                CompoundItemModelFactory * sharedItemModels = nullptr,
                                RawSource * rawSource = nullptr,
                                RawSource defValue = RawSource(),
                                int filterFlags = 0,
                                int uiFlags = UI_FLAG_NONE,
                                QString chkUseLabel = QString(),
                                int minValue = 0,
                                int maxValue = 100,
                                double stepValue = 1,
                                int decimals = 0,
                                QString suffixValue = QString());

    virtual ~RawSourceExtWidget();

    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              RawSource * rawSource,
              RawSource defValue = RawSource(),
              int filterFlags = 0,
              int uiFlags = UI_FLAG_NONE,
              QString chkUseLabel = QString(),
              int minValue = 0,
              int maxValue = 100,
              double stepValue = 1,
              int decimals = 0,
              QString suffixValue = QString());

    void setVisible(bool state);
    void update(bool notify = true);

  signals:
    void dataChanged();
    void resize();

  protected slots:
    void useChanged(int state);
    void valueChanged();

  protected:
    int minValue;
    int maxValue;
    double stepValue;
    int decimals;
    QString suffixValue;
    QString chkUseLabel;
    int uiFlags;

  private:
    QCheckBox *chkUse;
    QSpinBox *sbValue;
    QDoubleSpinBox *dsbValue;
};
