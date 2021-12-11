/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "modelslist.h"

TreeItem::TreeItem(const QVector<QVariant> & itemData):
  itemData(itemData),
  parentItem(NULL),
  categoryIndex(-1),
  modelIndex(-1),
  flags(0),
  highlightRX(false)
{
}

TreeItem::TreeItem(TreeItem * parent, int categoryIndex, int modelIndex):
  TreeItem(QVector<QVariant>(parent->columnCount()))
{
  setParent(parent);
  setCategoryIndex(categoryIndex);
  setModelIndex(modelIndex);
}

TreeItem::~TreeItem()
{
  qDeleteAll(childItems);
}

TreeItem * TreeItem::child(int number)
{
  return childItems.value(number);
}

int TreeItem::childCount() const
{
  return childItems.count();
}

int TreeItem::childNumber() const
{
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

  return 0;
}

int TreeItem::columnCount() const
{
  return itemData.count();
}

QVariant TreeItem::data(int column) const
{
  return itemData.value(column);
}

TreeItem *TreeItem::insertChild(const int row, int categoryIndex, int modelIndex)
{
  TreeItem * item = new TreeItem(this, categoryIndex, modelIndex);
  childItems.insert(row, item);
  return item;
}

TreeItem * TreeItem::appendChild(int categoryIndex, int modelIndex)
{
  return insertChild(childItems.size(), categoryIndex, modelIndex);
}

bool TreeItem::removeChildren(int position, int count)
{
  if (position < 0 || position + count > childItems.size())
    return false;

  for (int row = 0; row < count; ++row)
    delete childItems.takeAt(position);

  return true;
}

bool TreeItem::insertChildren(int row, int count)
{
  for (int i=0; i < count; ++i) {
    insertChild(row + i, -1, -1);
  }
  return true;
}

bool TreeItem::setData(int column, const QVariant & value)
{
  if (column < 0 || column >= itemData.size())
    return false;

  itemData[column] = value;
  return true;
}

void TreeItem::setFlag(const quint16 & flag, const bool on)
{
  if (on)
    flags |= flag;
  else
    flags &= ~flag;
}

bool TreeItem::isCategory() const
{
  return (modelIndex < 0 && categoryIndex > -1);
}

bool TreeItem::isModel() const
{
  return (modelIndex > -1);
}


/*
 * TreeModel
*/

TreeModel::TreeModel(RadioData * radioData, QObject * parent):
  QAbstractItemModel(parent),
  radioData(radioData),
  availableEEpromSize(-1)
{
  Board::Type board = getCurrentBoard();
  hasCategories = getCurrentFirmware()->getCapability(Capability::HasModelCategories);
  QVector<QVariant> labels;
  if (!hasCategories)
    labels << tr("Index");
  labels << tr("Name");
  if (!(IS_FAMILY_HORUS_OR_T16(board) || IS_SKY9X(board))) {
    labels << tr("Size");
  }
  labels << tr("RX #");
  rootItem = new TreeItem(labels);
  // uniqueId and version for drag/drop operations (see encodeHeaderData())
  mimeHeaderData.instanceId = QUuid::createUuid();
  mimeHeaderData.dataVersion = 1;

  refresh();
  //connect(this, &QAbstractItemModel::rowsAboutToBeRemoved, this, &TreeModel::onRowsAboutToBeRemoved);
  connect(this, &QAbstractItemModel::rowsRemoved, this, &TreeModel::onRowsRemoved);
}

