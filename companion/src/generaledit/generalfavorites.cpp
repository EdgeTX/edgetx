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

#include "generalfavorites.h"
#include "autocombobox.h"
#include "compounditemmodels.h"
#include "exclusivecombogroup.h"
#include "helpers.h"

#include <QLabel>
#include <QGridLayout>

GeneralFavsPanel::GeneralFavsPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  params(new QList<QWidget *>),
  row(0),
  col(0),
  lock(true),
  cboFavTools(new QList<AutoComboBox *>),
  strFavTools(new QList<QString *>)
{
  grid = new QGridLayout(this);
  // All values except QM_NONE are mutually exclusive
  cboQMGrp = new ExclusiveComboGroup(this,
                  [=](const QVariant &value) { return value == GeneralSettings::QM_NONE ||
                                                      value == GeneralSettings::QM_APP; });
  const int cnt = firmware->getCapability(QMFavourites);

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

  for (int i = 0; i < cnt; i++) {
    addLabel(tr("# %1").arg(i + 1));
    AutoComboBox *cboFav = new AutoComboBox(this);
    cboFav->setProperty("index", i);
    // separate item model per combo box due to mutual exclusivity
    cboFav->setModel(generalSettings.quickMenuItemModel(false));
    cboFav->setField(generalSettings.qmFavorites[i], this);
    connect(cboFav, &AutoComboBox::currentDataChanged, this, &GeneralFavsPanel::on_favChanged);
    cboQMGrp->addCombo(cboFav);
    params->append(cboFav);

    AutoComboBox *cboFavTool = new AutoComboBox(this);
    cboFavTool->setProperty("index", i);
    cboFavTool->setModel(mdlTools);
    // AutoComboBox does not support char * so use a proxy
    QString *str = new QString(generalSettings.qmFavoritesTools[i]);
    strFavTools->append(str);
    cboFavTool->setField(*str, this);

    if (generalSettings.qmFavorites[i] == GeneralSettings::QM_APP) {
      if (cboFavTool->currentIndex() < 0)
        cboFavTool->setCurrentIndex(0);
      cboFavTool->setVisible(true);
    } else {
      cboFavTool->setCurrentIndex(0);
      cboFavTool->setVisible(false);
    }

    connect(cboFavTool, &AutoComboBox::currentDataChanged, this, &GeneralFavsPanel::on_favToolChanged);
    cboFavTools->append(cboFavTool);
    params->append(cboFavTool);
    addParams();
  }

  addLine();

  QPushButton *reset = new QPushButton(tr("Reset"));
  connect(reset, &QPushButton::clicked, [&] ()
  {
    generalSettings.setDefaultFavorites();

    foreach(AutoComboBox *cb, findChildren<AutoComboBox*>())
      cb->updateValue();

    initComboQMGroup();
  });
  params->append(reset);
  addParams();

  lock = false;
  initComboQMGroup();
  addVSpring(grid, 0, grid->rowCount());
  addHSpring(grid, grid->columnCount(), 0);
  disableMouseScrolling();
}

GeneralFavsPanel::~GeneralFavsPanel()
{
  if (cboFavTools) {
    delete cboFavTools; // dialog destructor will delete members
  }

  if (strFavTools) {
    for (auto it = strFavTools->begin(); it != strFavTools->end(); ++it) {
      delete *it;
    }

    delete strFavTools;
  }

  if (params) {
    for (auto it = params->begin(); it != params->end(); ++it) {
      delete *it;
    }

    delete params;
  }
}

void GeneralFavsPanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void GeneralFavsPanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void GeneralFavsPanel::addParams()
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

void GeneralFavsPanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  row++;
}

void GeneralFavsPanel::initComboQMGroup()
{
  // force mutually exclusive update to lists
  for (int i = 0; i < cboQMGrp->getComboBoxes()->size(); i++) {
    QComboBox *cbo = cboQMGrp->getComboBoxes()->at(i);
    cboQMGrp->handleActivated(cbo, cbo->currentIndex());
  }
}

void GeneralFavsPanel::on_favChanged()
{
  if (!lock) {
    const int idx = sender()->property("index").toInt();
    const int i = cboFavTools->at(idx)->currentIndex();
    cboFavTools->at(idx)->setCurrentIndex(0);

    if (i == cboFavTools->at(idx)->currentIndex())
      setToolName(idx); // force update

    cboFavTools->at(idx)->setVisible(generalSettings.qmFavorites[idx] == GeneralSettings::QM_APP);
  }
}

void GeneralFavsPanel::on_favToolChanged()
{
  if (!lock) {
    bool ok;
    const int index = sender()->property("index").toInt(&ok);
    if (ok) setToolName(index);
  }
}

void GeneralFavsPanel::setToolName(int index)
{
  if (generalSettings.qmFavoritesTools[index])
    delete generalSettings.qmFavoritesTools[index];

  if (generalSettings.qmFavorites[index] == GeneralSettings::QM_APP) {
    // obtain current value from proxy
    std::string str = strFavTools->at(index)->toStdString();
    generalSettings.qmFavoritesTools[index] = new char[str.size() + 1];
    strncpy(generalSettings.qmFavoritesTools[index], str.c_str(), str.size());
    generalSettings.qmFavoritesTools[index][str.size()] = 0;
  } else {
    generalSettings.qmFavoritesTools[index] = nullptr;
  }
}
