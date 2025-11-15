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
                                       int imFilter,
                                       int uiFlags,
                                       RawSource * src,
                                       RawSource dflt,
                                       QString useLabel,
                                       int min,
                                       int max,
                                       int precision,
                                       int decimals,
                                       double step,
                                       QString prefix,
                                       QString suffix) :
  RawSourceWidget(parent, modelData, sharedItemModels, imFilter, src, dflt),
  uiFlags(uiFlags),
  useLabel(useLabel),
  min(min),
  max(max),
  precision(precision),
  decimals(decimals),
  step(step),
  prefix(prefix),
  suffix(suffix),
  chkUse(nullptr),
  sbValue(nullptr),
  dsbValue(nullptr)
{
  init(modelData, sharedItemModels, imFilter, uiFlags, src, dflt,
       useLabel, min, max, precision, decimals, step, prefix, suffix);
}

RawSourceExtWidget::~RawSourceExtWidget()
{
}

void RawSourceExtWidget::init(ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              int imFilter,
                              int uiFlags,
                              RawSource * src,
                              RawSource dflt,
                              QString useLabel,
                              int min,
                              int max,
                              int precision,
                              int decimals,
                              double step,
                              QString prefix,
                              QString suffix)
{
  RawSourceWidget::init(modelData, sharedItemModels, imFilter, src, dflt);
  this->uiFlags = uiFlags;
  this->useLabel = useLabel;
  this->min = min;
  this->max = max;
  this->step = step;
  this->precision = precision;
  this->decimals = decimals;
  this->prefix = prefix;
  this->suffix = suffix;

  if (uiFlags & UI_FLAG_USE) {
    chkUse = new QCheckBox(useLabel);
    connect(chkUse, &QCheckBox::checkStateChanged, this,
            &RawSourceExtWidget::useChanged);
  }

  if (uiFlags & UI_FLAG_SPINBOX) {
    sbValue = new QSpinBox(this);
    sbValue->setMinimum(min);
    sbValue->setMaximum(max);
    sbValue->setSingleStep((int)step);
    sbValue->setSuffix(suffix);
    sbValue->setValue(dflt.index);
    connect(sbValue, &QSpinBox::editingFinished, this,
            &RawSourceExtWidget::valueChanged);
  }

  if (uiFlags & UI_FLAG_DBLSPINBOX) {
    dsbValue = new QDoubleSpinBox(this);
    dsbValue->setDecimals(decimals);
    dsbValue->setMinimum(min);
    dsbValue->setMaximum(max);
    dsbValue->setSingleStep(step);
    dsbValue->setSuffix(suffix);
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
      *src = dflt;
    }
    else {
      *src = RawSource(SOURCE_TYPE_NUMBER);
      if (sbValue)
        sbValue->setValue(src->index);
      if (dsbValue)
        dsbValue->setValue(src->index);  //   this needs fixing probably / step
    }

    update();
  }
}

void RawSourceExtWidget::valueChanged()
{
  if (!lock) {
    src->type = SOURCE_TYPE_NUMBER;
    if (sbValue)
      src->index = sbValue->value();
    else if(dsbValue)
      src->index = round(dsbValue->value() / step); // ???????? might need to be raw
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

  if (src->type == SOURCE_TYPE_NUMBER) {
    RawSourceWidget::setVisible(false);

    if (sbValue)
      sbValue->setValue(src->index);

    if (dsbValue)
      dsbValue->setValue(src->index); // fix this
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
