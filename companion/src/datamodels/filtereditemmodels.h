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

#ifndef FILTEREDITEMMODELS_H
#define FILTEREDITEMMODELS_H

#include "compounditemmodels.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>

class FilteredItemModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    enum DataFilters {
      AllFilter = AbstractItemModel::IMDG_Negative | AbstractItemModel::IMDG_None | AbstractItemModel::IMDG_Positive,
      AllExcludeNoneFilter = AllFilter &~ AbstractItemModel::IMDG_None,
      NegativeFilter = AbstractItemModel::IMDG_Negative | AbstractItemModel::IMDG_None,
      NegativeExcludeNoneFilter = AbstractItemModel::IMDG_Negative,
      PositiveFilter = AbstractItemModel::IMDG_Positive | AbstractItemModel::IMDG_None,
      PositiveExcludeNoneFilter = AbstractItemModel::IMDG_Positive
    };
    Q_ENUM(DataFilters)

    explicit FilteredItemModel(AbstractItemModel * sourceModel, int flags);
    explicit FilteredItemModel(AbstractItemModel * sourceModel) :
      FilteredItemModel(sourceModel, 0) {}
    virtual ~FilteredItemModel() {};

    void setId(int id) { m_id = id; }
    int getId() const { return m_id; };
    void setName(QString name) { m_name = name; }
    QString getName() const { return m_name; }

    static void dumpItemModelContents(FilteredItemModel * itemModel);

  public slots:
    void setFilterFlags(int flags);
    void update() const;
    void onAboutToBeUpdated();
    void onUpdateComplete();

  signals:
    void aboutToBeUpdated();
    void updateComplete();

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

    int filterFlags;

  private:
    int m_id = 0;
    QString m_name = "";
};

class FilteredItemModelFactory
{
  public:
    explicit FilteredItemModelFactory();
    virtual ~FilteredItemModelFactory();

    int registerItemModel(FilteredItemModel * itemModel, const QString name, const int id = -1);
    void unregisterItemModels();
    void unregisterItemModel(const int id);
    bool isItemModelRegistered(const int id) const;
    bool isItemModelRegistered(const QString name) const;
    FilteredItemModel * getItemModel(const int id) const;
    FilteredItemModel * getItemModel(const QString name) const;
    void update();

    void dumpItemModelContents(const int id) const;
    void dumpItemModelContents(const QString name) const;
    void dumpAllItemModelContents() const;

  protected:
    QVector<FilteredItemModel *> registeredItemModels;
};

class CurveRefFilteredFactory : public FilteredItemModelFactory
{
  public:
    enum FilteredItemModelId {
      CRFIM_CURVE,
      CRFIM_GVARREF,
      CRFIM_TYPE,
      CRFIM_FUNC
    };

    explicit CurveRefFilteredFactory(CompoundItemModelFactory * sharedItemModels, const int curveFlags = 0, const int gvarRefFlags = 0);
    virtual ~CurveRefFilteredFactory();

    static QString fidToString(const int value);
};

#endif // FILTEREDITEMMODELS_H