TreeModel::~TreeModel()
{
  delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
  return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex & index, int role) const
{
  if (!index.isValid())
    return QVariant();

  TreeItem * item = getItem(index);

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return item->data(index.column());
  }

  if (role == Qt::FontRole && item->isModel() && item->getModelIndex() == (int)radioData->generalSettings.currModelIndex) {
    QFont font;
    font.setBold(true);
    return font;
  }

  if (role == Qt::ForegroundRole && (item->getFlags() & TreeItem::MarkedForCut)) {
    return QPalette().brush(QPalette::Disabled, QPalette::Text);
  }

  if (role == Qt::ForegroundRole && item->isModel()) {
    if (index.column() == (item->columnCount() - 1) && item->isHighlightRX()) {
      QBrush brush;
      brush.setColor(Qt::red);
      return brush;
    }
  }

  return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  if (index.isValid()) {
    if (getItem(index)->isCategory())
      f |= Qt::ItemIsEditable;
    else
      f |= Qt::ItemIsDragEnabled;  // TODO drag/drop categories
  }
  f |= Qt::ItemIsDropEnabled;

  //qDebug() << f;
  return f;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);

  return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex & parent) const
{
  if (parent.isValid() && parent.column() != 0)
    return QModelIndex();

  TreeItem * parentItem = getItem(parent);
  TreeItem * childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

  TreeItem * childItem = getItem(index);
  TreeItem * parentItem = childItem->parent();

  if (parentItem == rootItem)
    return QModelIndex();

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex & parent)
{
  TreeItem * parentItem = getItem(parent);
  if (!parentItem)
    return false;

  bool success = true;
  if (position >= 0 && rows > 0) {
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
  }

  return success;
}

/* unused but possibly useful in future
bool TreeModel::insertRows(int row, int count, const QModelIndex & parent)
{
  TreeItem * parentItem = getItem(parent);
  if (!parentItem)
    return false;

  bool success = true;
  if (row >= 0 && count > 0) {
    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildren(row, count);
    endInsertRows();
  }

  return success;
}  */

int TreeModel::rowCount(const QModelIndex &parent) const
{
  TreeItem * parentItem = getItem(parent);
  return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if (role != Qt::EditRole)
    return false;

  if (!index.isValid())
    return false;

  TreeItem * item = getItem(index);
  bool result = item->setData(index.column(), value);

  if (result) {
    emit dataChanged(index, index);
  }

  return result;
}

QStringList TreeModel::mimeTypes() const
{
  QStringList types;
  types << "application/x-companion-modeldata";
  types << "application/x-companion-generaldata";
  //type << "application/x-companion-radiodata-header";  // supported but not advertised, must be in conjunction with one of the above
  return types;
}

Qt::DropActions TreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions TreeModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

// This method encodes all the data on default drag operation, including general radio settings. This is useful for eg. Compare dialog/model printer.
QMimeData * TreeModel::mimeData(const QModelIndexList & indexes) const
{
  QMimeData * mimeData = new QMimeData();
  getModelsMimeData(indexes, mimeData);
  getGeneralMimeData(mimeData);
  getHeaderMimeData(mimeData);
  return mimeData;
}

bool TreeModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
  Q_UNUSED(action);
  //qDebug() << action << row << column << parent.row();

  // we do not accept dropped general settings right now (user must copy/paste those)
  if (hasHeaderMimeData(data) && hasModelsMimeData(data) && (row > -1 || parent.isValid()))
    return true;

  return false;
}

bool TreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
  if (!canDropMimeData(data, action, row, column, parent))
    return false;

  if (action == Qt::IgnoreAction)
    return true;

  QModelIndex idx;
  bool isInsert = false;
  if (row > -1) {
    // dropped between rows (insert)
    isInsert = true;
    idx = index(row, column, parent);
  }
  else if (parent.isValid()) {
    // was dropped on a row (overwrite)
    idx = parent;
  }
  else {
    // dropped who knows where, (shouldn't be here though due check in canDropMimeData())
    return false;
  }
  //qDebug() << action << row << column << parent.row() << idx << idx.row() << hasOwnMimeData(data);

  // Force drops from other file windows to be copy actions because we don't want to delete our models.
  if (action == Qt::MoveAction && !hasOwnMimeData(data))
    action = Qt::CopyAction;

  // canDropMimeData() only accepts models
  emit modelsDropped(data, idx, isInsert, (action == Qt::MoveAction));

  return true;
}

