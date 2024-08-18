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

#include "modelslist.h"

ModelListItem::ModelListItem(const QVector<QVariant> & itemData):
  itemData(itemData),
  parentItem(nullptr),
  modelIndex(-1),
  flags(0),
  highlightRX(false)
{
}

ModelListItem::ModelListItem(ModelListItem * parent, int modelIndex):
  ModelListItem(QVector<QVariant>(parent->columnCount()))
{
  setParent(parent);
  setModelIndex(modelIndex);
}

ModelListItem::~ModelListItem()
{
  qDeleteAll(childItems);
}

ModelListItem * ModelListItem::child(int number)
{
  return childItems.value(number);
}

int ModelListItem::childCount() const
{
  return childItems.count();
}

int ModelListItem::childNumber() const
{
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<ModelListItem*>(this));

  return 0;
}

int ModelListItem::columnCount() const
{
  return itemData.count();
}

QVariant ModelListItem::data(int column) const
{
  return itemData.value(column);
}

ModelListItem *ModelListItem::insertChild(const int row, int modelIndex)
{
  ModelListItem * item = new ModelListItem(this, modelIndex);
  childItems.insert(row, item);
  return item;
}

ModelListItem * ModelListItem::appendChild(int modelIndex)
{
  return insertChild(childItems.size(), modelIndex);
}

bool ModelListItem::removeChildren(int position, int count)
{
  if (position < 0 || position + count > childItems.size())
    return false;

  for (int row = 0; row < count; ++row)
    delete childItems.takeAt(position);

  return true;
}

bool ModelListItem::insertChildren(int row, int count)
{
  for (int i=0; i < count; ++i) {
    insertChild(row + i, -1);
  }
  return true;
}

bool ModelListItem::setData(int column, const QVariant & value)
{
  if (column < 0 || column >= itemData.size())
    return false;

  itemData[column] = value;
  return true;
}

void ModelListItem::setFlag(const quint16 & flag, const bool on)
{
  if (on)
    flags |= flag;
  else
    flags &= ~flag;
}

bool ModelListItem::isModel() const
{
  return (modelIndex > -1);
}


/*
 * ModelsListModel
*/

ModelsListModel::ModelsListModel(RadioData * radioData, QObject * parent):
  QAbstractItemModel(parent),
  radioData(radioData)
{
  hasLabels = getCurrentFirmware()->getCapability(Capability::HasModelLabels);
  QVector<QVariant> labels;
  if (!hasLabels)
    labels << tr("Index");
  labels << tr("Name");
  labels << tr("RX #");
  if (hasLabels)
    labels << tr("Labels");

  rootItem = new ModelListItem(labels);
  // uniqueId and version for drag/drop operations (see encodeHeaderData())
  mimeHeaderData.instanceId = QUuid::createUuid();
  mimeHeaderData.dataVersion = 1;

  refresh();
  //connect(this, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ModelsListModel::onRowsAboutToBeRemoved);
  connect(this, &QAbstractItemModel::rowsRemoved, this, &ModelsListModel::onRowsRemoved);
}

ModelsListModel::~ModelsListModel()
{
  delete rootItem;
}

int ModelsListModel::columnCount(const QModelIndex & /* parent */) const
{
  return rootItem->columnCount();
}

QVariant ModelsListModel::data(const QModelIndex & index, int role) const
{
  if (!index.isValid())
    return QVariant();

  ModelListItem * item = getItem(index);

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return item->data(index.column());
  }

  if (role == Qt::FontRole && item->isModel()) {
    QFont font;
    if (item->getModelIndex() == (int)radioData->generalSettings.currModelIndex)
      font.setBold(true);
    if (radioData->models[item->getModelIndex()].modelUpdated)
      font.setItalic(true);
    return font;
  }

  if (role == Qt::ForegroundRole && (item->getFlags() & ModelListItem::MarkedForCut)) {
    return QPalette().brush(QPalette::Disabled, QPalette::Text);
  }

  if (role == Qt::ForegroundRole && item->isModel()) {
    int col = item->columnCount() - 1;
    if(hasLabels)
        col --;
    if (index.column() == col && item->isHighlightRX()) {
      QBrush brush;
      brush.setColor(Qt::red);
      return brush;
    }
  }

  return QVariant();
}

Qt::ItemFlags ModelsListModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  f |= Qt::ItemIsDragEnabled;
  f |= Qt::ItemIsDropEnabled;

  //qDebug() << f;
  return f;
}

QVariant ModelsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);

  return QVariant();
}

