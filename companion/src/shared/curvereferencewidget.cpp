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
                                           int listFilter,
                                           int flags,
                                           CurveReference dflt,
                                           QString typeLabel,
                                           int min,
                                           int max,
                                           int precision,
                                           double step,
                                           QString prefix,
                                           QString suffix,
                                           bool isAvailable) :
  QWidget(parent)
{
  init();
  init(modelData, sharedItemModels, curveRef, listFilter, flags, dflt,
       typeLabel, min, max, precision, step, prefix, suffix, isAvailable);
}

CurveReferenceWidget::CurveReferenceWidget(QWidget * parent) :
  QWidget(parent)
{
  init();
}

CurveReferenceWidget::~CurveReferenceWidget()
{
  if (rawSourceWidget)
    delete rawSourceWidget;
}

void CurveReferenceWidget::init()
{
  cboType = nullptr;
  rawSourceWidget = nullptr;
  lock = false;
}

void CurveReferenceWidget::init(ModelData * modelData,
                                CompoundItemModelFactory * sharedItemModels,
                                CurveReference * curveRef,
                                int listFilter,
                                int flags,
                                CurveReference dflt,
                                QString typeLabel,
                                int min,
                                int max,
                                int precision,
                                double step,
                                QString prefix,
                                QString suffix,
                                bool isAvailable)
{
  this->curveRef = curveRef;

  // minimise padding around widget
  setContentsMargins(0, 0, 0, 0);
  // layout to group the requested ui objects
  QHBoxLayout *layout = new QHBoxLayout();
  // minimise padding around sub-widgets
  layout->setContentsMargins(0, 0, 0, 0);

  cboType = new QComboBox(this);
  cboType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  cboType->setModel(CurveReference::typeItemModel());
  cboType->setCurrentIndex(cboType->findData((int)curveRef->type));

  if (cboType->currentIndex() < 0)
    cboType->setCurrentIndex(Helpers::getFirstPosValueIndex(cboType));

  connect(cboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &CurveReferenceWidget::cboTypeChanged);
  layout->addWidget(cboType);

  rawSourceWidget = new RawSourceWidget(this, modelData, sharedItemModels,
                                        &curveRef->source, listFilter, flags,
                                        dflt.source, typeLabel, min, max, precision, step,
                                        prefix, suffix, isAvailable);

  connect(rawSourceWidget, &RawSourceWidget::dataChanged, [&]() { emit dataChanged(); });
  connect(rawSourceWidget, &RawSourceWidget::resized, [&]() { shrink(); });
  layout->addWidget(rawSourceWidget);

  setLayout(layout);
  update();
}

void CurveReferenceWidget::update()
{
  lock = true;

  if (cboType)
    cboType->setCurrentIndex(cboType->findData(curveRef->type));

  if (curveRef->type == CurveReference::CURVE_REF_DIFF || curveRef->type == CurveReference::CURVE_REF_EXPO) {
    rawSourceWidget->setVisible(true);
    rawSourceWidget->update();
  } else {
    rawSourceWidget->setVisible(false);
  }

  shrink();

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

void CurveReferenceWidget::shrink()
{
  //adjustSize();
  //resize(0, 0);
  //emit resized();
}