QMimeData *TreeModel::getModelsMimeData(const QModelIndexList & indexes, QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeModelsData(indexes, &mData);
  mimeData->setData("application/x-companion-modeldata", mData);
  return mimeData;
}

QMimeData *TreeModel::getGeneralMimeData(QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeGeneralData(&mData);
  mimeData->setData("application/x-companion-generaldata", mData);
  return mimeData;
}

QMimeData *TreeModel::getHeaderMimeData(QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeHeaderData(&mData);
  mimeData->setData("application/x-companion-radiodata-header", mData);
  return mimeData;
}

QUuid TreeModel::getMimeDataSourceId(const QMimeData * mimeData) const
{
  MimeHeaderData header;
  decodeHeaderData(mimeData, &header);
  return header.instanceId;
}

bool TreeModel::hasSupportedMimeData(const QMimeData * mimeData) const
{
  foreach (const QString & mtype, mimeTypes()) {
    if (mimeData->hasFormat(mtype))
      return true;
  }
  return false;
}

bool TreeModel::hasModelsMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-modeldata");
}

bool TreeModel::hasGenralMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-generaldata");
}

bool TreeModel::hasHeaderMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-radiodata-header");
}

// returns true if mime data origin was this data model (vs. from another file window)
bool TreeModel::hasOwnMimeData(const QMimeData * mimeData) const
{
  return (getMimeDataSourceId(mimeData) == mimeHeaderData.instanceId);
}

void TreeModel::encodeModelsData(const QModelIndexList & indexes, QByteArray * data) const
{
  foreach (const QModelIndex &index, indexes) {
    if (index.isValid() && index.column() == 0) {
      if (!getItem(index)->isCategory()) {  // TODO: encode categoreis also
        data->append('M');
        data->append((char *)&radioData->models[getModelIndex(index)], sizeof(ModelData));
      }
    }
  }
}

void TreeModel::encodeGeneralData(QByteArray * data) const
{
  data->append('G');
  data->append((char *)&radioData->generalSettings, sizeof(GeneralSettings));
}

void TreeModel::encodeHeaderData(QByteArray * data) const
{
  // We use a unique ID representing this TreeModel instance (a unique file).
  // This can be used eg. to detect cross-file drop operations.
  QDataStream stream(data, QIODevice::WriteOnly);
  stream << mimeHeaderData.dataVersion;
  stream << mimeHeaderData.instanceId;
}

// static
bool TreeModel::decodeHeaderData(const QMimeData * mimeData, MimeHeaderData * header)
{
  if (header && mimeData->hasFormat("application/x-companion-radiodata-header")) {
    QByteArray data = mimeData->data("application/x-companion-radiodata-header");
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> header->dataVersion >> header->instanceId;
    return true;
  }
  return false;
}

// static
bool TreeModel::decodeMimeData(const QMimeData * mimeData, QVector<ModelData> * models, GeneralSettings * gs, bool * hasGenSet)
{
  bool ret = false;
  char * gData;

  if (hasGenSet)
    *hasGenSet = false;

  if (models && mimeData->hasFormat("application/x-companion-modeldata")) {
    QByteArray mdlData = mimeData->data("application/x-companion-modeldata");
    gData = mdlData.data();
    int size = 0;
    while (size < mdlData.size()) {
      char c = *gData++;
      if (c != 'M')
        break;
      ModelData model(*((ModelData *)gData));
      models->append(model);
      gData += sizeof(ModelData);
      size += sizeof(ModelData) + 1;
      ret = true;
    }
  }

  // General settings
  if (gs && mimeData->hasFormat("application/x-companion-generaldata")) {
    QByteArray genData = mimeData->data("application/x-companion-generaldata");
    gData = genData.data();
    char c = *gData++;
    if (c == 'G') {
      *gs = *((GeneralSettings *)gData);
      ret = true;
      if (hasGenSet)
        *hasGenSet = true;
    }
  }

  return ret;
}

