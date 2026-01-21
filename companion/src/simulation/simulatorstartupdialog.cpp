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

#include "simulatorstartupdialog.h"
#include "ui_simulatorstartupdialog.h"

#include "appdata.h"
#include "constants.h"
#include "simulatorinterface.h"
#include "eeprominterface.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

using namespace Simulator;

extern AppData g;

SimulatorStartupDialog::SimulatorStartupDialog(SimulatorOptions * options, int * profId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulatorStartupDialog),
  m_options(options),
  m_profileId(profId),
  m_simProxy(nullptr)

{
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/icon.png"));
  this->setWindowTitle(QString("%1 - %2").arg(CPN_STR_SIMU_NAME).arg(tr("Startup Options")));

  QMapIterator<int, QString> pi(g.getActiveProfiles());
  while (pi.hasNext()) {
    pi.next();
    ui->radioProfile->addItem(pi.value(), pi.key());
  }

  QStandardItemModel * fwModel = new QStandardItemModel(this);
  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    QStandardItem * item = new QStandardItem();
    //qDebug() << "name:" << firmware->getName() << "firmware:" << firmware->getId() << "simulator:" << firmware->getSimulatorId();
    item->setText(firmware->getName());
    item->setData(firmware->getId(), IMDR_Id);
    item->setData(firmware->getSimulatorId(), IMDR_SimulatorId);
    fwModel->appendRow(item);
  }

  QSortFilterProxyModel * fwProxy = new QSortFilterProxyModel(this);
  fwProxy->setSourceModel(fwModel);
  fwProxy->sort(1);

  ui->cbRadioType->setModel(fwProxy);

  QStandardItemModel * simModel = new QStandardItemModel(this);
  foreach(QString sim, SimulatorLoader::getAvailableSimulators()) {
    QStandardItem * item = new QStandardItem();
    item->setText(sim);
    simModel->appendRow(item);
  }

  m_simProxy = new QSortFilterProxyModel(this);
  m_simProxy->setSourceModel(simModel);
  m_simProxy->sort(1);

  ui->cbSimulator->setModel(m_simProxy);

  ui->optGrp_dataSource->setId(ui->optFile, SimulatorOptions::START_WITH_FILE);
  ui->optGrp_dataSource->setId(ui->optFolder, SimulatorOptions::START_WITH_FOLDER);
  ui->optGrp_dataSource->setId(ui->optSdPath, SimulatorOptions::START_WITH_SDPATH);

  SimulatorIcon icon("folder_open");
  ui->btnSelectDataFile->setIcon(icon);
  ui->btnSelectDataFolder->setIcon(icon);
  ui->btnSelectSdPath->setIcon(icon);

  loadRadioProfile(*m_profileId);

  QObject::connect(ui->radioProfile, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimulatorStartupDialog::onRadioProfileChanged);
  QObject::connect(ui->cbRadioType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimulatorStartupDialog::onRadioTypeChanged);
  QObject::connect(ui->btnSelectDataFile, &QToolButton::clicked, this, &SimulatorStartupDialog::onDataFileSelect);
  QObject::connect(ui->btnSelectDataFolder, &QToolButton::clicked, this, &SimulatorStartupDialog::onDataFolderSelect);
  QObject::connect(ui->btnSelectSdPath, &QToolButton::clicked, this, &SimulatorStartupDialog::onSdPathSelect);

  if (ui->radioProfile->count() < 1) {
    // give Startup dialog time to display so this error message can overlay it
    QTimer::singleShot(250, [=] {
      QMessageBox::critical(this, CPN_STR_SIMU_NAME, tr("No radio profiles have been found. Use %1 to create.").arg(CPN_STR_APP_NAME));
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    });
  }

}

SimulatorStartupDialog::~SimulatorStartupDialog()
{
  delete ui;
  delete m_simProxy;
}

void SimulatorStartupDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

// FIXME : need a better way to check for this
bool SimulatorStartupDialog::usesCategorizedStorage(const QString & name)
{
  return true;
}

bool SimulatorStartupDialog::usesCategorizedStorage()
{
  return usesCategorizedStorage(ui->cbRadioType->currentText());
}

QString SimulatorStartupDialog::findRadioId(const QString & str)
{
  QString radioId(str);
  int pos = str.indexOf("-");
  if (pos > 0) {
    pos = str.indexOf("-", pos + 1);
    if (pos > 0) {
      radioId = str.mid(0, pos);
    }
  }
  return radioId;
}

// TODO : this could be smarter and actually look for a matching file in the folder
QString SimulatorStartupDialog::radioEepromFileName(const QString & simulatorId, QString folder)
{
  QString eepromFileName = "", ext = "bin";

  if (folder.isEmpty())
    folder = g.eepromDir();

  QString radioId = findRadioId(simulatorId);
  int pos = radioId.indexOf("-");
  if (pos > 0)
    radioId = radioId.mid(pos+1);
  if (usesCategorizedStorage(radioId))
    ext = "etx";

  eepromFileName = QString("%1.%2").arg(radioId, ext);
  eepromFileName = QDir(folder).filePath(eepromFileName.toLatin1());
  // qDebug() << "radioId" << radioId << "eepromFileName" << eepromFileName;

  return eepromFileName;
}

