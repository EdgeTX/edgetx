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

#include "setup_function_switches.h"
#include "ui_setup_function_switches.h"
#include "namevalidator.h"
#include "autolineedit.h"

#include <QDir>

bool FilteredGroupSwitchesModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  if (sourceRow == 0) return true;
  if (sourceRow == m_switchcnt + 1) {
    if (m_model->getFuncSwitchAlwaysOnGroup(m_group) || m_model->getFuncGroupSwitchCount(m_group, m_switchcnt) == 0)
      return false;
    return true;
  }
  if ((int)m_model->getFuncSwitchGroup(sourceRow - 1) == m_group)
    return true;
  return false;
}

bool FilteredSwitchGroupsModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  if (sourceRow == 0) return true;
  if (m_model->getFuncSwitchConfig(m_switch) == ModelData::FUNC_SWITCH_CONFIG_NONE) return false;
  return true;
}

bool FilteredSwitchConfigsModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  if (sourceRow == ModelData::FUNC_SWITCH_CONFIG_NONE || sourceRow == ModelData::FUNC_SWITCH_CONFIG_2POS) return true;
  return !m_model->getFuncSwitchAlwaysOnGroupForSwitch(m_switch);
}

FunctionSwitchesPanel::FunctionSwitchesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::FunctionSwitches)
{
  ui->setupUi(this);

  fsConfig = ModelData::funcSwitchConfigItemModel();
  AbstractStaticItemModel *fsStart = ModelData::funcSwitchStartItemModel();
  AbstractStaticItemModel *fsGroups = ModelData::funcSwitchGroupsModel();

  Board::Type board = firmware->getBoard();

  lock = true;

  switchcnt = Boards::getCapability(board, Board::FunctionSwitches);

  fsGroupStart = ModelData::funcSwitchGroupStartSwitchModel(switchcnt);

  for (int i = 0; i < switchcnt; i++) {
    QLabel * lblSwitchId = new QLabel(this);
    lblSwitchId->setText(tr("SW%1").arg(i + 1));

    AutoLineEdit * aleName = new AutoLineEdit(this);
    aleName->setProperty("index", i);
    aleName->setValidator(new NameValidator(board, this));
    aleName->setField((char *)model.functionSwitchNames[i], 3);

    QComboBox * cboConfig = new QComboBox(this);
    cboConfig->setProperty("index", i);
    auto configFilter = new FilteredSwitchConfigsModel(fsConfig, i, &model);
    cboConfig->setModel(configFilter);
    filterSwitchConfigs << configFilter;

    QComboBox * cboStartPosn = new QComboBox(this);
    cboStartPosn->setProperty("index", i);
    cboStartPosn->setModel(fsStart);

    QComboBox * cboGroup = new QComboBox(this);
    cboGroup->setProperty("index", i);
    auto groupFilter = new FilteredSwitchGroupsModel(fsGroups, i, &model);
    cboGroup->setModel(groupFilter);
    filterSwitchGroups << groupFilter;

    int row = 0;
    int coloffset = 1;
    ui->gridSwitches->addWidget(lblSwitchId, row++, i + coloffset);
    ui->gridSwitches->addWidget(aleName, row++, i + coloffset);
    ui->gridSwitches->addWidget(cboConfig, row++, i + coloffset);
    ui->gridSwitches->addWidget(cboStartPosn, row++, i + coloffset);
    ui->gridSwitches->addWidget(cboGroup, row++, i + coloffset);

    connect(aleName, &AutoLineEdit::currentDataChanged, this, &FunctionSwitchesPanel::on_nameEditingFinished);
    connect(cboConfig, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_configCurrentIndexChanged);
    connect(cboStartPosn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_startPosnCurrentIndexChanged);
    connect(cboGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_groupChanged);

    aleNames << aleName;
    cboConfigs << cboConfig;
    cboStartupPosns << cboStartPosn;
    cboGroups << cboGroup;
  }

  for (int i = 0; i < 3; i += 1) {
    QLabel * lblGroupId = new QLabel(this);
    lblGroupId->setText(tr("Group %1").arg(i + 1));

    QCheckBox * cbAlwaysOnGroup = new QCheckBox(this);
    cbAlwaysOnGroup->setProperty("index", i);

    QComboBox * cboStartPosn = new QComboBox(this);
    cboStartPosn->setProperty("index", i);
    auto filter = new FilteredGroupSwitchesModel(fsGroupStart, i + 1, &model, switchcnt);
    cboStartPosn->setModel(filter);
    filterGroupSwitches << filter;

    int row = 0;
    int coloffset = 1;
    ui->gridGroups->addWidget(lblGroupId, row++, i + coloffset);
    ui->gridGroups->addWidget(cbAlwaysOnGroup, row++, i + coloffset);
    ui->gridGroups->addWidget(cboStartPosn, row++, i + coloffset);

    connect(cbAlwaysOnGroup, &QCheckBox::toggled, this, &FunctionSwitchesPanel::on_alwaysOnGroupChanged);
    connect(cboStartPosn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_groupStartPosnCurrentIndexChanged);

    cbAlwaysOnGroups << cbAlwaysOnGroup;
    cboGroupStartupPosns << cboStartPosn;
  }

  update();

  lock = false;
}

FunctionSwitchesPanel::~FunctionSwitchesPanel()
{
  delete ui;
}