QModelIndex ModelsListModel::index(int row, int column, const QModelIndex & parent) const
{
  if (parent.isValid() && parent.column() != 0)
    return QModelIndex();

  ModelListItem * parentItem = getItem(parent);
  ModelListItem * childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex ModelsListModel::parent(const QModelIndex & index) const
{
  return QModelIndex();
}

bool ModelsListModel::removeRows(int position, int rows, const QModelIndex & parent)
{
  ModelListItem * parentItem = getItem(parent);
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
bool ModelsListModel::insertRows(int row, int count, const QModelIndex & parent)
{
  ModelListItem * parentItem = getItem(parent);
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

int ModelsListModel::rowCount(const QModelIndex &parent) const
{
  ModelListItem * parentItem = getItem(parent);
  return parentItem->childCount();
}

bool ModelsListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if (role != Qt::EditRole)
    return false;

  if (!index.isValid())
    return false;

  ModelListItem * item = getItem(index);
  bool result = item->setData(index.column(), value);

  if (result) {
    emit dataChanged(index, index);
  }

  return result;
}

QStringList ModelsListModel::mimeTypes() const
{
  QStringList types;
  types << "application/x-companion-modeldata";
  types << "application/x-companion-generaldata";
  //type << "application/x-companion-radiodata-header";  // supported but not advertised, must be in conjunction with one of the above
  return types;
}

Qt::DropActions ModelsListModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions ModelsListModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

// This method encodes all the data on default drag operation, including general radio settings. This is useful for eg. Compare dialog/model printer.
QMimeData * ModelsListModel::mimeData(const QModelIndexList & indexes) const
{
  QMimeData * mimeData = new QMimeData();
  getModelsMimeData(indexes, mimeData);
  getGeneralMimeData(mimeData);
  getHeaderMimeData(mimeData);
  getFileMimeData(mimeData);
  return mimeData;
}

bool ModelsListModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
  Q_UNUSED(action);
  //qDebug() << action << row << column << parent.row();

  // we do not accept dropped general settings right now (user must copy/paste those)
  if (hasHeaderMimeData(data) && hasModelsMimeData(data) && (row > -1 || parent.isValid()))
    return true;

  return false;
}

bool ModelsListModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
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

QMimeData *ModelsListModel::getModelsMimeData(const QModelIndexList & indexes, QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeModelsData(indexes, &mData);
  mimeData->setData("application/x-companion-modeldata", mData);
  return mimeData;
}

QMimeData *ModelsListModel::getGeneralMimeData(QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeGeneralData(&mData);
  mimeData->setData("application/x-companion-generaldata", mData);
  return mimeData;
}

QMimeData *ModelsListModel::getHeaderMimeData(QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeHeaderData(&mData);
  mimeData->setData("application/x-companion-radiodata-header", mData);
  return mimeData;
}

QMimeData *ModelsListModel::getFileMimeData(QMimeData * mimeData) const
{
  if (!mimeData)
    mimeData = new QMimeData();
  QByteArray mData;
  encodeFileData(&mData);
  mimeData->setData("application/x-companion-filedata", mData);
  return mimeData;
}

QUuid ModelsListModel::getMimeDataSourceId(const QMimeData * mimeData) const
{
  MimeHeaderData header;
  decodeHeaderData(mimeData, &header);
  return header.instanceId;
}

bool ModelsListModel::hasSupportedMimeData(const QMimeData * mimeData) const
{
  foreach (const QString & mtype, mimeTypes()) {
    if (mimeData->hasFormat(mtype))
      return true;
  }
  return false;
}

bool ModelsListModel::hasModelsMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-modeldata");
}

bool ModelsListModel::hasGeneralMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-generaldata");
}

bool ModelsListModel::hasHeaderMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-radiodata-header");
}

bool ModelsListModel::hasFileMimeData(const QMimeData * mimeData) const
{
  return mimeData->hasFormat("application/x-companion-filedata");
}

// returns true if mime data origin was this data model (vs. from another file window)
bool ModelsListModel::hasOwnMimeData(const QMimeData * mimeData) const
{
  return (getMimeDataSourceId(mimeData) == mimeHeaderData.instanceId);
}

void ModelsListModel::encodeModelsData(const QModelIndexList & indexes, QByteArray * data) const
{
  foreach (const QModelIndex &index, indexes) {
    if (index.isValid() && index.column() == 0) {
      data->append('M');
      data->append((char *)&radioData->models[getModelIndex(index)], sizeof(ModelData));
    }
  }
}

void ModelsListModel::encodeGeneralData(QByteArray * data) const
{
  data->append('G');
  data->append((char *)&radioData->generalSettings, sizeof(GeneralSettings));
}

void ModelsListModel::encodeHeaderData(QByteArray * data) const
{
  // We use a unique ID representing this ModelsListModel instance (a unique file).
  // This can be used eg. to detect cross-file drop operations.
  QDataStream stream(data, QIODevice::WriteOnly);
  stream << mimeHeaderData.dataVersion;
  stream << mimeHeaderData.instanceId;
}

