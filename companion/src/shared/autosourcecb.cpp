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
  if (lock() || index < 0)
    return;

  bool ok;
  const int val = itemData(index).toInt(&ok);

  if (ok && getSource() != RawSource(val)) {
      setSource(RawSource(val));
      emit dataChanged(val);
      AutoWidget::dataChanged();
  }
}

void AutoSourceCB::setDefault(RawSource dflt)
{
  m_dflt = dflt;
}

void AutoSourceCB::setField(RawSource * field, RawSource dflt, GenericPanel * panel)
{
  setDefault(dflt);
  AutoSource::setField(field, panel);
}

void AutoSourceCB::setField(RawSource * field, CompoundItemModelFactory * itemModels,
                          int filter, bool isAvailable, RawSource dflt,
                          GenericPanel * panel)
{
  setField(field, dflt, panel);
  setModelFilter(itemModels, filter, isAvailable);
}

void AutoSourceCB::setField(RawSource * field, AbstractItemModel * itemModel,
                          int filter, bool isAvailable, RawSource dflt,
                          GenericPanel * panel)
{
  setField(field, dflt, panel);
  setModelFilter(itemModel, filter, isAvailable);
}

void AutoSourceCB::setFilter(int filter, bool isAvailable)
{
  if (m_itemModel) {
    if (!m_filteredSource) {
      m_filteredSource = new FilteredItemModel(m_itemModel, filter, isAvailable);

      if (m_filteredSource) {
        connectItemModelEvents(m_filteredSource);
        setLock(true);
        QComboBox::setModel(m_filteredSource);
        setLock(false);
      }
    } else {
      m_filteredSource->setFilterFlags(filter, isAvailable);
    }

    updateValue();
  }
}

void AutoSourceCB::setModelFilter(CompoundItemModelFactory * itemModels,
                                int filter, bool isAvailable)
{
  setModelFilter(itemModels->getItemModel(AbstractItemModel::IMID_RawSource),
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

void AutoSourceCB::setValueDefault()
{
  setSource(m_dflt);
}

void AutoSourceCB::updateValue()
{
  if (getSource().type != SOURCE_TYPE_NUMBER) {
    setLock(true);
    setCurrentIndex(findData(getSource().toValue()));;

    if (currentIndex() < 0)
      setCurrentIndex(Helpers::getFirstPosValueIndex(this));

    setLock(false);
  }
}
