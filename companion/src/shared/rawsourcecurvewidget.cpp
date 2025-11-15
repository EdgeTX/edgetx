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

#include "rawsourcecurvewidget.h"
#include "curveimagewidget.h"
#include "helpers.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

#include <float.h>

RawSourceCurveWidget::RawSourceCurveWidget(QWidget * parent,
                        ModelData * modelData,
                        CompoundItemModelFactory * sharedItemModels,
                        RawSource * src,
                        RawSource dflt,
                        int imFilter,
                        int uiFlags) :
  RawSourceWidget(parent, modelData, sharedItemModels, imFilter, src, dflt),
  uiFlags(uiFlags),
  curveImage(nullptr)
{
  init(modelData, sharedItemModels, src, dflt, imFilter, uiFlags);
}

RawSourceCurveWidget::~RawSourceCurveWidget()
{
}

void RawSourceCurveWidget::init(ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              RawSource * src,
                              RawSource dflt,
                              int imFilter,
                              int uiFlags)
{
  RawSourceWidget::init(modelData, sharedItemModels, imFilter, src, dflt);
  this->uiFlags = uiFlags;

  if (uiFlags & UI_FLAG_CURVE_IMAGE) {
    curveImage = new CurveImageWidget(this);
    curveImage->set(modelData, getCurrentFirmware(), sharedItemModels, src->index, Qt::black, 3);
    curveImage->setGrid(Qt::gray, 2);
    connect(curveImage, &CurveImageWidget::doubleClicked, [&]() { curveImage->edit(); });
  }
}

void RawSourceWidget::update(bool notify)
{
  lock = true;

  if (curveRef->type == CurveReference::CURVE_REF_DIFF || curveRef->type == CurveReference::CURVE_REF_EXPO) {
    RawSourceWidget::setVisible(true);
    RawSourceWidget::update();
  } else {
    RawSourceWidget::setVisible(false);
  }

  if (curveImage) {
    if (curveRef->type == CurveReference::CURVE_REF_CUSTOM &&
        curveRef->source.type == RawSourceType::SOURCE_TYPE_CURVE) {
      curveImage->setIndex(curveRef->source.index);

      if (abs(curveRef->source.index) > 0 && abs(curveRef->source.index) <= CPN_MAX_CURVES)
        curveImage->setPen(colors[abs(curveRef->source.index) - 1], 3);
      else
        curveImage->setPen(Qt::black, 3);

      curveImage->draw();
      curveImage->setVisible(true);
    } else {
      curveImage->setVisible(false);
    }
  }

  if (notify)
    emit dataChanged();

  emit resize();

  lock = false;
}

void RawSourceCurveWidget::setVisible(bool state)
{
  if (curveImage)
    curveImage->setVisible(state);
}
