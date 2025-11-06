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
                                  ModelData * modelData,
                                  CompoundItemModelFactory * sharedItemModels,
                                  CurveReference * curveRef,
                                  int listFilter,
                                  int flags,
                                  CurveReference dflt = CurveReference(),
                                  QString typeLabel = "",
                                  int min = -1000,
                                  int max = 1000,
                                  int precision = 1,
                                  double step = -1.0,
                                  QString prefix = "",
                                  QString suffix = "",
                                  bool isAvailable = true);

    virtual ~CurveReferenceWidget();

    // if the widget is included in a .ui definition file
    // only the parent widget is passed to the constructor
    explicit CurveReferenceWidget(QWidget * parent);

    void init();

    // if the widget is included in a .ui definition file
    // this function must be called after setupUi(this)
    // as only the parent widget is passed to the constructor
    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              CurveReference * curveRef,
              int listFilter,
              int flags,
              CurveReference dflt = CurveReference(),
              QString typeLabel = "",
              int min = -1000,
              int max = 1000,
              int precision = 1,
              double step = -1.0,
              QString prefix = "",
              QString suffix = "",
              bool isAvailable = true);

    void update();

    signals:
    void dataChanged();
    void resized();

  protected slots:
    void cboTypeChanged(int);
    void rswDataChanged(RawSource src);

  private:
    CurveReference *curveRef;
    int listFilter;
    QComboBox *cboType;
    RawSourceWidget *rawSourceWidget;
    bool lock;

    void setRawSourceWidget();
    void shrink();
};
