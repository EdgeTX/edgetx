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

class CurveImageWidget;
class RawSourceExtWidget : public RawSourceWidget {

  Q_OBJECT

  public:
    explicit RawSourceExtWidget(QWidget * parent,
                                ModelData * modelData = nullptr,
                                CompoundItemModelFactory * sharedItemModels = nullptr,
                                int imFilter = SOURCE_TYPE_NONE,
                                int uiFlags = UI_FLAG_SOURCE | UI_FLAG_USE,
                                RawSource * src = nullptr,
                                RawSource dflt = RawSource(),
                                QString useLabel = "",
                                int min = -100,
                                int max = 100,
                                int precision = 1,
                                int decimals = 0,
                                double step = 1,
                                QString prefix = "",
                                QString suffix = "");

    virtual ~RawSourceExtWidget();

    // if the widget is included in a .ui definition file
    // this function must be called after setupUi(this)
    // as only the parent widget is passed to the constructor
    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              int imFilter,
              int uiFlags,
              RawSource * src,
              RawSource dflt = RawSource(),
              QString useLabel = "",
              int min = -100,
              int max = 100,
              int precision = 1,
              int decimals = 0,
              double step = 1,
              QString prefix = "",
              QString suffix = "");

    void setVisible(bool state);
    void update();
    void updateMinMax(int max);

  signals:
    void dataChanged();
    void resize();

  protected slots:
    void useChanged(int state);
    void valueChanged();

  protected:
    int uiFlags;
    QString useLabel;
    int min;
    int max;
    int precision;
    int decimals;
    double step;
    QString prefix;
    QString suffix;

  private:
    QCheckBox *chkUse;
    QSpinBox *sbValue;
    QDoubleSpinBox *dsbValue;
    CurveImageWidget *curveImage;
};
