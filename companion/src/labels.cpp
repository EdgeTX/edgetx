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

#include "labels.h"
#include "radiodata.h"

LabelsModel::LabelsModel(QSortFilterProxyModel * modelsListProxyModel,
                         QItemSelectionModel *selectionModel,
                         RadioData *radioData, QObject *parent) :
  QAbstractItemModel(parent),
  modelsListProxyModel(modelsListProxyModel),
  modelsSelection(selectionModel),
  radioData(radioData),
  selectedModel(-1)
{
  connect(modelsSelection, &QItemSelectionModel::currentChanged, this, &LabelsModel::modelsSelectionChanged );

  buildLabelsList();
}

LabelsModel::~LabelsModel()
{

}

Qt::ItemFlags LabelsModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;

  if (index.column() == 0) {
    flags |= Qt::ItemIsUserCheckable;
    flags |= Qt::ItemIsSelectable;
    flags |= Qt::ItemIsDropEnabled;
    flags |= Qt::ItemIsEditable;
    if (index.isValid())
      flags |= Qt::ItemIsDragEnabled;
  }
  return flags;
}

bool LabelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || index.column() != 0 ||
     index.row() >= radioData->labels.size())
    return false;

  if (role == Qt::CheckStateRole) {
    if (value==Qt::Unchecked && selectedModel != -1) {
      if (radioData->removeLabelFromModel(selectedModel, radioData->labels.at(index.row()).name))
        emit modelChanged(selectedModel);
    } else {
      try {
      if (radioData->addLabelToModel(selectedModel, radioData->labels.at(index.row()).name))
        emit modelChanged(selectedModel);
      } catch(const std::length_error& le) {
        emit labelsFault(tr("Unable to add label \"%1\" to model \"%2\" not enough room")\
                         .arg(radioData->labels.at(index.row()).name).arg(le.what()));
      }
    }

    emit dataChanged(this->index(index.row(), 0), this->index(index.row(), 0));
    return true;

  } else if (role == Qt::EditRole) {
    QString replFrom = labels[index.row()].label;
    QString replTo = value.toString();

    if (replFrom == replTo) // User exits edit without changing
      return true;

    if (radioData->indexOfLabel(replTo) == -1) { // Don't allow duplicates
      bool modelsChanged = false;

      try {
        modelsChanged = radioData->renameLabel(replFrom, replTo);
      } catch(const std::length_error& le) {
        emit labelsFault(tr("Unable to rename \"%1\" to \"%2\" not enough room in model %3")\
                         .arg(replFrom).arg(replTo).arg(le.what()));
        return false;
      }

      labels[index.row()].label = replTo;
      emit dataChanged(this->index(index.row(), 0), this->index(index.row(), 0));

      if (selectedModel != -1 && modelsChanged)
        emit modelChanged(selectedModel);

    } else {
      emit labelsFault(tr("Unable to rename \"%1\" to \"%2\" the label already exists")\
                       .arg(replFrom).arg(replTo));
    }

    return true;
  }

  return false;
}

QVariant LabelsModel::data(const QModelIndex &index, int role) const
{
  if (index.row() >= labels.size() || !index.isValid() )
    return QVariant();

  QString label = radioData->labels.at(index.row()).name;

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == 0) {
      return labels.at(index.row()).label;
        //return label;
    }
  } else if (role == Qt::CheckStateRole) {
    if (index.column() == 0 && selectedModel >= 0 &&
      selectedModel < (int)radioData->models.size()) {
      QStringList modelLabels = QString(radioData->models.at(selectedModel).labels).split(',',Qt::SkipEmptyParts);
      label = RadioData::escapeCSV(label);
      return modelLabels.indexOf(label)==-1?Qt::Unchecked:Qt::Checked;
    } else if (index.column() == 0 && selectedModel == -1) {
      return Qt::Unchecked;
    }
  }
  return QVariant();
}

QModelIndex LabelsModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(parent)

  if (row < labels.size())
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
  if (role != Qt::DisplayRole ||
      orientation != Qt::Horizontal ||
      section != 0)
    return QVariant();

  return tr("Labels");
}

bool LabelsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (parent.isValid())
    return false;

  // Find a New Number
  for (int i = 0; i < count; i++) {
    int newNo=0;
    QString newStr;

    do {
      newStr = QString(tr("New%1").arg(newNo));
      if (newNo == 0)
        newStr = QString(tr("New"));
      newNo++;
    } while(radioData->indexOfLabel(newStr) >= 0);

    // Add it to radioData
    RadioData::LabelData ld = { newStr , false };
    radioData->labels.insert(row + i, ld);
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

  for (int i = 0; i != count; ++i) {
    if (radioData->deleteLabel(row+i)) {
      deleted = true;
    }
  }
  endRemoveRows();
  // Refresh all
  if (deleted)
    emit modelChanged(-1);

  buildLabelsList();
  return true;
}

void LabelsModel::buildLabelsList()
{
  labels.clear();

  int i = 0;

  foreach(RadioData::LabelData ld, radioData->labels) {
    LabelItem itm;
    itm.label = ld.name;
    itm.radioLabelIndex = i++;
    labels.append(itm);
  }

  if (i) {
    emit dataChanged(index(0, 0), index(i - 1, 0));
  }
}

void LabelsModel::modelsSelectionChanged()
{
  QModelIndex index = getDataIndex(modelsSelection->currentIndex());

  if (index.isValid()) {
    int mi = index.row();

    if (mi < (int)radioData->models.size()) {
      selectedModel = mi;
      buildLabelsList();
    }
  }
  else
    selectedModel = -1;
}

QModelIndex LabelsModel::getDataIndex(QModelIndex viewIndex) const
{
  return modelsListProxyModel->mapToSource(viewIndex);
}

QWidget * LabelEditTextDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
  QLineEdit *editor = new QLineEdit(parent);
  editor->setValidator(new LabelValidator(parent));
  editor->setMaxLength(LABEL_LENGTH);
  return editor;
}
