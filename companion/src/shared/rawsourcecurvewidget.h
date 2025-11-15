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

constexpr int UI_FLAG_CURVE_IMAGE { 1 << 4 };

class CurveImageWidget;

class RawSourceCurveWidget : public RawSourceWidget {

  Q_OBJECT

  public:
    explicit RawSourceCurveWidget(QWidget * parent,
                                ModelData * modelData = nullptr,
                                CompoundItemModelFactory * sharedItemModels = nullptr,
                                RawSource * src = nullptr,
                                RawSource dflt = RawSource(),
                                int imFilter = RawSource::CurvesGroup | RawSource::NoneGroup,
                                int uiFlags = UI_FLAG_SOURCE | UI_FLAG_CURVE_IMAGE);

    virtual ~RawSourceCurveWidget();

    // if the widget is included in a .ui definition file
    // this function must be called after setupUi(this)
    // as only the parent widget is passed to the constructor
    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              RawSource * src,
              RawSource dflt = RawSource(),
              int imFilter = RawSource::CurvesGroup | RawSource::NoneGroup,
              int uiFlags = UI_FLAG_SOURCE | UI_FLAG_CURVE_IMAGE);

    void setVisible(bool state);

  signals:
    void dataChanged();

  protected:
    int uiFlags;

  protected slots:
    void update(bool notify = true);


  private:
    CurveImageWidget *curveImage;
};
