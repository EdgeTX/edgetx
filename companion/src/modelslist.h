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

#include "eeprominterface.h"
#include "labels.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QUuid>

class ModelListItem
{
  public:
    enum ModelListItemFlags { MarkedForCut = 0x01 };

    explicit ModelListItem(const QVector<QVariant> & itemData);
    explicit ModelListItem(ModelListItem * parent, int modelIndex);
    ~ModelListItem();

    ModelListItem * child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    ModelListItem * appendChild( int modelIndex);
    ModelListItem * insertChild(const int row, int modelIndex);
    bool removeChildren(int position, int count);
    bool insertChildren(int row, int count);

    int childNumber() const;
    bool setData(int column, const QVariant &value);

    ModelListItem * parent() { return parentItem; }
    void setParent(ModelListItem * p) { parentItem = p; }
    int getModelIndex() const { return modelIndex; }
    void setModelIndex(int value) { modelIndex = value; }
    void setHighlightRX(int value) { highlightRX = value; }
    bool isHighlightRX() const { return highlightRX; }

    quint16 getFlags() const { return flags; }
    void setFlags(const quint16 & value) { flags = value; }
    void setFlag(const quint16 & flag, const bool on = true);

    bool isModel() const;

  private:
    QList<ModelListItem*> childItems;
    QVector<QVariant> itemData;
    ModelListItem * parentItem;
    int modelIndex;
    quint16 flags;
    bool highlightRX;
};


class ModelsListModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    struct MimeHeaderData {
      QUuid instanceId;
      quint16 dataVersion;
    };

    ModelsListModel(RadioData * radioData, QObject *parent = nullptr);
    virtual ~ModelsListModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    //virtual bool insertRows(int row, int count, const QModelIndex & parent) override;

    virtual QStringList mimeTypes() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const override;
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const override;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    void encodeModelsData(const QModelIndexList & indexes, QByteArray * data) const;
    void encodeGeneralData(QByteArray * data) const;
    void encodeHeaderData(QByteArray * data) const;
    void encodeFileData(QByteArray * data) const;
    QMimeData * getModelsMimeData(const QModelIndexList & indexes, QMimeData * mimeData = nullptr) const;
    QMimeData * getGeneralMimeData(QMimeData * mimeData = nullptr) const;
    QMimeData * getHeaderMimeData(QMimeData * mimeData = nullptr) const;
    QMimeData * getFileMimeData(QMimeData * mimeData = nullptr) const;
    QUuid getMimeDataSourceId(const QMimeData * mimeData) const;
    bool hasSupportedMimeData(const QMimeData * mimeData) const;
    bool hasModelsMimeData(const QMimeData * mimeData) const;
    bool hasGeneralMimeData(const QMimeData * mimeData) const;
    bool hasHeaderMimeData(const QMimeData * mimeData) const;
    bool hasOwnMimeData(const QMimeData * mimeData) const;
    bool hasFileMimeData(const QMimeData * mimeData) const;

    static bool decodeFileData(const QMimeData * mimeData, QString * filedata);
    static bool decodeHeaderData(const QMimeData * mimeData, MimeHeaderData * header);
    static bool decodeMimeData(const QMimeData * mimeData, QVector<ModelData> * models = nullptr, GeneralSettings * gs = nullptr, bool * hasGenSet = nullptr);
    static int countModelsInMimeData(const QMimeData * mimeData);

    QModelIndex getIndexForModel(const int modelIndex, QModelIndex parent = QModelIndex());
    int getModelIndex(const QModelIndex & index) const;
    int rowNumber(const QModelIndex & index = QModelIndex()) const;
    bool isModelType(const QModelIndex & index) const;
    void setFilename(QString & name);

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
    ModelListItem * getItem(const QModelIndex & index) const;
    bool isModelIdUnique(unsigned modelId, unsigned module, unsigned protocol);

    ModelListItem * rootItem;
    RadioData * radioData;
    MimeHeaderData mimeHeaderData;
    bool hasLabels;
    QString filename;
};

class ModelsListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    ModelsListProxyModel() {}
    virtual ~ModelsListProxyModel() {}

  public slots:
    void setFilter(LabelsModel* labels) { m_labels = labels; }

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

  private:
    LabelsModel* m_labels;
};
