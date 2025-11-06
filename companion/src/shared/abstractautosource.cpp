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

#include "abstractautosource.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

AbstractAutoSource::AbstractAutoSource() :
  AutoWidget(),
  m_rawSource(nullptr),
  m_itemModel(nullptr),
  m_filteredSource(nullptr)
{
}

AbstractAutoSource::~AbstractAutoSource()
{
  if (m_filteredSource)
    delete m_filteredSource;
}

void AbstractAutoSource::setField(RawSource * field, CompoundItemModelFactory * itemModels,
                          int filter, bool isAvailable, GenericPanel * panel)
{
  setField(field, panel);
  setModelFilter(itemModels, filter, isAvailable);
}

void AbstractAutoSource::setField(RawSource * field, AbstractItemModel * itemModel,
                          int filter, bool isAvailable, GenericPanel * panel)
{
  setField(field, panel);
  setModelFilter(itemModel, filter, isAvailable);
}

void AbstractAutoSource::setField(RawSource * field, GenericPanel * panel)
{
  m_rawSource = field;
  setPanel(panel);
}

void AbstractAutoSource::setFilter(int filter, bool isAvailable)
{
  if (m_itemModel) {
    if (!m_filteredSource) {
      m_filteredSource = new FilteredItemModel(m_itemModel, filter, isAvailable);

      if (m_filteredSource) {
        connectItemModelEvents(m_filteredSource);
      }
    } else {
      m_filteredSource->setFilterFlags(filter, isAvailable);
    }
  }
}

void AbstractAutoSource::setModelFilter(CompoundItemModelFactory * itemModels,
                                int filter, bool isAvailable)
{
  setModelFilter(itemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                 filter, isAvailable);
}

void AbstractAutoSource::setModelFilter(AbstractItemModel * itemModel,
                                int filter, bool isAvailable)
{
  m_itemModel = itemModel;

  if (m_filteredSource)
    delete m_filteredSource;

  setFilter(filter, isAvailable);
}
