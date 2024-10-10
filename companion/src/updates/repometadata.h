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

#include "updatestatus.h"
#include "updatenetwork.h"
#include "repodatamodels.h"

#include <QtCore>

class QAbstractItemModel;

class RepoMetaData
{
  public:
    explicit RepoMetaData(UpdateStatus * status, UpdateNetwork * network);
    virtual ~RepoMetaData() {}

    virtual bool retrieveMetaDataAll() = 0;
    virtual bool retrieveMetaDataOne(const int id) = 0;
    virtual int setId(const int id);

    const int count() const;
    const QString date() const;
    void dumpRawItemModel() const;
    void dumpFilteredItemModel() const;
    const int flags() const;
    int getSetId();
    int getSetId(const int row);
    int getSetId(QVariant value, Qt::MatchFlags flags = Qt::MatchExactly, int role = Qt::DisplayRole);
    const int id() const;
    void invalidate();
    const int isEmpty() const;
    const bool isIdValid() const;
    const QStringList list() const;
    const QString name() const;
    UpdateNetwork* const network() const;
    bool retrieveMetaData(const int mdt, const QString url);
    bool setFlags(int flags);
    void setFilterFlags(int flags);
    void setFilterPattern(const QString & pattern);
    void setModels(RepoRawItemModel * repoRawItemModel, RepoFilteredItemModel * repoFilteredItemModel);
    UpdateStatus* const status() const;

  private:
    UpdateStatus* const m_status;
    UpdateNetwork* const m_network;
    RepoRawItemModel* m_rawItemModel;
    RepoFilteredItemModel* m_filteredItemModel;

    int m_id;

    virtual void dumpItemModel(const QString modelName, const QAbstractItemModel * itemModel) const = 0;
};
