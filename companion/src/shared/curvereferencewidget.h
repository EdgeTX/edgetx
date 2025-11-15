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

#include "curvereference.h"
#include "rawsourceextwidget.h"

class CurveImageWidget;

constexpr int UI_FLAG_CURVE_TYPE  { 1 << 4 };
constexpr int UI_FLAG_CURVE_IMAGE { 1 << 5 };

class CurveReferenceWidget : public RawSourceExtWidget {

  Q_OBJECT

  public:
    explicit CurveReferenceWidget(QWidget * parent,
                                  ModelData * modelData = nullptr,
                                  CompoundItemModelFactory * sharedItemModels = nullptr,
                                  CurveReference * curveRef = nullptr,
                                  RawSource defValue = RawSource(),
                                  int filterFlags = 0,
                                  int uiFlags = UI_FLAG_NONE,
                                  QString chkUseLabel = QString(),
                                  int minValue = 0,
                                  int maxValue = 100,
                                  double stepValue = 1,
                                  int decimals = 0,
                                  QString suffixValue = QString());

    virtual ~CurveReferenceWidget();

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

  signals:
    void dataChanged();
    void resize();

  protected slots:
    void cboTypeChanged(int);
    void curveImageDoubleClicked();
    void update(bool notify = true);

  private:
    CurveReference *curveRef;
    QComboBox *cboType;
    CurveImageWidget *curveImage;
};