// static
int TreeModel::countModelsInMimeData(const QMimeData * mimeData)
{
  int ret = 0;
  if (mimeData->hasFormat("application/x-companion-modeldata")) {
    QByteArray mdlData = mimeData->data("application/x-companion-modeldata");
    ret = mdlData.size() / (sizeof(ModelData) + 1);
  }
  return ret;
}


TreeItem * TreeModel::getItem(const QModelIndex & index) const
{
  if (index.isValid()) {
    TreeItem * item = static_cast<TreeItem *>(index.internalPointer());
    if (item) {
      return item;
    }
  }
  return rootItem;
}

// recursive
QModelIndex TreeModel::getIndexForModel(const int modelIndex, QModelIndex parent)
{
  for (int i=0; i < rowCount(parent); ++i) {
    QModelIndex idx = index(i, 0, parent);
    if (hasChildren(idx) && (idx = getIndexForModel(modelIndex, idx)).isValid())
      return idx;
    if (getItem(idx)->getModelIndex() == modelIndex)
      return idx;
  }
  return QModelIndex();
}

QModelIndex TreeModel::getIndexForCategory(const int categoryIndex)
{
  for (int i=0; i < rowCount(); ++i) {
    if (getItem(index(i, 0))->getCategoryIndex() == categoryIndex)
      return index(i, 0);
  }
  return QModelIndex();
}

int TreeModel::getAvailableEEpromSize()
{
  return availableEEpromSize;
}

int TreeModel::getModelIndex(const QModelIndex & index) const
{
  return getItem(index)->getModelIndex();
}

int TreeModel::getCategoryIndex(const QModelIndex & index) const
{
  return getItem(index)->getCategoryIndex();
}

int TreeModel::rowNumber(const QModelIndex & index) const
{
  return getItem(index)->childNumber();
}

bool TreeModel::isCategoryType(const QModelIndex & index) const
{
  return index.isValid() && getItem(index)->isCategory();
}

bool TreeModel::isModelType(const QModelIndex & index) const
{
  return index.isValid() && getItem(index)->isModel();
}

void TreeModel::markItemForCut(const QModelIndex & index, bool on)
{
  if (index.isValid() && index.column() == 0)
    getItem(index)->setFlag(TreeItem::MarkedForCut, on);
}

void TreeModel::markItemsForCut(const QModelIndexList & indexes, bool on)
{
  foreach (const QModelIndex &index, indexes)
    markItemForCut(index, on);
}

// onRowsAboutToBeRemoved could be a way to deal with models being drag-drop moved to another window/file.
// TreeModel detects these as removals and runs removeRows(), which deletes the Model indexes
//   but not the actual models from the RadioData::models array.
// BUT this also runs when moving rows within our own tree, and if there is an error during the move,
//   or the user cancels the operation, removeRows() is still called automatically somewhere inside QAbstractItemModel().
// If a solution could be found to this problem then we could enable DnD-moving models between file windows.
/*
void TreeModel::onRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last)
{
  qDebug() << parent << first << last;
  QVector<int> modelIndices;
  for (int i=first; i <= last; ++i) {
    modelIndices << getItem(index(i, 0, parent))->getModelIndex();
  }
  if (modelIndices.size())
    emit modelsRemoved(modelIndices);
}
*/

void TreeModel::onRowsRemoved(const QModelIndex & parent, int first, int last)
{
  // This is a workaround to deal with models being DnD moved to another window/file or if user cancels a DnD move within our own.
  //  TreeModel detects these as removals and runs removeRows(), which deletes the Model indexes but not our actual models. See notes above.
  //qDebug() << parent << first << last;
  emit refreshRequested();  // request refresh from View because it may have it's own ideas
}

