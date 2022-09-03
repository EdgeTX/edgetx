#include "labels.h"

LabelsModel::LabelsModel(QItemSelectionModel *selectionModel,
                         RadioData *radioData, QObject *parent) :
  QAbstractItemModel(parent),
  modelsSelection(selectionModel),
  radioData(radioData),
  selectedModel(-1)
{
  connect(modelsSelection, &QItemSelectionModel::currentChanged,
          this, &LabelsModel::modelsSelectionChanged );

  buildLabelsList();
}

LabelsModel::~LabelsModel()
{

}

Qt::ItemFlags LabelsModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;

  if(index.column() == 0) {
    flags |= Qt::ItemIsUserCheckable;
    flags |= Qt::ItemIsSelectable;
    flags |= Qt::ItemIsDropEnabled;
    flags |= Qt::ItemIsEditable;
    if(index.isValid())
      flags |= Qt::ItemIsDragEnabled;
  }
  return flags;
}

bool LabelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid() || index.column() != 0 ||
     index.row() >= radioData->labels.size())
    return false;

  if(role == Qt::CheckStateRole) {
    if(value==Qt::Unchecked && selectedModel != -1) {
      if(radioData->removeLabelFromModel(selectedModel, radioData->labels.at(index.row())))
        emit modelChanged(selectedModel);
    } else {
      if(radioData->addLabelToModel(selectedModel, radioData->labels.at(index.row())))
        emit modelChanged(selectedModel);
    }
    emit dataChanged(this->index(index.row(), 0),
                     this->index(index.row(), 0));
    return true;
  } else if(role == Qt::EditRole) {
    QString replFrom = labels[index.row()].label;
    QString replTo = value.toString().replace(',','_'); // Don't allow comma, replace with _
    if(radioData->labels.indexOf(replTo) == -1) { // Don't allow duplicates
      bool modelsChanged = radioData->renameLabel(replFrom,replTo);
      labels[index.row()].label = replTo;
      emit dataChanged(this->index(index.row(), 0),
                       this->index(index.row(), 0));
      if(selectedModel != -1 && modelsChanged)
        emit modelChanged(selectedModel);
    }
    return true;
  }
  return false;
}

QVariant LabelsModel::data(const QModelIndex &index, int role) const
{
  if(index.row() >= labels.size() || !index.isValid() )
    return QVariant();

   QString label = radioData->labels.at(index.row());

  if(role == Qt::DisplayRole || role == Qt::EditRole) {
    if(index.column() == 0) {
      return labels.at(index.row()).label;
        //return label;
    }
  } else if (role == Qt::CheckStateRole) {
    if(index.column() == 0 && selectedModel >= 0 &&
      selectedModel < (int)radioData->models.size()) {
      QStringList modelLabels = QString(radioData->models.at(selectedModel).labels).split(',',QString::SkipEmptyParts);
      return modelLabels.indexOf(label)==-1?Qt::Unchecked:Qt::Checked;
    }
  }
  return QVariant();
}

QModelIndex LabelsModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(parent)

  if(row < labels.size())
    return createIndex(row,0);

  return QModelIndex();
}

QModelIndex LabelsModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int LabelsModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);  
  return labels.size();
}

int LabelsModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant LabelsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role != Qt::DisplayRole ||
     orientation != Qt::Horizontal ||
     section != 0)
    return QVariant();
  return tr("Labels");
}

Qt::DropActions LabelsModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QStringList LabelsModel::mimeTypes() const
{
  QStringList types;
  types << "application/edglbl.text.list";
  return types;
}

QMimeData *LabelsModel::mimeData(const QModelIndexList &indexes) const
{
  QMimeData *mimeData = new QMimeData;
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  for (const QModelIndex &index : indexes) {
      if (index.isValid()) {
          QString text = data(index, Qt::DisplayRole).toString();
          stream << text;
      }
  }

  mimeData->setData("application/edglbl.text.list", encodedData);
  return mimeData;
}

bool LabelsModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(parent);

  if (!data->hasFormat("application/edglbl.text.list"))
      return false;

  if (column > 0)
      return false;

  return true;
}

bool LabelsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  // TODO: This needs to be completed
  if (!canDropMimeData(data, action, row, column, parent))
    return false;

  if(row < 0) return false;

  if (action == Qt::IgnoreAction)
    return true;
  else if (action  != Qt::MoveAction)
     return false;

  QByteArray encodedData = data->data("application/edglbl.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList newItems;
  int rows = 0;

  while (!stream.atEnd()) {
    QString text;
    stream >> text;
    newItems << text;
    ++rows;
  }

  /*insertRows(row, rows, QModelIndex());
  for (const QString &text : qAsConst(newItems))
  {
      QModelIndex idx = index(row, 0, QModelIndex());
      row++;
  }*/

  return true;
}

bool LabelsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (parent.isValid())
    return false;

  // Find a New Number
  for (int i = 0; i < count; i++) {
    int newno=0;
    QString newstr;
    do {
      newstr = QString(tr("New%1").arg(newno));
      if(newno == 0)
        newstr = QString(tr("New"));
      newno++;
    } while(radioData->labels.indexOf(newstr) >= 0);
    // Add it to radioData
    radioData->labels.insert(row+i, newstr);
  }
  buildLabelsList();
  return true;
}

bool LabelsModel::removeRows(int row, int count, const QModelIndex &parent)
{
  if (parent.isValid() || row < 0)
       return false;
  bool deleted=false;
  beginRemoveRows(parent, row, row + count - 1);
  for (int i = 0; i != count; ++i)
   if(radioData->deleteLabel(row+i)) {
     deleted = true;
   }
  endRemoveRows();
  // Refresh all
  if(deleted)
    emit modelChanged(-1);
  buildLabelsList();
return true;
}

void LabelsModel::buildLabelsList()
{
  labels.clear();

  int i=0;
  foreach(QString lbl, radioData->labels) {
    LabelItem itm;
    itm.label = lbl;
    itm.radioLabelIndex = i++;
    labels.append(itm);
  }
  if (i) {
    emit dataChanged(index(0,0),
                     index(i-1,0));
  }
}

void LabelsModel::modelsSelectionChanged()
{
  QModelIndex index = modelsSelection->currentIndex();
  if (index.isValid()) {
      int mi = modelsSelection->currentIndex().row();
      if(mi < (int)radioData->models.size()) {
        selectedModel = mi;
        buildLabelsList();
    }
  }
  else
    selectedModel = -1;
}