void FunctionSwitchesPanel::update()
{
  lock = true;

  for (int i = 0; i < switchcnt; i++) {
    filterSwitchConfigs[i]->invalidate();
    filterSwitchGroups[i]->invalidate();

    aleNames[i]->update();
    cboConfigs[i]->setCurrentIndex(filterSwitchConfigs[i]->mapFromSource(fsConfig->index(model->getFuncSwitchConfig(i), 0)).row());
    cboStartupPosns[i]->setCurrentIndex(model->getFuncSwitchStart(i));
    unsigned int grp = model->getFuncSwitchGroup(i);
    cboGroups[i]->setCurrentIndex(grp);

    cboStartupPosns[i]->setEnabled(cboConfigs[i]->currentIndex() >= ModelData::FUNC_SWITCH_CONFIG_2POS && (grp == 0));

    cboGroups[i]->setEnabled(cboConfigs[i]->currentIndex() >= ModelData::FUNC_SWITCH_CONFIG_TOGGLE);
  }

  for (int i = 0; i < 3; i += 1) {
    filterGroupSwitches[i]->invalidate();

    model->setGroupSwitchState(i + 1, switchcnt);

    cbAlwaysOnGroups[i]->setChecked(model->getFuncSwitchAlwaysOnGroup(i + 1));
    cboGroupStartupPosns[i]->setCurrentIndex(filterGroupSwitches[i]->mapFromSource(fsGroupStart->index(model->getFuncGroupSwitchStart(i + 1, switchcnt), 0)).row());
  }

  lock = false;
}

void FunctionSwitchesPanel::on_nameEditingFinished()
 {
   emit updateDataModels();
 }


void FunctionSwitchesPanel::on_configCurrentIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    unsigned int sw = sender()->property("index").toInt(&ok);
    unsigned int config = filterSwitchConfigs[sw]->mapToSource(filterSwitchConfigs[sw]->index(index, 0)).row();
    if (ok && model->getFuncSwitchConfig(sw) != config) {
      model->setFuncSwitchConfig(sw, config);
      if (config != ModelData::FUNC_SWITCH_CONFIG_2POS) {
        model->setFuncSwitchStart(sw, ModelData::FUNC_SWITCH_START_PREVIOUS);
        if ((config == ModelData::FUNC_SWITCH_CONFIG_NONE) || model->getFuncSwitchAlwaysOnGroupForSwitch(sw))
          model->setFuncSwitchGroup(sw, 0);
      }
      update();
      emit modified();
      emit updateDataModels();
     }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_startPosnCurrentIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    unsigned int sw = sender()->property("index").toInt(&ok);
    if (ok && model->getFuncSwitchStart(sw) != (unsigned int)index) {
      model->setFuncSwitchStart(sw, index);
      update();
      emit modified();
    }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_groupStartPosnCurrentIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    unsigned int grp = sender()->property("index").toInt(&ok);
    unsigned int sw = filterGroupSwitches[grp]->mapToSource(filterGroupSwitches[grp]->index(index, 0)).row();
    if (ok && model->getFuncGroupSwitchStart(grp + 1, switchcnt) != sw) {
      model->setFuncGroupSwitchStart(grp + 1, sw, switchcnt);
      update();
      emit modified();
    }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_groupChanged(int value)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    int sw = sender()->property("index").toInt(&ok);
    unsigned int grp = filterSwitchGroups[sw]->mapToSource(filterSwitchGroups[sw]->index(value, 0)).row();
    if (ok && model->getFuncSwitchGroup(sw) != grp) {
      unsigned oldGrp = model->getFuncSwitchGroup(sw);
      if (model->getFuncSwitchAlwaysOnGroup(grp)) {
        if (model->getFuncSwitchConfig(sw) == ModelData::FUNC_SWITCH_CONFIG_TOGGLE)
          model->setFuncSwitchConfig(sw, ModelData::FUNC_SWITCH_CONFIG_2POS);
      }
      if ((grp == 0) || (model->getFuncGroupSwitchStart(grp, switchcnt) == 0))
        model->setFuncSwitchStart(sw, ModelData::FUNC_SWITCH_START_PREVIOUS);
      else
        model->setFuncSwitchStart(sw, ModelData::FUNC_SWITCH_START_OFF);
      model->setFuncSwitchGroup(sw, grp);
      if (oldGrp > 0)
        model->setFuncGroupSwitchStart(oldGrp, model->getFuncGroupSwitchStart(oldGrp, switchcnt), switchcnt);
      update();
      emit modified();
      emit updateDataModels();
    }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_alwaysOnGroupChanged(int value)
{
  if (!sender())
    return;

  QCheckBox * cb = qobject_cast<QCheckBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    int grp = sender()->property("index").toInt(&ok) + 1;

    if (ok) {
      model->setFuncSwitchAlwaysOnGroup(grp, (unsigned int)value);
      if (value) {
        for (int i = 0; i < switchcnt; i += 1) {
          if (((int)model->getFuncSwitchGroup(i) == grp) && (model->getFuncSwitchConfig(i) == ModelData::FUNC_SWITCH_CONFIG_TOGGLE))
            model->setFuncSwitchConfig(i, ModelData::FUNC_SWITCH_CONFIG_2POS);
        }
        if ((int)model->getFuncGroupSwitchStart(grp, switchcnt) == switchcnt + 1) {
          for (int i = 0; i < switchcnt; i += 1) {
            if ((int)model->getFuncSwitchGroup(i) == grp)
              model->setFuncSwitchStart(i, ModelData::FUNC_SWITCH_START_PREVIOUS);
          }
        }
      }
      update();
      emit modified();
    }

    lock = false;
  }
}
