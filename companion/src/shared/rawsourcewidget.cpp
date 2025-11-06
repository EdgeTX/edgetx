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

RawSourceWidget::RawSourceWidget(QWidget * parent,
                                 ModelData * modelData,
                                 CompoundItemModelFactory * sharedItemModels,
                                 RawSource * rawSource,
                                 RawSource dflt,
                                 int filterFlags) :
  QWidget(parent),
  lock(false),
  fimRawSource(nullptr),
  cboSource(nullptr)
{
  init(modelData, sharedItemModels, rawSource, dflt, filterFlags);
}

void RawSourceWidget::init(ModelData * modelData,
                           CompoundItemModelFactory * sharedItemModels,
                           RawSource * rawSource,
                           RawSource defValue,
                           int filterFlags)
{
  fimRawSource = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource), filterFlags);
  connectItemModelEvents(fimRawSource);

  cboSource = new QComboBox(this);
  cboSource->setModel(fimRawSource);
  cboSource->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  cboSource->setMaxVisibleItems(10);
  cboSource->setCurrentIndex(Helpers::getFirstPosValueIndex(cboSource));
  connect(cboSource, QOverload<int>::of(&QComboBox::currentIndexChanged), [&] (int index)
  {
    if (!lock) {
      *rawSource = RawSource(cboSource->itemData(index).toInt());
      update();
    }
  });

  update(false);
}

RawSourceWidget::~RawSourceWidget()
{
  if (fimRawSource)
    delete fimRawSource;
}

void RawSourceWidget::setDefault(RawSource value)
{
  dflt = value;
}

void RawSourceWidget::setFilterFlags(int flags)
{
  fimRawSource->setFilterFlags(flags);
}

void RawSourceWidget::setVisible(bool state)
{
  cboSource->setVisible(state);

  emit resize();
}

void RawSourceWidget::update(bool notify)
{
  lock = true;

  cboSource->setCurrentIndex(cboSource->findData(rawSource->toValue()));
  if (cboSource->currentIndex() < 0)
    cboSource->setCurrentIndex(Helpers::getFirstPosValueIndex(cboSource));

  if (notify)
    emit dataChanged();

  lock = false;
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
