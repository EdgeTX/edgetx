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

#include "autosourcecb.h"
#include "helpers.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

constexpr int AIM_RawSource { AbstractItemModel::IMID_RawSource };

AutoSourceCB::AutoSourceCB(QWidget * parent) :
  QComboBox(parent),
  AutoSource(),
  m_itemModel(nullptr),
  m_filteredSource(nullptr)
{
  setSizeAdjustPolicy(QComboBox::AdjustToContents);
  setMaxVisibleItems(10);
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AutoSourceCB::onCurrentIndexChanged);
}

AutoSourceCB::~AutoSourceCB()
{
  if (m_filteredSource)
    delete m_filteredSource;
}

void AutoSourceCB::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, [&] ()
    {
      setLock(true);
    }
  );

  connect(itemModel, &FilteredItemModel::updateComplete, [&] ()
    {
      updateValue();
      setLock(false);
    }
  );
}

void AutoSourceCB::onCurrentIndexChanged(int index)
{
  if (lock() || index < 0 || index >= count())
    return;

  bool ok;
  const int val = itemData(index).toInt(&ok);

  if (ok && getValue() != RawSource(val)) {
      setValue(RawSource(val));
      emit dataChanged(val);
      AutoWidget::dataChanged();
  }
}

void AutoSourceCB::setField(RawSource * field, RawSource dflt, GenericPanel * panel,
                            CompoundItemModelFactory * itemModels,
                          int filter, bool isAvailable)
{
  setField(field, dflt, panel, itemModels->getItemModel(AIM_RawSource),
           filter, isAvailable);
}

void AutoSourceCB::setField(RawSource * field, RawSource dflt, GenericPanel * panel,
                            AbstractItemModel * itemModel,
                            int filter, bool isAvailable)
{
  AutoSource::setField(field, dflt, panel);
  setModelFilter(itemModel, filter, isAvailable);
}

void AutoSourceCB::setFilter(int filter, bool isAvailable)
{
  if (m_itemModel) {
    setLock(true);

    if (!m_filteredSource) {
      m_filteredSource = new FilteredItemModel(m_itemModel, filter, isAvailable);

      if (m_filteredSource) {
        connectItemModelEvents(m_filteredSource);
        QComboBox::setModel(m_filteredSource);
      }
    } else {
      m_filteredSource->setFilterFlags(filter, isAvailable);
    }

    setLock(false);
    updateValue();
  }
}

void AutoSourceCB::setModelFilter(CompoundItemModelFactory * itemModels,
                                  int filter, bool isAvailable)
{
  setModelFilter(itemModels->getItemModel(AIM_RawSource),
                 filter, isAvailable);
}

void AutoSourceCB::setModelFilter(AbstractItemModel * itemModel,
                                  int filter, bool isAvailable)
{
  m_itemModel = itemModel;

  if (m_filteredSource)
    delete m_filteredSource;

  setFilter(filter, isAvailable);
}

void AutoSourceCB::updateValue()
{
  if (getValue().type != SOURCE_TYPE_NUMBER) {
    setLock(true);
    setCurrentIndex(findData(getValue().toValue()));;

    if (currentIndex() < 0)
      setCurrentIndex(Helpers::getFirstPosValueIndex(this));

    setLock(false);
  }
}
