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

#include "rawsourceextwidget.h"
#include "helpers.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

#include <float.h>

RawSourceExtWidget::RawSourceExtWidget(QWidget * parent,
                                       ModelData * modelData,
                                       CompoundItemModelFactory * sharedItemModels,
                                       RawSource * rawSource,
                                       RawSource defValue,
                                       int filterFlags,
                                       int uiFlags,
                                       QString chkUseLabel,
                                       int minValue,
                                       int maxValue,
                                       double stepValue,
                                       int decimals,
                                       QString suffixValue) :
  RawSourceWidget(parent, modelData, sharedItemModels, rawSource, dflt, filterFlags),
  minValue(minValue),
  maxValue(maxValue),
  stepValue(stepValue),
  decimals(decimals),
  suffixValue(suffixValue),
  chkUseLabel(chkUseLabel),
  uiFlags(uiFlags),
  chkUse(nullptr),
  sbValue(nullptr),
  dsbValue(nullptr)
{
  init(modelData, sharedItemModels, rawSource, dflt, filterFlags,
       uiFlags, chkUseLabel, minValue, maxValue, stepValue, decimals, suffixValue);
}

RawSourceExtWidget::~RawSourceExtWidget()
{

}

void RawSourceExtWidget::init(ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              RawSource * rawSource,
                              RawSource defValue,
                              int filterFlags,
                              int uiFlags,
                              QString chkUseLabel,
                              int minValue,
                              int maxValue,
                              double stepValue,
                              int decimals,
                              QString suffixValue)
{
  RawSourceWidget::init(modelData, sharedItemModels, rawSource, dflt, filterFlags);
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->stepValue = stepValue;
  this->decimals = decimals;
  this->suffixValue = suffixValue;
  this->chkUseLabel = chkUseLabel;
  this->uiFlags = uiFlags;

  if (uiFlags & UI_FLAG_USE) {
    chkUse = new QCheckBox(chkUseLabel);
    connect(chkUse, &QCheckBox::checkStateChanged, this,
            &RawSourceExtWidget::useChanged);
  }

  if (uiFlags & UI_FLAG_SPINBOX) {
    sbValue = new QSpinBox(this);
    sbValue->setMinimum(minValue);
    sbValue->setMaximum(maxValue);
    sbValue->setSingleStep((int)stepValue);
    sbValue->setSuffix(suffixValue);
    sbValue->setValue(dflt.index);
    connect(sbValue, &QSpinBox::editingFinished, this,
            &RawSourceExtWidget::valueChanged);
  }

  if (uiFlags & UI_FLAG_DBLSPINBOX) {
    dsbValue = new QDoubleSpinBox(this);
    dsbValue->setDecimals(decimals);
    dsbValue->setMinimum(minValue);
    dsbValue->setMaximum(maxValue);
    dsbValue->setSingleStep(stepValue);
    dsbValue->setSuffix(suffixValue);
    dsbValue->setValue(dflt.index); // conversion / step ????
    connect(dsbValue, &QDoubleSpinBox::editingFinished, this,
            &RawSourceExtWidget::valueChanged);
  }

  update(false);
}

void RawSourceExtWidget::useChanged(int state)
{
  if (!lock) {
    if (state == Qt::Checked) {
      *rawSource = dflt;
    }
    else {
      *rawSource = RawSource(SOURCE_TYPE_NUMBER);
      if (sbValue)
        sbValue->setValue(rawSource->index);
      if (dsbValue)
        dsbValue->setValue(rawSource->index);  //   this needs fixing probably / step
    }

    update();
  }
}

void RawSourceExtWidget::valueChanged()
{
  if (!lock) {
    rawSource->type = SOURCE_TYPE_NUMBER;
    if (sbValue)
      rawSource->index = sbValue->value();
    else if(dsbValue)
      rawSource->index = round(dsbValue->value() / stepValue); // ???????? might need to be raw
    update();
  }
}

void RawSourceExtWidget::setVisible(bool state)
{
  if (chkUse)
    chkUse->setVisible(state);
  if (sbValue)
    sbValue->setVisible(state);
  if (dsbValue)
    dsbValue->setVisible(state);
}

void RawSourceExtWidget::update(bool notify)
{
  lock = true;

  if (rawSource->type == SOURCE_TYPE_NUMBER) {
    RawSourceWidget::setVisible(false);

    if (sbValue)
      sbValue->setValue(rawSource->index);

    if (dsbValue)
      dsbValue->setValue(rawSource->index); // fix this
  }
  else {
    RawSourceWidget::setVisible(true);
    RawSourceWidget::update();

    if (sbValue)
      sbValue->setVisible(false);

    if (dsbValue)
      dsbValue->setVisible(false);
  }

  emit resize();

  if (notify)
    emit dataChanged();

  lock = false;
}
