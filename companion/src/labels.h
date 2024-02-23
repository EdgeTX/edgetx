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

#pragma once

#include "labelvalidator.h"

#include <QObject>
#include <QList>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

class RadioData;

typedef struct {
  QString label;
  int radioLabelIndex;
} LabelItem;

class LabelsModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
    LabelsModel(QSortFilterProxyModel * modelsListProxyModel, QItemSelectionModel *selectionModel, RadioData *radioData, QObject *parent = nullptr);
    virtual ~LabelsModel();

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

  public slots:
    void buildLabelsList();

  private slots:
    void modelsSelectionChanged();

  signals:
    void modelChanged(int index);
    void labelsFault(QString msg);

  private:
    QSortFilterProxyModel * modelsListProxyModel;
    QItemSelectionModel *modelsSelection;
    RadioData *radioData;
    int selectedModel;
    QList<QModelIndex> modelIndices;
    QList<LabelItem> labels;

    QModelIndex getDataIndex(QModelIndex viewIndex) const;
};

class LabelEditTextDelegate : public QStyledItemDelegate
{
  Q_OBJECT

  public:
    LabelEditTextDelegate(QObject * parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};
