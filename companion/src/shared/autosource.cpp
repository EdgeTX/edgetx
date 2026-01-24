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

#include "autosource.h"
#include "helpers.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

AutoSource::AutoSource(QWidget * parent) :
  QComboBox(parent),
  AutoWidget()
{
  m_rawSource = nullptr;
  m_itemModel = nullptr;
  m_filteredSource = nullptr;
  setSizeAdjustPolicy(QComboBox::AdjustToContents);
  setMaxVisibleItems(10);
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AutoSource::onCurrentIndexChanged);
}

AutoSource::AutoSource(RawSource * field, CompoundItemModelFactory * itemModels,
                       int filter, bool isAvailable,
                       GenericPanel * panel,
                       QWidget * parent) :
  QComboBox(parent),
  AutoWidget()
{
  AutoSource(parent);
  setField(field, itemModels, filter, isAvailable, panel);
}

AutoSource::~AutoSource()
{
  if (m_filteredSource)
    delete m_filteredSource;
}

void AutoSource::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, [&] ()
    {
      setLock(true);
    }
  );

  connect(itemModel, &FilteredItemModel::updateComplete, [&] ()
    {
      update();
      setLock(false);
    }
  );
}

void AutoSource::onCurrentIndexChanged(int index)
{
  if (lock() || index < 0)
    return;

  bool ok;
  const int val = itemData(index).toInt(&ok);

  if (ok) {
    if (m_rawSource && m_rawSource->toValue() != val)
      *m_rawSource = RawSource(val);
    else
      return;

    emit currentDataChanged(val);
    dataChanged();
  }
}

void AutoSource::setField(RawSource * field, CompoundItemModelFactory * itemModels,
                          int filter, bool isAvailable, GenericPanel * panel)
{
  setField(field, itemModels->getItemModel(AbstractItemModel::IMID_RawSource),
           filter, isAvailable, panel);
}

void AutoSource::setField(RawSource * field, AbstractItemModel * itemModel,
                          int filter, bool isAvailable, GenericPanel * panel)
{
  m_rawSource = field;
  m_itemModel = itemModel;
  setPanel(panel);

  if (m_filteredSource)
    delete m_filteredSource;

  setFilter(filter, isAvailable);
}

void AutoSource::setFilter(int filter, bool isAvailable)
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

void AutoSource::updateValue()
{
  setLock(true);

  if (m_rawSource) {
    setCurrentIndex(findData(m_rawSource->toValue()));

    if (currentIndex() < 0)
      setCurrentIndex(Helpers::getFirstPosValueIndex(this));
  }

  setLock(false);
}
