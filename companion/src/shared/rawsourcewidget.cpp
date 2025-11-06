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
                                  int listFilter,
                                  int flags,
                                  RawSource dflt,
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
  init(modelData, sharedItemModels, src, listFilter, flags, dflt,
       typeLabel, min, max, precision, step, prefix, suffix, isAvailable);
}

RawSourceWidget::RawSourceWidget(QWidget * parent) :
  QWidget(parent)
{
  init();
}

void RawSourceWidget::init()
{
  fimSource = nullptr;
  chkType = nullptr;
  cboSource = nullptr;
  sbValue = nullptr;
  dsbValue = nullptr;
  curveImage = nullptr;
  lock = false;
}

void RawSourceWidget::init(ModelData * modelData,
                           CompoundItemModelFactory * sharedItemModels,
                           RawSource * src,
                           int listFilter,
                           int flags,
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
  this->listFilter = listFilter;
  this->flags = flags;
  this->typeLabel = typeLabel;
  this->min = min;
  this->max = max;
  this->precision = precision;
  this->step = step < 0 ? Helpers::precisionToStep(precision) : step;
  this->prefix = prefix;
  this->suffix = suffix;
  this->isAvailable = isAvailable;

  fimSource = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource), listFilter, isAvailable);
  connectItemModelEvents(fimSource);

  // minimise padding around widget
  setContentsMargins(0, 0, 0, 0);
  // layout to group the requested ui objects
  QHBoxLayout *layout = new QHBoxLayout();
  // minimise padding around sub-widgets
  layout->setContentsMargins(0, 0, 0, 0);

  // need a checkbox to switch between value and source list
  if (flags & UI_FLAG_LIST && flags & UI_FLAG_VALUE) {
    chkType = new QCheckBox(typeLabel.isEmpty() ? tr("Source") : typeLabel);
    chkType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    if (src->type != SOURCE_TYPE_NUMBER) {
      chkType->setChecked(true);
    }

    connect(chkType, &QCheckBox::checkStateChanged, this,
            &RawSourceWidget::typeChanged);
    layout->addWidget(chkType);
  }

  if (flags & UI_FLAG_LIST) {
    cboSource = new QComboBox();
    cboSource->setModel(fimSource);
    cboSource->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboSource->setMaxVisibleItems(10);
    connect(cboSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RawSourceWidget::sourceChanged);
    layout->addWidget(cboSource);
  }

  if (flags & UI_FLAG_VALUE) {
    if (precision > 1) {
      dsbValue = new QDoubleSpinBox();
      dsbValue->setAccelerated(true);
      dsbValue->setDecimals(Helpers::precisionToDecimals(precision));
      dsbValue->setMinimum((double)min / (double)precision);
      dsbValue->setMaximum((double)max / (double)precision);
      dsbValue->setSingleStep(step);
      dsbValue->setSuffix(suffix);
      dsbValue->setValue((double)dflt.index / (double)precision);
      dsbValue->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
      dsbValue->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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

  if (flags & UI_FLAG_CURVE_IMAGE) {
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

void RawSourceWidget::setFilter(int filter, bool isAvailable)
{
  fimSource->setFilterFlags(filter, isAvailable);
}

void RawSourceWidget::shrink()
{
  // when parent resizes the widget seems to be resizing okay
  //adjustSize();
  //resize(0, 0);
  emit resized();
}

void RawSourceWidget::sourceChanged(int index)
{
  if (!lock) {
    *src = RawSource(cboSource->itemData(index).toInt());
    update();
    emit dataChanged(*src);
  }
}

void RawSourceWidget::typeChanged(int state)
{
  if (!lock) {
    if (state == Qt::Checked)
      *src = RawSource();
    else
      *src = dflt;

    update();
    emit dataChanged(*src);
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
        dsbValue->setValue((double)src->index / (double)precision);
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
void RawSourceWidget::updateMinMax(const int newMin, const int newMax)
{
  if (newMin > newMax) {
    qDebug() << "Error: new minimum" << newMin << "greater than new maximum" << newMax;
    return;
  }

  lock = true;
  bool valueChanged = false;

  if (newMin != min) {
    if (sbValue)
      sbValue->setMinimum(newMin);
    else if (dsbValue)
      dsbValue->setMinimum((double)newMin / (double)precision);
  }

  if (newMax != max) {
    if (sbValue)
      sbValue->setMaximum(newMax);
    else if (dsbValue)
      dsbValue->setMaximum((double)newMax / (double)precision);
  }

  if (src->type == SOURCE_TYPE_NUMBER) {
    if (src->index < newMin) {
      src->index = newMin;
      valueChanged = true;
    }

    if (src->index > newMax) {
      src->index = newMax;
      valueChanged = true;
    }
  }

  min = newMin;
  max = newMax;
  lock = false;

  if (valueChanged) {
    update();
    emit dataChanged(*src);
  }
}

void RawSourceWidget::valueChanged()
{
  if (!lock) {
    src->type = SOURCE_TYPE_NUMBER;
    if (sbValue)
      src->index = sbValue->value();
    else if(dsbValue)
      src->index = round(dsbValue->value() * (double)precision);

    update();
    emit dataChanged(*src);
  }
}
