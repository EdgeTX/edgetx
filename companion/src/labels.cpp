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
  if(index.column() == 0)
    return Qt::ItemIsUserCheckable |
           Qt::ItemIsEnabled;
  return Qt::ItemIsEnabled;
}

bool LabelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid())
    return false;
  if(index.column() == 0 && role == Qt::CheckStateRole
     && index.row() < radioData->labels.size()) {
    qDebug() << "User checked row" << index.row();
/*    DataItem *itm = datalist.at(index.row());
    bool checked = value==Qt::Checked?true:false;

    // Is it an array value
    QString arrname = itm->name.mid(0,itm->name.indexOf('['));
    if(arrname.size()) {
      trkset->setDataItemSend(arrname, checked);
      checkArray(arrname, checked); // Select them all
    // Single Values
    } else {
      itm->checked = checked;
      trkset->setDataItemSend(datalist.at(index.row())->name,itm->checked);
    }*/
    return true;
  }
  return false;
}

QVariant LabelsModel::data(const QModelIndex &index, int role) const
{
  if(index.row() >= radioData->labels.size() || !index.isValid() )
    return QVariant();

   QString label = radioData->labels.at(index.row());

  if(role == Qt::DisplayRole) {
    if(index.column() == 0) {
        return label;
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
  if(row >= radioData->labels.size())
    return QModelIndex();

  if(column == 0 && row < modelIndices.size())
    return modelIndices.at(row);

  return QModelIndex();
}

QModelIndex LabelsModel::parent(const QModelIndex &index) const
{

}

int LabelsModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return radioData->labels.size();
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

void LabelsModel::buildLabelsList()
{
  modelIndices.clear();
  int i=0;
  foreach(QString lbl, radioData->labels) {
    modelIndices.append(createIndex(i++,0));
  }
  if (i) {
    emit dataChanged(modelIndices.at(0),
                     modelIndices.at(modelIndices.size()-1));
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
