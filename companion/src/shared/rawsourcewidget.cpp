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

#include "rawsourcewidget.h"
#include "helpers.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"
#include "curveimagewidget.h"

#include <float.h>

RawSourceWidget::RawSourceWidget(QWidget * parent,
                                  ModelData * modelData,
                                  CompoundItemModelFactory * sharedItemModels,
                                  RawSource * src,
                                  int imFilter,
                                  int uiFlags,
                                  RawSource dflt,
                                  QString typeLabel,
                                  int min,
                                  int max,
                                  int precision,
                                  int decimals,
                                  double step,
                                  QString prefix,
                                  QString suffix) :
  QWidget(parent)
{
  init(modelData, sharedItemModels, src, imFilter, uiFlags, dflt,
       typeLabel, min, max, precision, decimals, step, prefix, suffix);
}

void RawSourceWidget::init(ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              RawSource * src,
                              int imFilter,
                              int uiFlags,
                              RawSource dflt,
                              QString typeLabel,
                              int min,
                              int max,
                              int precision,
                              int decimals,
                              double step,
                              QString prefix,
                              QString suffix)
{
  this->modelData = modelData;
  this->src = src;
  this->dflt = dflt;
  this->imFilter = imFilter;
  this->uiFlags = uiFlags;
  this->typeLabel = typeLabel;
  this->min = min;
  this->max = max;
  this->step = step;
  this->precision = precision;
  this->decimals = decimals;
  this->prefix = prefix;
  this->suffix = suffix;

  fimSource = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource), imFilter);
  connectItemModelEvents(fimSource);

  // layout to group the requested ui objects
  QHBoxLayout *layout = new QHBoxLayout();

  if (uiFlags & UI_FLAG_LIST && uiFlags & UI_FLAG_VALUE) {
    chkType = new QCheckBox(typeLabel.isEmpty() ? tr("Source") : typeLabel);
    chkType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    connect(chkType, &QCheckBox::checkStateChanged, this,
            &RawSourceWidget::typeChanged);

    layout->addWidget(chkType);
  }

  if (uiFlags & UI_FLAG_LIST) {
    cboSource = new QComboBox();
    cboSource->setModel(fimSource);
    cboSource->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboSource->setMaxVisibleItems(10);
    connect(cboSource, QOverload<int>::of(&QComboBox::currentIndexChanged), [&] (int index)
    {
      if (!lock) {
        *src = RawSource(cboSource->itemData(index).toInt());
        emit dataChanged();
        update();
      }
    });

    layout->addWidget(cboSource);
  }

  if (uiFlags & UI_FLAG_VALUE) {
    if(precision) {
      dsbValue = new QDoubleSpinBox();
      dsbValue->setAccelerated(true);
      dsbValue->setDecimals(decimals);
      dsbValue->setMinimum(min);
      dsbValue->setMaximum(max);
      dsbValue->setSingleStep(step);
      dsbValue->setSuffix(suffix);
      dsbValue->setValue(dflt.index); // conversion / step ????
      dsbValue->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
      dsbValue->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      connect(dsbValue, &QDoubleSpinBox::editingFinished, this,
              &RawSourceWidget::valueChanged);
      layout->addWidget(dsbValue);
    } else {
      sbValue = new QSpinBox();
      sbValue->setAccelerated(true);
      sbValue->setMinimum(min);
      sbValue->setMaximum(max);
      sbValue->setSingleStep((int)step);
      sbValue->setSuffix(suffix);
      sbValue->setValue(dflt.index);
      sbValue->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
      sbValue->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      connect(sbValue, &QSpinBox::editingFinished, this,
              &RawSourceWidget::valueChanged);
      layout->addWidget(sbValue);
    }
  }

  if (uiFlags & UI_FLAG_CURVE_IMAGE) {
    curveImage = new CurveImageWidget(this);
    curveImage->setFixedSize(QSize(100, 100));
    curveImage->set(modelData, getCurrentFirmware(), sharedItemModels,
                    src->index, Qt::black, 3);
    curveImage->setGrid(Qt::gray, 2);
    connect(curveImage, &CurveImageWidget::doubleClicked, [&]() { curveImage->edit(); });
    layout->addWidget(curveImage);
  }

  setLayout(layout);
  update();
}

