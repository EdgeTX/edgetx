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
                                  double step,
                                  QString prefix,
                                  QString suffix,
                                  bool isAvailable) :
  QWidget(parent),
  fimSource(nullptr),
  chkType(nullptr),
  cboSource(nullptr),
  sbValue(nullptr),
  dsbValue(nullptr),
  curveImage(nullptr)
{
  init(modelData, sharedItemModels, src, imFilter, uiFlags, dflt,
       typeLabel, min, max, precision, step, prefix, suffix, isAvailable);
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
                              double step,
                              QString prefix,
                              QString suffix,
                              bool isAvailable)
{
  this->modelData = modelData;
  this->src = src;
  this->dflt = dflt;
  this->imFilter = imFilter;
  this->uiFlags = uiFlags;
  this->typeLabel = typeLabel;
  this->min = min;
  this->max = max;
  this->precision = precision;
  this->step = step;
  this->prefix = prefix;
  this->suffix = suffix;
  this->isAvailable = isAvailable;

  fimSource = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource), imFilter, isAvailable);
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
    connect(cboSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RawSourceWidget::sourceChanged);
    layout->addWidget(cboSource);
  }

  if (uiFlags & UI_FLAG_VALUE) {
    if (precision > 1) {
      dsbValue = new QDoubleSpinBox();
      dsbValue->setAccelerated(true);
      dsbValue->setDecimals(precision == 10 ? 1 : precision == 100 ? 2 : 3);
      dsbValue->setMinimum(min / precision);
      dsbValue->setMaximum(max / precision);
      dsbValue->setSingleStep(step);
      dsbValue->setSuffix(suffix);
      dsbValue->setValue(dflt.index / precision);
      dsbValue->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
      dsbValue->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      connect(dsbValue, &QDoubleSpinBox::editingFinished, this,
              &RawSourceWidget::valueChanged);
      layout->addWidget(dsbValue);
    } else {
      sbValue = new QSpinBox();
      sbValue->setAccelerated(true);
      sbValue->setMinimum(min);
      sbValue->setMaximum(max);
      sbValue->setSingleStep(step);
      sbValue->setSuffix(suffix);
      sbValue->setValue(dflt.index);
      sbValue->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
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

void RawSourceWidget::shrink()
{
  //adjustSize();
  //resize(0, 0);
  //emit resized();
}

void RawSourceWidget::typeChanged(int state)
{
  if (!lock) {
    if (state == Qt::Checked)
      *src = RawSource();
    else
      *src = dflt;

    update();
    emit dataChanged();
  }
}

void RawSourceWidget::update()
{
  lock = true;

  if (chkType) {
    if (chkType->checkState() == Qt::Unchecked) {
      if (sbValue) {
        sbValue->setValue(src->index);
        sbValue->setVisible(true);
      }

      if (dsbValue) {
        dsbValue->setValue(src->index / precision);
        dsbValue->setVisible(true);
      }
    } else {
      if (sbValue) {
        sbValue->setVisible(false);
      }

      if (dsbValue) {
        dsbValue->setVisible(false);
      }
    }
  }

  if (cboSource) {
    if (src->type != SOURCE_TYPE_NUMBER) {
      cboSource->setCurrentIndex(cboSource->findData(src->toValue()));

      if (cboSource->currentIndex() < 0)
        cboSource->setCurrentIndex(Helpers::getFirstPosValueIndex(cboSource));

      cboSource->setVisible(true);
    } else {
      cboSource->setVisible(false);
    }
  }

  if (curveImage) {
    if (src->type == SOURCE_TYPE_CURVE) {
      curveImage->setIndex(src->index);

      if (abs(src->index) > 0 && abs(src->index) <= CPN_MAX_CURVES)
        curveImage->setPen(colors[abs(src->index) - 1], 3);
      else
        curveImage->setPen(Qt::black, 3);

      curveImage->draw();
      curveImage->setVisible(true);
    } else {
      curveImage->setVisible(false);
    }
  }

  shrink();
  lock = false;
}

//  this is normally invoked when extended trims setting changes
void RawSourceWidget::updateMinMax(int max)
{
  bool valueChanged = false;

  if (sbValue) {
    if (sbValue->maximum() == 0) {
      sbValue->setMinimum(-max * precision);
      if (chkType && !chkType->isChecked() && src->index < -max) {
        src->index = -max;
        valueChanged = true;
      }
    }

    if (sbValue->minimum() == 0) {
      sbValue->setMaximum(max * precision);
      if (chkType && !chkType->isChecked() && src->index > max) {
        src->index = max;
        valueChanged = true;
      }
    }
  } else if (dsbValue) {
    if (dsbValue->maximum() == 0) {
      dsbValue->setMinimum(-max * precision);
      if (chkType && !chkType->isChecked() && src->index < -max) {
        src->index = -max;
        valueChanged = true;
      }
    }

    if (dsbValue->minimum() == 0) {
      dsbValue->setMaximum(max * precision);
      if (chkType && !chkType->isChecked() && src->index > max) {
        src->index = max;
        valueChanged = true;
      }
    }
  }

  if (valueChanged) {
    update();
    emit dataChanged();
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

    update();
    emit dataChanged();
  }
}

void RawSourceWidget::sourceChanged(int index)
{
  if (!lock) {
    *src = RawSource(cboSource->itemData(index).toInt());
    update();
    emit dataChanged();
  }
}