void SimulatorStartupDialog::updateContainerTypes(void)
{
  static int oldstate = -1;
  int state = usesCategorizedStorage();

  if (state == oldstate)
    return;

  oldstate = state;

  ui->wdgt_dataSource->setVisible(state);
  ui->layout_options->labelForField(ui->wdgt_dataSource)->setVisible(state);
  ui->wdgt_dataFolder->setVisible(state);
  ui->layout_options->labelForField(ui->wdgt_dataFolder)->setVisible(state);

  if (!state || ui->optGrp_dataSource->checkedId() < 0)
    ui->optFile->setChecked(true);

}

void SimulatorStartupDialog::loadRadioProfile(int id)
{
  QString tmpstr, tmpstr2;
  int i;

  if (id < 0 || !g.getActiveProfiles().contains(id))
    return;

  i = ui->radioProfile->findData(id);
  if (i > -1 && ui->radioProfile->currentIndex() != i)
    ui->radioProfile->setCurrentIndex(i);

  *m_options = g.profile[id].simulatorOptions();

  if (m_options->firmwareId.isEmpty() && !g.profile[id].fwType().isEmpty())
    m_options->firmwareId = g.profile[id].fwType();

  m_options->firmwareId = findRadioId(m_options->firmwareId);

  i = ui->cbRadioType->findData(m_options->firmwareId, IMDR_Id, Qt::MatchContains);
  if (i > -1)
    ui->cbRadioType->setCurrentIndex(i);

  setGlobalFirmware(m_options->firmwareId);

  //  always refresh as linked simulatorId as could change over time
  m_options->simulatorId = getCurrentFirmware()->getSimulatorId();

  m_simProxy->setFilterFixedString(m_options->simulatorId);

  i = ui->cbSimulator->findText(findRadioId(m_options->simulatorId), Qt::MatchContains);
  if (i > -1)
    ui->cbSimulator->setCurrentIndex(i);

  //qDebug() << "firmware:" << m_options->firmwareId << "simulator:" << m_options->simulatorId;

  tmpstr = m_options->dataFile;
  if (tmpstr.isEmpty())
    tmpstr = radioEepromFileName(ui->cbRadioType->currentData().toString());
  ui->dataFile->setText(tmpstr);

  tmpstr = m_options->dataFolder;
  if (tmpstr.isEmpty())
    tmpstr = g.eepromDir();
  ui->dataFolder->setText(tmpstr);

  tmpstr = m_options->sdPath;
  if (tmpstr.isEmpty())
    tmpstr = g.profile[id].sdPath();
  ui->sdPath->setText(tmpstr);

  foreach (QAbstractButton * btn, ui->optGrp_dataSource->buttons()) {
    if (ui->optGrp_dataSource->id(btn) == m_options->startupDataType) {
      btn->setChecked(true);
      break;
    }
  };

  updateContainerTypes();
}

void SimulatorStartupDialog::accept()
{
  if (ui->cbSimulator->currentText() == "")
    return;

  *m_profileId = ui->radioProfile->currentData().toInt();
  m_options->firmwareId = ui->cbRadioType->currentData().toString();
  m_options->simulatorId = ui->cbSimulator->currentText();
  m_options->dataFile = ui->dataFile->text();
  m_options->dataFolder = ui->dataFolder->text();
  m_options->sdPath = ui->sdPath->text();
  m_options->startupDataType = ui->optGrp_dataSource->checkedId();

  QDialog::accept();
}

void SimulatorStartupDialog::onRadioProfileChanged(int index)
{
  if (index < 0)
    return;

  loadRadioProfile(ui->radioProfile->currentData().toInt());
}

void SimulatorStartupDialog::onRadioTypeChanged(int index)
{
  if (index < 0)
    return;

  QString id = ui->cbRadioType->currentData(IMDR_Id).toString();
  ui->dataFile->setText(radioEepromFileName(id));
  updateContainerTypes();

  setGlobalFirmware(id);

  //  always refresh as linked simulatorId as could change over time
  id = getCurrentFirmware()->getSimulatorId();

  m_simProxy->setFilterFixedString(id);

  const int i = ui->cbSimulator->findText(findRadioId(id), Qt::MatchContains);
  if (i > -1)
    ui->cbSimulator->setCurrentIndex(i);
}

void SimulatorStartupDialog::onDataFileSelect(bool)
{
  QString filter = SIMU_FILES_FILTER;
  QString file = QFileDialog::getSaveFileName(this, tr("Select a data file"), ui->dataFile->text(),
                                              filter, NULL, QFileDialog::DontConfirmOverwrite);
  if (!file.isEmpty()) {
    ui->dataFile->setText(file);
    ui->optFile->setChecked(true);
  }
}

void SimulatorStartupDialog::onDataFolderSelect(bool)
{
  QString folder = QFileDialog::getExistingDirectory(this, tr("Select Data Directory"),
                                                     ui->dataFolder->text(), QFileDialog::DontUseNativeDialog);
  if (!folder.isEmpty()) {
    ui->dataFolder->setText(folder);
    if (usesCategorizedStorage())
      ui->optFolder->setChecked(true);
  }
}

void SimulatorStartupDialog::onSdPathSelect(bool)
{
  QString folder = QFileDialog::getExistingDirectory(this, tr("Select SD Card Image Folder"),
                                                     ui->sdPath->text(), QFileDialog::DontUseNativeDialog);
  if (!folder.isEmpty()) {
    ui->sdPath->setText(folder);
    if (usesCategorizedStorage())
      ui->optSdPath->setChecked(true);
  }
}

void SimulatorStartupDialog::setGlobalFirmware(const QString & id)
{
  Firmware::setCurrentVariant(Firmware::getFirmwareForId(id));
  //qDebug() << "current firmware:" << getCurrentFirmware()->getId();
}