RawSourceWidget::~RawSourceWidget()
{
  if (fimSource)
    delete fimSource;
}

void RawSourceWidget::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, [&] ()
  {
    lock = true;
  });

  connect(itemModel, &FilteredItemModel::updateComplete, [&] ()
  {
    update();
    lock = false;
  });
}

void RawSourceWidget::setDefault(RawSource value)
{
  dflt = value;
}

void RawSourceWidget::setFilterFlags(int flags)
{
  fimSource->setFilterFlags(flags);
}

//  TODO this makes no sense as it is designed to be called from curvereferencewidget
//  and thus selective visibility
void RawSourceWidget::setVisible(bool state)
{
  if (chkType)
    chkType->setVisible(state);
  if (cboSource)
    cboSource->setVisible(state);
  if (sbValue)
    sbValue->setVisible(state);
  if (dsbValue)
    dsbValue->setVisible(state);
  if (curveImage)
    curveImage->setVisible(state);
}

void RawSourceWidget::shrink()
{
  adjustSize();
  resize(0, 0);
  emit resized();
}

void RawSourceWidget::typeChanged(int state)
{
  if (!lock) {
    if (state == Qt::Checked) {
      *src = dflt;
    } else {
      *src = RawSource(SOURCE_TYPE_NUMBER);
      if (sbValue)
        sbValue->setValue(src->index);
      if (dsbValue)
        dsbValue->setValue(src->index / precision);
    }

    emit dataChanged();
    update();
  }
}

void RawSourceWidget::update()
{
  lock = true;

  if (cboSource && src->type != SOURCE_TYPE_NUMBER) {
    cboSource->setCurrentIndex(cboSource->findData(src->toValue()));

    if (cboSource->currentIndex() < 0)
      cboSource->setCurrentIndex(Helpers::getFirstPosValueIndex(cboSource));
  }

  if (src->type == SOURCE_TYPE_NUMBER) {
    RawSourceWidget::setVisible(false);

    if (sbValue)
      sbValue->setValue(src->index);

    if (dsbValue)
      dsbValue->setValue(src->index / precision);
  }
  else {
    RawSourceWidget::setVisible(true);
    RawSourceWidget::update();

    if (sbValue)
      sbValue->setVisible(false);

    if (dsbValue)
      dsbValue->setVisible(false);
  }

  if (curveImage && src->type == SOURCE_TYPE_CURVE) {
    curveImage->setIndex(src->index);

    if (abs(src->index) > 0 && abs(src->index) <= CPN_MAX_CURVES)
      curveImage->setPen(colors[abs(src->index) - 1], 3);
    else
      curveImage->setPen(Qt::black, 3);

    curveImage->draw();
    curveImage->setVisible(true);
  }

  shrink();
  lock = false;
}

//  this is normally invoked when extended trims setting changes
void RawSourceWidget::updateMinMax(int max)
{
  if (sbValue) {
    if (sbValue->maximum() == 0) {
      sbValue->setMinimum(-max * precision);
      if (!chkType->isChecked() && src->index < -max) {
        src->index = -max;
      }
    }

    if (sbValue->minimum() == 0) {
      sbValue->setMaximum(max * precision);
      if (!chkType->isChecked() && src->index > max) {
        src->index = max;
      }
    }
  }

  if (dsbValue) {
    if (dsbValue->maximum() == 0) {
      dsbValue->setMinimum(-max * precision);
      if (!chkType->isChecked() && src->index < -max) {
        src->index = -max;
      }
    }

    if (dsbValue->minimum() == 0) {
      dsbValue->setMaximum(max * precision);
      if (!chkType->isChecked() && src->index > max) {
        src->index = max;
      }
    }
  }
}

void RawSourceWidget::valueChanged()
{
  if (!lock) {
    src->type = SOURCE_TYPE_NUMBER;
    if (sbValue)
      src->index = sbValue->value();
    else if(dsbValue)
      src->index = round(dsbValue->value() * precision);

    emit dataChanged();
    update();
  }
}
