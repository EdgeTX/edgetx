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
#include "rawsourcewidget.h"

#include <QString>
#include <QComboBox>

class CompoundItemModelFactory;
class RawSourceWidget;

class CurveReferenceWidget : public QWidget {

  Q_OBJECT

  public:
    explicit CurveReferenceWidget(QWidget * parent,
                                  ModelData * modelData = nullptr,
                                  CompoundItemModelFactory * sharedItemModels = nullptr,
                                  CurveReference * curveRef = nullptr,
                                  int imFilter = SOURCE_TYPE_CURVE,
                                  int uiFlags = UI_FLAG_LIST,
                                  CurveReference dflt = CurveReference(),
                                  QString useLabel = "",
                                  int min = -1000,
                                  int max = 1000,
                                  int precision = 10,
                                  int decimals = 1,
                                  double step = 0.1,
                                  QString prefix = "",
                                  QString suffix = "");

    virtual ~CurveReferenceWidget();

    // if the widget is included in a .ui definition file
    // this function must be called after setupUi(this)
    // as only the parent widget is passed to the constructor
    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              CurveReference * curveRef,
              int imFilter,
              int uiFlags,
              CurveReference dflt = CurveReference(),
              QString useLabel = "",
              int min = -100,
              int max = 100,
              int precision = 1,
              int decimals = 0,
              double step = 1,
              QString prefix = "",
              QString suffix = "");

    void update();

    signals:
    void dataChanged();
    void resized();

  protected slots:
    void cboTypeChanged(int);

  private:
    CurveReference *curveRef;
    QComboBox *cboType;
    RawSourceWidget *rawSourceWidget;
    bool lock;

    void shrink();
};
