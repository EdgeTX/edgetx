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

CurveReferenceWidget::CurveReferenceWidget(QWidget * parent,
                                           ModelData * modelData,
                                           CompoundItemModelFactory * sharedItemModels,
                                           CurveReference * curveRef,
                                           int imFilter,
                                           int uiFlags,
                                           CurveReference dflt,
                                           QString useLabel,
                                           int min,
                                           int max,
                                           int precision,
                                           int decimals,
                                           double step,
                                           QString prefix,
                                           QString suffix) :
  RawSourceWidget(parent, modelData, sharedItemModels, &curveRef->source,
                     imFilter, uiFlags, dflt.source),
  curveRef(curveRef),
  cboType(nullptr)
{
  init(modelData, sharedItemModels, curveRef, imFilter, uiFlags, dflt,
       useLabel, min, max, precision, decimals, step, prefix, suffix);
}

CurveReferenceWidget::~CurveReferenceWidget()
{
}

void CurveReferenceWidget::init(ModelData * modelData,
                                CompoundItemModelFactory * sharedItemModels,
                                CurveReference * curveRef,
                                int imFilter,
                                int uiFlags,
                                CurveReference dflt,
                                QString useLabel,
                                int min,
                                int max,
                                int precision,
                                int decimals,
                                double step,
                                QString prefix,
                                QString suffix)
{
  RawSourceWidget::init(modelData, sharedItemModels, &curveRef->source,
                        imFilter, uiFlags, dflt.source);

  if (uiFlags & UI_FLAG_CURVE_TYPE) {
    cboType = new QComboBox(this);
    cboType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboType->setModel(CurveReference::typeItemModel());
    cboType->setCurrentIndex(cboType->findData((int)curveRef->type));
    if (cboType->currentIndex() < 0)
      cboType->setCurrentIndex(Helpers::getFirstPosValueIndex(cboType));
    connect(cboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveReferenceWidget::cboTypeChanged);
  }

  update();
}

void CurveReferenceWidget::update()
{
  lock = true;

  if (cboType)
    cboType->setCurrentIndex(cboType->findData(curveRef->type));

  if (curveRef->type == CurveReference::CURVE_REF_DIFF || curveRef->type == CurveReference::CURVE_REF_EXPO) {
    RawSourceWidget::setVisible(true);
    RawSourceWidget::update();
  } else {
    RawSourceWidget::setVisible(false);
  }

  emit resize();

  lock = false;
}

void CurveReferenceWidget::cboTypeChanged(int index)
{
  if (!lock) {
    CurveReference::CurveRefType type = (CurveReference::CurveRefType)cboType->itemData(index).toInt();
    *curveRef = CurveReference::getDefaultValue(type);
    emit dataChanged();
    update();
  }
}
