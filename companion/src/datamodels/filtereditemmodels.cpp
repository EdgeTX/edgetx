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

#include "filtereditemmodels.h"

FilteredItemModel::FilteredItemModel(AbstractItemModel * sourceModel, int flags) :
  QSortFilterProxyModel(nullptr),
  filterFlags(0),
  m_id(0),
  m_name("")
{
  setFilterRole(AbstractItemModel::IMDR_Available);
  setFilterKeyColumn(0);
  setFilterFlags(flags);
  setDynamicSortFilter(true);
  setSourceModel(sourceModel);

  AbstractDynamicItemModel * itemModel = qobject_cast<AbstractDynamicItemModel *>(sourceModel);
  if (itemModel) {
    connect(itemModel, &AbstractDynamicItemModel::aboutToBeUpdated, this, &FilteredItemModel::onAboutToBeUpdated);
    connect(itemModel, &AbstractDynamicItemModel::updateComplete, this, &FilteredItemModel::onUpdateComplete);
  }
}

void FilteredItemModel::setFilterFlags(int flags)
{
  if (filterFlags != flags) {
    filterFlags = flags;
    invalidateFilter();
  }
}

bool FilteredItemModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  const QModelIndex & srcIdx = sourceModel()->index(sourceRow, 0, sourceParent);
  if (!srcIdx.isValid() || !sourceModel()->data(srcIdx, filterRole()).toBool())
    return false;

  if (!filterFlags)
    return true;

  bool ok;
  const int flags = sourceModel()->data(srcIdx, AbstractItemModel::IMDR_Flags).toInt(&ok);
  return (ok && (!flags || (filterFlags & flags)));
}

void FilteredItemModel::update() const
{
  AbstractDynamicItemModel * itemModel = qobject_cast<AbstractDynamicItemModel *>(sourceModel());
  if (itemModel)
    itemModel->update();
}

void FilteredItemModel::onAboutToBeUpdated()
{
  emit aboutToBeUpdated();
}

void FilteredItemModel::onUpdateComplete()
{
  emit updateComplete();
}

// static
void FilteredItemModel::dumpItemModelContents(FilteredItemModel * itemModel)
{
  if (itemModel) {
    qDebug() << "id:" << itemModel->getId() << "name:" << itemModel->getName();

    for (int i = 0; i < itemModel->rowCount(); ++i) {
      qDebug() << "row:"   << i
               << "text:"  << itemModel->data(itemModel->index(i, 0), Qt::DisplayRole).toString()
               << "id:"    << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Id).toInt()
               << "avail:" << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Available).toBool()
               << "flags:" << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Flags).toInt()
               << "type:"  << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Type).toInt();
    }
  }
  else
    qDebug() << "Error: model not of class FilteredItemModel";
}

//
// FilteredItemModelFactory
//

FilteredItemModelFactory::FilteredItemModelFactory()
{
}

FilteredItemModelFactory::~FilteredItemModelFactory()
{
  unregisterItemModels();
}

int FilteredItemModelFactory::registerItemModel(FilteredItemModel * itemModel, const QString name, const int id)
{
  if (itemModel) {
    if (isItemModelRegistered(name)) {
      qDebug() << "Error: item model already registered with name:" << name;
      return 0;
    }
    else if (id > -1 && isItemModelRegistered(id)) {
      qDebug() << "Error: item model already registered with id:" << id;
      return 0;
    }
    else {
      int fid = id;
      if (fid < 0) {
        fid = registeredItemModels.count() + 1;
        AbstractItemModel * mdl = qobject_cast<AbstractItemModel *>(itemModel->sourceModel());
        if (mdl) {
          fid += mdl->getId() * 128;
        }
      }
      itemModel->setId(fid);
      itemModel->setName(name);
      registeredItemModels.append(itemModel);
      return fid;
    }
  }

  qDebug() << "Error: invalid filtered item model pointer";
  return 0;
}

void FilteredItemModelFactory::unregisterItemModels()
{
  foreach (FilteredItemModel * itemModel, registeredItemModels) {
    delete itemModel;
  }
}

void FilteredItemModelFactory::unregisterItemModel(const int id)
{
  FilteredItemModel * itemModel =  getItemModel(id);
  if (itemModel)
    delete itemModel;
}

bool FilteredItemModelFactory::isItemModelRegistered(const int id) const
{
  return getItemModel(id);
}

bool FilteredItemModelFactory::isItemModelRegistered(const QString name) const
{
  return getItemModel(name);
}

FilteredItemModel * FilteredItemModelFactory::getItemModel(const int id) const
{
  foreach (FilteredItemModel * itemModel, registeredItemModels) {
    if (itemModel->getId() == id)
      return itemModel;
  }

  return nullptr;
}

FilteredItemModel * FilteredItemModelFactory::getItemModel(const QString name) const
{
  foreach (FilteredItemModel * itemModel, registeredItemModels) {
    if (itemModel->getName() == name)
      return itemModel;
  }

  return nullptr;
}

void FilteredItemModelFactory::update()
{
  foreach (FilteredItemModel * itemModel, registeredItemModels) {
    itemModel->update();
  }
}

void FilteredItemModelFactory::dumpItemModelContents(const int id) const
{
  FilteredItemModel::dumpItemModelContents(getItemModel(id));
}

void FilteredItemModelFactory::dumpItemModelContents(const QString name) const
{
  FilteredItemModel::dumpItemModelContents(getItemModel(name));
}

void FilteredItemModelFactory::dumpAllItemModelContents() const
{
  foreach (FilteredItemModel * itemModel, registeredItemModels) {
    FilteredItemModel::dumpItemModelContents(itemModel);
  }
}

//
//  CurveRefFilteredFactory
//

CurveRefFilteredFactory::CurveRefFilteredFactory(CompoundItemModelFactory * sharedItemModels, const int curveFlags, const int gvarRefFlags)
{
  if (!sharedItemModels) {
    qDebug() << "Error: invalid compound item model factory pointer";
    return;
  }

  registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_Curve), curveFlags),
                                          fidToString(CRFIM_CURVE), CRFIM_CURVE);
  registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_CurveRefType)),
                                          fidToString(CRFIM_TYPE), CRFIM_TYPE);
  registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_CurveRefFunc)),
                                          fidToString(CRFIM_FUNC), CRFIM_FUNC);
}

CurveRefFilteredFactory::~CurveRefFilteredFactory()
{
}

//  static
QString CurveRefFilteredFactory::fidToString(const int value)
{
  switch(value) {
    case CRFIM_CURVE:
      return "Curves";
    case CRFIM_GVARREF:
      return "Global Variables";
    case CRFIM_TYPE:
      return "Types";
    case CRFIM_FUNC:
      return "Functions";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}