void TreeModel::refresh()
{
  EEPROMInterface * eepromInterface = getCurrentEEpromInterface();
  Board::Type board = eepromInterface->getBoard();
  TreeItem * defaultCategoryItem = NULL;
  bool hasEepromSizeData = false; //  not necessary with move to YAML storage for all boards TODO: remove unused code

  if (hasEepromSizeData) {
    availableEEpromSize = Boards::getEEpromSize(board) - 64; // let's consider fat
    availableEEpromSize -= 16 * ((eepromInterface->getSize(radioData->generalSettings) + 14) / 15);
  }

  this->blockSignals(true);  // make sure onRowsRemoved is not triggered
  removeRows(0, rowCount());
  this->blockSignals(false);

  if (hasCategories) {
    for (unsigned i = 0; i < radioData->categories.size(); i++) {
      TreeItem * current = rootItem->appendChild(i, -1);
      current->setData(0, QString(radioData->categories[i].name));
    }
  }

  for (unsigned i=0; i<radioData->models.size(); i++) {
    ModelData & model = radioData->models[i];
    int currentColumn = 0;
    TreeItem * current = NULL;

    model.modelIndex = i;

    if (hasCategories) {
      if (!model.isEmpty()) {
        TreeItem * categoryItem;
        // TODO category should be set to -1 if not Horus
        if (model.category >= 0 && model.category < rootItem->childCount()) {
          categoryItem = rootItem->child(model.category);
        }
        else {
          model.category = 0;
          if (!defaultCategoryItem) {
            defaultCategoryItem = rootItem->appendChild(0, -1);
            /*: Translators do NOT use accent for this, this is the default category name on Horus. */
            defaultCategoryItem->setData(0, tr("Models"));
          }
          categoryItem = defaultCategoryItem;
        }
        current = categoryItem->appendChild(model.category, i);
      }
    }
    else {
      current = rootItem->appendChild(0, i);
      current->setData(currentColumn++, QString().sprintf("%02d", i + 1));
    }

    if (!model.isEmpty() && current) {
      QString modelName;
      if (strlen(model.name) > 0) {
        modelName = model.name;
      }
      else {
        /*: Translators: do NOT use accents here, this is a default model name. */
        modelName = tr("Model %1").arg(uint(i+1), 2, 10, QChar('0'));
      }
      current->setData(currentColumn++, modelName);
      if (hasEepromSizeData) {
        int size = eepromInterface->getSize(model);
        current->setData(currentColumn++, QString().sprintf("%5d", size));
        size = 16 * ((size + 14) / 15);
        availableEEpromSize -= size;
        if (i == radioData->generalSettings.currModelIndex) {
          // Because we need this space for a TEMP model each time we have to write it again
          availableEEpromSize -= size;
        }
      }
      int protocol;
      QString rxs;
      unsigned moduleIdx = 0;
      for (auto const& moduleData: model.moduleData) {
        protocol = moduleData.protocol;
        // These are the only RXs that allow nominating RX # but changing RX or copying models can leave residual configuration which can cause issues
        // if (protocol == PULSES_PXX_XJT_X16 || protocol == PULSES_PXX_XJT_LR12 || protocol == PULSES_PXX_R9M || protocol == PULSES_DSMX || protocol == PULSES_MULTIMODULE) {
        if (moduleData.supportRxNum() && moduleData.modelId > 0) {
          if (!rxs.isEmpty()) {
            rxs.append(", ");
          }
          unsigned mdlidx = moduleData.modelId;
          rxs.append(QString("%1").arg(uint(mdlidx), 2, 10, QChar('0')));
          if (!isModelIdUnique(mdlidx, moduleIdx, protocol)) {
            current->setHighlightRX(true);
          }
        }
        moduleIdx++;
      }
      current->setData(currentColumn++, rxs);
    }
  }

  if (hasEepromSizeData) {
    availableEEpromSize = (availableEEpromSize / 16) * 15;
  }
}

bool TreeModel::isModelIdUnique(unsigned modelIdx, unsigned module, unsigned protocol)
{
  int cnt = 0;
  if (protocol== PULSES_PXX_XJT_D8)
    return true;

  for (auto const& model: radioData->models) {
    if (!model.isEmpty()) {
      const ModuleData& moduleData = model.moduleData[module];
      if (moduleData.protocol == protocol && moduleData.modelId == modelIdx) {
        if (++cnt > 1) {
          return false;
        }
      }
    }
  }
  return true;
}
