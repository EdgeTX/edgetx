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

#include <QtCore>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QJsonDocument>

class RepoItemModelBase
{
  public:
    explicit RepoItemModelBase(QString name) : m_modelName(name) {}
    virtual ~RepoItemModelBase() {}

    inline const QString modelName() const { return m_modelName; }

    enum RepoItemModelRole {
      RIMR_Id = Qt::UserRole,
      RIMR_Date,
      RIMR_SortOrder,
      RIMR_Tag,
      RIMR_Content,
      RIMR_Available,
      RIMR_Flags,
      RIMR_CopyFilter,
      RIMR_SubDirectory,
      RIMR_DownloadUrl,
      RIMR_DownloadName,
    };

  private:
    QString m_modelName;
};

class RepoRawItemModel : public QStandardItemModel, public RepoItemModelBase
{
  Q_OBJECT

  public:
    explicit RepoRawItemModel(QString name);
    virtual ~RepoRawItemModel() {}

    enum MetaDataType {
      MDT_Unknown,
      MDT_GitHub_Asset,
      MDT_GitHub_First = MDT_GitHub_Asset,
      MDT_GitHub_Assets,
      MDT_GitHub_Release,
      MDT_GitHub_Releases,
      MDT_GitHub_Last = MDT_GitHub_Releases,
      MDT_Build_Asset,
      MDT_Build_First = MDT_Build_Asset,
      MDT_Build_Assets,
      MDT_Build_Release,
      MDT_Build_Releases,
      MDT_Build_Last = MDT_Build_Releases,
    };

    static QString typeToString(const int val);

  protected:
    friend class RepoMetaData;
    friend class RepoAssets;
    friend class RepoReleases;

    virtual bool isAvailable(QStandardItem * item) { return true; }
    virtual void parseJsonObject(const QJsonObject & obj) = 0;
    virtual void setDynamicItemData(QStandardItem * item) {}

    void invalidate();
    const bool isRefreshRequired() const { return m_refreshRequired; }
    void parseAll(QJsonDocument * json);
    void parseMetaData(const int mdt, QJsonDocument * json);
    void parseOne(QJsonDocument * json);
    void setRefreshRequired(const bool val) { m_refreshRequired = val; }
    bool setValue(const int id, const int role, const QVariant value);
    void update();

  private:
    bool m_refreshRequired;
};

class RepoFilteredItemModel: public QSortFilterProxyModel, public RepoItemModelBase
{
    Q_OBJECT

  public:
    explicit RepoFilteredItemModel(QString name);
    virtual ~RepoFilteredItemModel() {}

  protected:
    friend class RepoMetaData;
    friend class RepoAssets;
    friend class RepoReleases;

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

    const int count() const { return rowCount(); }
    const QString date(const int id) const;
    const int getFlags(const int id) const;
    const int id(const int row) const ;
    const int id(const QVariant & value, const Qt::MatchFlags flags = Qt::MatchExactly, const int role = Qt::DisplayRole) const;
    const bool isEmpty() const { return rowCount() == 0; }
    const QStringList list() const;
    const QString name(const int id) const;
    void setFilterFlags(const int flags);
    void setFilterPattern(const QString & pattern);
    bool setFlags(const int id, const int flags);
    bool setValue(const int id, const int role, const QVariant value);
    const QVariant value(const int id, const int role) const;
    const QMap<int, QVariant> valuesMap(int id) const;

  private:
    int m_filterFlags;
    QRegularExpression m_filterText;
};
