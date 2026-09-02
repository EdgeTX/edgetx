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

#include "generalkeyshortcuts.h"
#include "compounditemmodels.h"
#include "exclusivecombogroup.h"
#include "helpers.h"

#include <QLabel>
#include <QGridLayout>

GeneralKeysPanel::GeneralKeysPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  params(new QList<QWidget *>),
  row(0),
  col(0),
  lock(true),
  cboShortcuts(new QList<AutoComboBox *>),
  cboShortcutTools(new QList<AutoComboBox *>),
  strKeyShortcutTools(new QList<QString *>)
{
  grid = new QGridLayout(this);
  // All values except QM_NONE and QM_APP are mutually exclusive
  cboQMGrp = new ExclusiveComboGroup(this,
                  [=](const QVariant &value) { return value == GeneralSettings::QM_NONE ||
                                                      value == GeneralSettings::QM_APP; });
  const int cnt = firmware->getCapability(KeyShortcuts);

  // add lua tool scripts from radio profile sdcard
  QStringList toolsSet = getListLuaTools();
  QStandardItemModel *mdl = new QStandardItemModel(this);

  for (int i = 0; i < toolsSet.size(); i++) {
    QStandardItem *item = new QStandardItem(toolsSet[i]);
    mdl->appendRow(item);
  }

  // Ensure existing configured tool is included in toolsSet as the radio profile
  // sdcard contents may not match radio
  for (int i = 0; i < cnt; i++) {
    if (generalSettings.qmFavorites[i] == GeneralSettings::QM_APP) {
      QString temp(generalSettings.qmFavoritesTools[i]);
      if (!temp.isEmpty() && mdl->findItems(temp).size() < 1) {
        QStandardItem *item = new QStandardItem(temp);
        mdl->appendRow(item);
      }
    }
  }

  QSortFilterProxyModel *mdlTools = new QSortFilterProxyModel(this);
  mdlTools->setSourceModel(mdl);
  mdlTools->setSortCaseSensitivity(Qt::CaseInsensitive);
  mdlTools->setFilterKeyColumn(0);
  mdlTools->sort(0);

  const int split = 2;

  for (int i = 0; i < split; i++) {
    if (i == 0) {
      addSection(tr("Short Press"));
    } else {
      addLine();
      addSection(tr("Long Press"));
    }

    for (int j = 0; j < (cnt / split); j++) {
      const int idx = (i * (split + 1)) + j;
      addLabel(j == 0 ? tr("MDL") : (j == 1 ? tr("SYS") : tr("TELE")));
      AutoComboBox *cboShortcut = new AutoComboBox(this);
      cboShortcut->setProperty("index", idx);
      // separate item model per combo box due to mutual exclusivity
      cboShortcut->setModel(generalSettings.quickMenuItemModel(true));
      cboShortcut->setField(generalSettings.keyShortcuts[idx], this);
      connect(cboShortcut, &AutoComboBox::currentDataChanged, this, &GeneralKeysPanel::on_shortcutChanged);
      cboShortcuts->append(cboShortcut);
      cboQMGrp->addCombo(cboShortcut);
      params->append(cboShortcut);

      AutoComboBox *cboShortcutTool = new AutoComboBox(this);
      cboShortcutTool->setProperty("index", idx);
      cboShortcutTool->setModel(mdlTools);
      // AutoComboBox does not support char * so use a proxy
      QString *str = new QString(generalSettings.keyShortcutTools[idx]);
      strKeyShortcutTools->append(str);
      cboShortcutTool->setField(*str, this);
      connect(cboShortcutTool, &AutoComboBox::currentDataChanged, this, &GeneralKeysPanel::on_shortcutToolChanged);
      cboShortcutTools->append(cboShortcutTool);
      params->append(cboShortcutTool);
      addParams();
    }
  }

  addLine();

  QPushButton *reset = new QPushButton(tr("Reset"));
  connect(reset, &QPushButton::clicked, [&] ()
  {
    generalSettings.setDefaultKeyShortcuts();
    update();
    initComboQMGroup();
  });
  params->append(reset);
  addParams();

  update();
  initComboQMGroup();
  addVSpring(grid, 0, grid->rowCount());
  addHSpring(grid, grid->columnCount(), 0);
  disableMouseScrolling();
  lock = false;
}

GeneralKeysPanel::~GeneralKeysPanel()
{
  if (cboShortcutTools) {
    delete cboShortcutTools;  // dialog destructor will delete members
  }

  if (cboShortcuts) {
    delete cboShortcuts;  // dialog destructor will delete members
  }

  if (strKeyShortcutTools) {
    for (auto it = strKeyShortcutTools->begin(); it != strKeyShortcutTools->end(); ++it) {
      delete *it;
    }

    delete strKeyShortcutTools;
  }

  if (params) {
    for (auto it = params->begin(); it != params->end(); ++it) {
      delete *it;
    }

    delete params;
  }
}

void GeneralKeysPanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void GeneralKeysPanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void GeneralKeysPanel::addParams()
{
  int col = 0;
  QGridLayout *subgrid = new QGridLayout();

  for (int i = 0; i < params->size(); i++) {
    subgrid->addWidget(params->at(i), 0, col++);
  }

  addHSpring(subgrid, col, 0);
  grid->addLayout(subgrid, row++, this->col + 1);
  params->clear();
}

void GeneralKeysPanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  row++;
}

void GeneralKeysPanel::initComboQMGroup()
{
  // force mutually exclusive update to lists
  for (int i = 0; i < cboQMGrp->getComboBoxes()->size(); i++) {
    QComboBox *cbo = cboQMGrp->getComboBoxes()->at(i);
    cboQMGrp->handleActivated(cbo, cbo->currentIndex());
  }
}

void GeneralKeysPanel::on_shortcutChanged()
{
  if (!lock) {
    const int idx = sender()->property("index").toInt();
    if (generalSettings.keyShortcuts[idx] == GeneralSettings::QM_APP)
      cboShortcutTools->at(idx)->setCurrentIndex(0);
    setToolName(idx);
    updateRow(idx);
  }
}

void GeneralKeysPanel::on_shortcutToolChanged()
{
  if (!lock) {
    bool ok;
    const int index = sender()->property("index").toInt(&ok);
    if (ok) setToolName(index);
  }
}

void GeneralKeysPanel::setToolName(int index)
{
  if (generalSettings.keyShortcutTools[index])
    delete generalSettings.keyShortcutTools[index];

  if (generalSettings.keyShortcuts[index] == GeneralSettings::QM_APP) {
    // obtain current value from proxy
    std::string str = strKeyShortcutTools->at(index)->toStdString();
    generalSettings.keyShortcutTools[index] = new char[str.size() + 1];
    strncpy(generalSettings.keyShortcutTools[index], str.c_str(), str.size());
    generalSettings.keyShortcutTools[index][str.size()] = 0;
  } else {
    generalSettings.keyShortcutTools[index] = nullptr;
  }
}

void GeneralKeysPanel::update()
{
  const int cnt = cboShortcuts->count();
  for (int i = 0; i < cnt; i++)
    updateRow(i);

}

void GeneralKeysPanel::updateRow(const int index)
{
  lock = true;
  cboShortcuts->at(index)->updateValue();

  if (generalSettings.keyShortcuts[index] == GeneralSettings::QM_APP) {
    cboShortcutTools->at(index)->updateValue();
    cboShortcutTools->at(index)->setVisible(true);
  } else {
    cboShortcutTools->at(index)->setVisible(false);
  }

  lock = false;
}
