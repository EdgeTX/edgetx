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

#include "modeledit.h"
#include "eeprominterface.h"
#include "filtereditemmodels.h"

class AutoLineEdit;

namespace Ui {
  class FunctionSwitches;
}

class FilteredGroupSwitchesModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit FilteredGroupSwitchesModel(AbstractItemModel * sourceModel, int group, ModelData * model, int switchcnt) :
      QSortFilterProxyModel(nullptr),
      m_group(group),
      m_switchcnt(switchcnt),
      m_model(model)
    {
      setFilterKeyColumn(0);
      setDynamicSortFilter(true);
      setSourceModel(sourceModel);
    }

    explicit FilteredGroupSwitchesModel(AbstractItemModel * sourceModel) :
      FilteredGroupSwitchesModel(sourceModel, 0, nullptr, 0) {}
    virtual ~FilteredGroupSwitchesModel() {};

    void setGroup(int group) { m_group = group; }
    int getGroup() const { return m_group; }
    void setSwitchcnt(int n) { m_switchcnt = n; }
    int getSwitchcnt() const { return m_switchcnt; }
    void setModel(ModelData* model) { m_model = model; }
    ModelData* getModel() const { return m_model; }

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

  private:
    int m_group = 0;
    int m_switchcnt = 0;
    ModelData* m_model = nullptr;
};

class FilteredSwitchGroupsModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit FilteredSwitchGroupsModel(AbstractItemModel * sourceModel, int sw, ModelData * model) :
      QSortFilterProxyModel(nullptr),
      m_switch(sw),
      m_model(model)
    {
      setFilterKeyColumn(0);
      setDynamicSortFilter(true);
      setSourceModel(sourceModel);
    }

    explicit FilteredSwitchGroupsModel(AbstractItemModel * sourceModel) :
      FilteredSwitchGroupsModel(sourceModel, 0, nullptr) {}
    virtual ~FilteredSwitchGroupsModel() {};

    void setSwitch(int group) { m_switch = group; }
    int getSwitch() const { return m_switch; }
    void setModel(ModelData* model) { m_model = model; }
    ModelData* getModel() const { return m_model; }

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

  private:
    int m_switch = 0;
    ModelData* m_model = nullptr;
};

class FilteredSwitchConfigsModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit FilteredSwitchConfigsModel(AbstractItemModel * sourceModel, int sw, ModelData * model) :
      QSortFilterProxyModel(nullptr),
      m_switch(sw),
      m_model(model)
    {
      setFilterKeyColumn(0);
      setDynamicSortFilter(true);
      setSourceModel(sourceModel);
    }

    explicit FilteredSwitchConfigsModel(AbstractItemModel * sourceModel) :
      FilteredSwitchConfigsModel(sourceModel, 0, nullptr) {}
    virtual ~FilteredSwitchConfigsModel() {};

    void setSwitch(int group) { m_switch = group; }
    int getSwitch() const { return m_switch; }
    void setModel(ModelData* model) { m_model = model; }
    ModelData* getModel() const { return m_model; }

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

  private:
    int m_switch = 0;
    ModelData* m_model = nullptr;
};

class FunctionSwitchesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    FunctionSwitchesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~FunctionSwitchesPanel();

    void update();

  signals:
    void updateDataModels();

  private slots:
    void on_nameEditingFinished();
    void on_configCurrentIndexChanged(int index);
    void on_startPosnCurrentIndexChanged(int index);
    void on_groupStartPosnCurrentIndexChanged(int index);
    void on_groupChanged(int value);
    void on_alwaysOnGroupChanged(int value);

  private:
    Ui::FunctionSwitches * ui;
    QVector<AutoLineEdit *> aleNames;
    QVector<QComboBox *> cboConfigs;
    QVector<QComboBox *> cboStartupPosns;
    QVector<QComboBox *> cboGroups;
    QVector<QCheckBox *> cbAlwaysOnGroups;
    QVector<QComboBox *> cboGroupStartupPosns;
    QVector<FilteredGroupSwitchesModel *> filterGroupSwitches;
    QVector<FilteredSwitchGroupsModel *> filterSwitchGroups;
    QVector<FilteredSwitchConfigsModel *> filterSwitchConfigs;
    int switchcnt;
    AbstractStaticItemModel *fsConfig;
    AbstractStaticItemModel *fsGroupStart;
};
