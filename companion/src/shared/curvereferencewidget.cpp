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

#include "curvereferencewidget.h"
#include "helpers.h"
#include "modeldata.h"
#include "generalsettings.h"
#include "compounditemmodels.h"
#include "curveimagewidget.h"

CurveReferenceWidget::CurveReferenceWidget(QWidget * parent,
                                           ModelData * modelData,
                                           CompoundItemModelFactory * sharedItemModels,
                                           CurveReference * curveRef,
                                           RawSource dflt,
                                           int filterFlags,
                                           int minValue,
                                           int maxValue,
                                           double stepValue,
                                           QString suffixValue,
                                           QString chkUseLabel,
                                           int uiFlags) :
  RawSourceExtWidget(parent, modelData, sharedItemModels, &curveRef->source, dflt, filterFlags,
                     minValue, maxValue, stepValue, suffixValue, chkUseLabel, uiFlags),
  curveRef(curveRef),
  cboType(nullptr),
  curveImage(nullptr)
{
  init(modelData, sharedItemModels, rawSource, dflt, filterFlags,
       minValue, maxValue, stepValue, suffixValue, chkUseLabel, uiFlags);
}

CurveReferenceWidget::~CurveReferenceWidget()
{

}

void CurveReferenceWidget::init(ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              RawSource * rawSource,
                              RawSource dflt,
                              int filterFlags,
                              int minValue,
                              int maxValue,
                              double stepValue,
                              QString suffixValue,
                              QString chkUseLabel,
                              int uiFlags)
{
  RawSourceExtWidget::init(modelData, sharedItemModels, rawSource, dflt, filterFlags,
                      minValue, maxValue, stepValue, suffixValue, chkUseLabel, uiFlags);

  if (uiFlags & UI_FLAG_CURVE_TYPE) {
    cboType = new QComboBox(this);
    cboType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboType->setModel(CurveReference::typeItemModel());
    cboType->setCurrentIndex(cboType->findData((int)curveRef->type));
    connect(cboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveReferenceWidget::cboTypeChanged);
  }

  if (uiFlags & UI_FLAG_CURVE_IMAGE) {
    curveImage = new CurveImageWidget(this);
    curveImage->set(modelData, getCurrentFirmware(), sharedItemModels, curveRef->source.index, Qt::black, 3);
    curveImage->setGrid(Qt::gray, 2);
    connect(curveImage, &CurveImageWidget::doubleClicked, this, &CurveReferenceWidget::curveImageDoubleClicked);
  }

  update(false);
}

void CurveReferenceWidget::update(bool notify)
{
  lock = true;

  if (cboType)
    cboType->setCurrentIndex(cboType->findData(curveRef->type));

  if (curveRef->type == CurveReference::CURVE_REF_DIFF || curveRef->type == CurveReference::CURVE_REF_EXPO) {
    RawSourceExtWidget::setVisible(true);
    RawSourceExtWidget::update();
  } else {
    RawSourceExtWidget::setVisible(false);
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

void CurveReferenceWidget::cboTypeChanged(int index)
{
  if (!lock) {
    CurveReference::CurveRefType type = (CurveReference::CurveRefType)cboType->itemData(index).toInt();
    *curveRef = CurveReference::getDefaultValue(type);
    update();
  }
}

void CurveReferenceWidget::curveImageDoubleClicked()
{
  if (curveRef->type == CurveReference::CURVE_REF_CUSTOM && abs(curveRef->source.index) > 0)
    curveImage->edit();
}
