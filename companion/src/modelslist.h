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

#pragma once

#include "eeprominterface.h"
#include <QAbstractItemModel>
#include <QMimeData>
#include <QUuid>

class TreeItem
{
  public:
    enum TreeItemFlags { MarkedForCut = 0x01 };

    explicit TreeItem(const QVector<QVariant> & itemData);
    explicit TreeItem(TreeItem * parent, int categoryIndex, int modelIndex);
    ~TreeItem();

    TreeItem * child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    TreeItem * appendChild(int categoryIndex, int modelIndex);
    TreeItem * insertChild(const int row, int categoryIndex, int modelIndex);
    bool removeChildren(int position, int count);
    bool insertChildren(int row, int count);

    int childNumber() const;
    bool setData(int column, const QVariant &value);

    TreeItem * parent() { return parentItem; }
    void setParent(TreeItem * p) { parentItem = p; }
    int getModelIndex() const { return modelIndex; }
    void setModelIndex(int value) { modelIndex = value; }
    int getCategoryIndex() const { return categoryIndex; }
    void setCategoryIndex(int value) { categoryIndex = value; }
    void setHighlightRX(int value) { highlightRX = value; }
    bool isHighlightRX() const { return highlightRX; }

    quint16 getFlags() const { return flags; }
    void setFlags(const quint16 & value) { flags = value; }
    void setFlag(const quint16 & flag, const bool on = true);

    bool isCategory() const;
    bool isModel() const;

  private:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
    TreeItem * parentItem;
    int categoryIndex;
    int modelIndex;
    quint16 flags;
    bool highlightRX;
};


class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    struct MimeHeaderData {
      QUuid instanceId;
      quint16 dataVersion;
    };

    TreeModel(RadioData * radioData, QObject *parent = 0);
    virtual ~TreeModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
    //virtual bool insertRows(int row, int count, const QModelIndex & parent) Q_DECL_OVERRIDE;

    virtual QStringList mimeTypes() const Q_DECL_OVERRIDE;
    virtual Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
    virtual Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const Q_DECL_OVERRIDE;
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const Q_DECL_OVERRIDE;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;

    void encodeModelsData(const QModelIndexList & indexes, QByteArray * data) const;
    void encodeGeneralData(QByteArray * data) const;
    void encodeHeaderData(QByteArray * data) const;
    QMimeData * getModelsMimeData(const QModelIndexList & indexes, QMimeData * mimeData = NULL) const;
    QMimeData * getGeneralMimeData(QMimeData * mimeData = NULL) const;
    QMimeData * getHeaderMimeData(QMimeData * mimeData = NULL) const;
    QUuid getMimeDataSourceId(const QMimeData * mimeData) const;
    bool hasSupportedMimeData(const QMimeData * mimeData) const;
    bool hasModelsMimeData(const QMimeData * mimeData) const;
    bool hasGenralMimeData(const QMimeData * mimeData) const;
    bool hasHeaderMimeData(const QMimeData * mimeData) const;
    bool hasOwnMimeData(const QMimeData * mimeData) const;

    static bool decodeHeaderData(const QMimeData * mimeData, MimeHeaderData * header);
    static bool decodeMimeData(const QMimeData * mimeData, QVector<ModelData> * models = NULL, GeneralSettings * gs = NULL, bool * hasGenSet = NULL);
    static int countModelsInMimeData(const QMimeData * mimeData);

    QModelIndex getIndexForModel(const int modelIndex, QModelIndex parent = QModelIndex());
    QModelIndex getIndexForCategory(const int categoryIndex);
    int getModelIndex(const QModelIndex & index) const;
    int getCategoryIndex(const QModelIndex & index) const;
    int rowNumber(const QModelIndex & index = QModelIndex()) const;
    bool isCategoryType(const QModelIndex & index) const;
    bool isModelType(const QModelIndex & index) const;

  public slots:
    void markItemForCut(const QModelIndex & index, bool on = true);
    void markItemsForCut(const QModelIndexList & indexes, bool on = true);
    void refresh();

  signals:
    void modelsDropped(const QMimeData * mimeData, const QModelIndex toRowIdx, const bool insert, const bool move);
    void modelsRemoved(const QVector<int> modelIndices);
    void refreshRequested();

  private slots:
    //void onRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
    void onRowsRemoved(const QModelIndex & parent, int first, int last);

  private:
    TreeItem * getItem(const QModelIndex & index) const;
    bool isModelIdUnique(unsigned modelId, unsigned module, unsigned protocol);

    TreeItem * rootItem;
    RadioData * radioData;
    MimeHeaderData mimeHeaderData;
    bool hasCategories;
};