void ModelsListModel::encodeFileData(QByteArray * data) const
{
  *data = filename.toLatin1();
}

// static
bool ModelsListModel::decodeHeaderData(const QMimeData * mimeData, MimeHeaderData * header)
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
bool ModelsListModel::decodeFileData(const QMimeData * mimeData, QString * filedata)
{
  if (filedata && mimeData->hasFormat("application/x-companion-filedata")) {
    *filedata = mimeData->data("application/x-companion-filedata").data();
    return true;
  }
  return false;
}

// static
bool ModelsListModel::decodeMimeData(const QMimeData * mimeData, QVector<ModelData> * models, GeneralSettings * gs, bool * hasGenSet)
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
int ModelsListModel::countModelsInMimeData(const QMimeData * mimeData)
{
  int ret = 0;
  if (mimeData->hasFormat("application/x-companion-modeldata")) {
    QByteArray mdlData = mimeData->data("application/x-companion-modeldata");
    ret = mdlData.size() / (sizeof(ModelData) + 1);
  }
  return ret;
}


ModelListItem * ModelsListModel::getItem(const QModelIndex & index) const
{
  if (index.isValid()) {
    ModelListItem * item = static_cast<ModelListItem *>(index.internalPointer());
    if (item) {
      return item;
    }
  }
  return rootItem;
}

// recursive
QModelIndex ModelsListModel::getIndexForModel(const int modelIndex, QModelIndex parent)
{
  for (int i = 0; i < rowCount(parent); ++i) {
    QModelIndex idx = index(i, 0, parent);
    if (hasChildren(idx) && (idx = getIndexForModel(modelIndex, idx)).isValid())
      return idx;
    if (getItem(idx)->getModelIndex() == modelIndex)
      return idx;
  }
  return QModelIndex();
}

int ModelsListModel::getModelIndex(const QModelIndex & index) const
{
  return getItem(index)->getModelIndex();
}

int ModelsListModel::rowNumber(const QModelIndex & index) const
{
  return getItem(index)->childNumber();
}

bool ModelsListModel::isModelType(const QModelIndex & index) const
{
  return index.isValid() && getItem(index)->isModel();
}

void ModelsListModel::markItemForCut(const QModelIndex & index, bool on)
{
  if (index.isValid() && index.column() == 0)
    getItem(index)->setFlag(ModelListItem::MarkedForCut, on);
}

void ModelsListModel::markItemsForCut(const QModelIndexList & indexes, bool on)
{
  foreach (const QModelIndex &index, indexes)
    markItemForCut(index, on);
}

// onRowsAboutToBeRemoved could be a way to deal with models being drag-drop moved to another window/file.
// ModelsListModel detects these as removals and runs removeRows(), which deletes the Model indexes
//   but not the actual models from the RadioData::models array.
// BUT this also runs when moving rows within our own tree, and if there is an error during the move,
//   or the user cancels the operation, removeRows() is still called automatically somewhere inside QAbstractItemModel().
// If a solution could be found to this problem then we could enable DnD-moving models between file windows.
/*
void ModelsListModel::onRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last)
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

void ModelsListModel::onRowsRemoved(const QModelIndex & parent, int first, int last)
{
  // This is a workaround to deal with models being DnD moved to another window/file or if user cancels a DnD move within our own.
  //  ModelsListModel detects these as removals and runs removeRows(), which deletes the Model indexes but not our actual models. See notes above.
  //qDebug() << parent << first << last;
  emit refreshRequested();  // request refresh from View because it may have it's own ideas
}

void ModelsListModel::refresh()
{
  this->blockSignals(true);  // make sure onRowsRemoved is not triggered
  removeRows(0, rowCount());
  this->blockSignals(false);

  for (unsigned i = 0; i < radioData->models.size(); i++) {
    ModelData & model = radioData->models[i];
    int currentColumn = 0;
    ModelListItem * current = nullptr;

    model.modelIndex = i;

    if (hasLabels) {
      current = rootItem->appendChild(i);
    }
    else {
      current = rootItem->appendChild(i);
      current->setData(currentColumn++, QString("%1").arg(i + 1, 2, 10, QChar('0')));
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
   if (hasLabels) {
      QStringList labels = RadioData::fromCSV(QString::fromUtf8(model.labels));
     current->setData(currentColumn++, labels.join(QChar(0x2022)));
   }
  }
}

bool ModelsListModel::isModelIdUnique(unsigned modelIdx, unsigned module, unsigned protocol)
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

void ModelsListModel::setFilename(QString & name)
{
  filename = name;
}

/*
  ModelsListProxyModel
*/

bool ModelsListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  return true;
}

