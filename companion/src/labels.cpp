#include "labels.h"

LabelsModel::LabelsModel(QItemSelectionModel *selectionModel,
                         RadioData *radioData, QObject *parent) :
  modelsSelection(selectionModel),
  radioData(radioData),
  QAbstractItemModel(parent),
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
     index.row() >= radioData->labels.size() ||
     selectedModel == -1)
    return false;

  if(role == Qt::CheckStateRole) {
    if(value==Qt::Unchecked) {
      if(radioData->removeLabelFromModel(selectedModel, radioData->labels.at(index.row())))
        emit modelChanged(selectedModel);
    } else {
      if(radioData->addLabelToModel(selectedModel, radioData->labels.at(index.row())))
        emit modelChanged(selectedModel);
    }
    emit dataChanged(labels.at(index.row()).index,
                     labels.at(index.row()).index);
    return true;
  } else if(role == Qt::EditRole) {
    radioData->renameLabel(index.row(),value.toString());
    return true;
  }
  return false;
}

QVariant LabelsModel::data(const QModelIndex &index, int role) const
{ 
 // if(index.row() >= radioData->labels.size() || !index.isValid() )
  if(index.row() >= labels.size() || !index.isValid() )
    return QVariant();

   QString label = radioData->labels.at(index.row());

  if(role == Qt::DisplayRole) {
    if(index.column() == 0) {
      return labels.at(index.row()).label;
        //return label;
    }
  } else if (role == Qt::CheckStateRole) {
      if(index.column() == 0 && selectedModel >= 0 &&
         selectedModel < (int)radioData->models.size()) {
        return QString(radioData->models.at(selectedModel).labels).contains(label)?Qt::Checked:Qt::Unchecked;
      }
    }
  return QVariant();
}

QModelIndex LabelsModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(parent)

  if(row < labels.size())
    return labels[row].index;

  return QModelIndex();
}

QModelIndex LabelsModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int LabelsModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  //return radioData->labels.size();
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
  types << "application/vnd.text.list";
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

  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

bool LabelsModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(parent);

  if (!data->hasFormat("application/vnd.text.list"))
      return false;

  if (column > 0)
      return false;

  return true;
}

bool LabelsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (!canDropMimeData(data, action, row, column, parent))
    return false;

  if(row < 0) return false;

  if (action == Qt::IgnoreAction)
    return true;
  else if (action  != Qt::MoveAction)
     return false;

  QByteArray encodedData = data->data("application/vnd.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList newItems;
  int rows = 0;

  while (!stream.atEnd()) {
    QString text;
    stream >> text;
    newItems << text;
    ++rows;
  }

  insertRows(row, rows, QModelIndex());
  for (const QString &text : qAsConst(newItems))
  {
      QModelIndex idx = index(row, 0, QModelIndex());
      //setData(idx, text);
      row++;
  }

  return true;
}

bool LabelsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (parent.isValid())
    return false;
  for (int i = 0; i < count; i++)
    radioData->labels.insert(row+i, "");
  return true;
}

bool LabelsModel::removeRows(int row, int count, const QModelIndex &parent)
{
  if (parent.isValid())
       return false;
   beginRemoveRows(parent, row, row + count - 1);
   for (int i = 0; i != count; ++i)
     radioData->labels.removeAt(row);
   endRemoveRows();
   return true;
}

void LabelsModel::buildLabelsList()
{
  labels.clear();

  int i=0;
  foreach(QString lbl, radioData->labels) {
    LabelItem itm;
    itm.label = lbl;
    itm.index = createIndex(i,0);
    itm.radioLabelIndex = i++;
    labels.append(itm);
  }
  if (i) {
    emit dataChanged(labels.at(0).index,
                     labels.at(i-1).index);
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
