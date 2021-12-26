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

#include "simulatorstartupdialog.h"
#include "ui_simulatorstartupdialog.h"

#include "appdata.h"
#include "constants.h"
#include "simulatorinterface.h"

#include <QFileDialog>

using namespace Simulator;

extern AppData g;

SimulatorStartupDialog::SimulatorStartupDialog(SimulatorOptions * options, int * profId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulatorStartupDialog),
  m_options(options),
  m_profileId(profId)
{
  ui->setupUi(this);

  QMapIterator<int, QString> pi(g.getActiveProfiles());
  while (pi.hasNext()) {
    pi.next();
    ui->radioProfile->addItem(pi.value(), pi.key());
  }

  ui->radioType->addItems(SimulatorLoader::getAvailableSimulators());

  ui->optGrp_dataSource->setId(ui->optFile, SimulatorOptions::START_WITH_FILE);
  ui->optGrp_dataSource->setId(ui->optFolder, SimulatorOptions::START_WITH_FOLDER);
  ui->optGrp_dataSource->setId(ui->optSdPath, SimulatorOptions::START_WITH_SDPATH);

  SimulatorIcon icon("folder_open");
  ui->btnSelectDataFile->setIcon(icon);
  ui->btnSelectDataFolder->setIcon(icon);
  ui->btnSelectSdPath->setIcon(icon);

  loadRadioProfile(*m_profileId);

  QObject::connect(ui->radioProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(onRadioProfileChanged(int)));
  QObject::connect(ui->radioType,  SIGNAL(currentIndexChanged(int)), this, SLOT(onRadioTypeChanged(int)));
  QObject::connect(ui->btnSelectDataFile, &QToolButton::clicked, this, &SimulatorStartupDialog::onDataFileSelect);
  QObject::connect(ui->btnSelectDataFolder, &QToolButton::clicked, this, &SimulatorStartupDialog::onDataFolderSelect);
  QObject::connect(ui->btnSelectSdPath, &QToolButton::clicked, this, &SimulatorStartupDialog::onSdPathSelect);

}

SimulatorStartupDialog::~SimulatorStartupDialog()
{
  delete ui;
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
  return name.contains(QRegExp("(x12|x10|horus|16|18|nv14|pl18)", Qt::CaseInsensitive));
}

bool SimulatorStartupDialog::usesCategorizedStorage()
{
  return usesCategorizedStorage(ui->radioType->currentText());
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
QString SimulatorStartupDialog::radioEepromFileName(const QString & firmwareId, QString folder)
{
  QString eepromFileName = "", ext = "bin";

  if (folder.isEmpty())
    folder = g.eepromDir();

  QString radioId = findRadioId(firmwareId);
  int pos = radioId.indexOf("-");
  if (pos > 0)
    radioId = radioId.mid(pos+1);
  if (usesCategorizedStorage(radioId))
    ext = "etx";

  eepromFileName = QString("eeprom-%1.%2").arg(radioId, ext);
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

  tmpstr = m_options->firmwareId;
  if (tmpstr.isEmpty() && !g.profile[id].fwType().isEmpty())
    tmpstr = g.profile[id].fwType();
  else if (!(tmpstr2 = SimulatorLoader::findSimulatorByFirmwareName(m_options->firmwareId)).isEmpty())
    tmpstr = tmpstr2;
  i = ui->radioType->findText(findRadioId(tmpstr), Qt::MatchContains);
  if (i > -1)
    ui->radioType->setCurrentIndex(i);

  tmpstr = m_options->dataFile;
  if (tmpstr.isEmpty())
    tmpstr = radioEepromFileName(ui->radioType->currentText());
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
  *m_profileId = ui->radioProfile->currentData().toInt();
  m_options->firmwareId = ui->radioType->currentText();
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
  ui->dataFile->setText(radioEepromFileName(ui->radioType->currentText()));
  updateContainerTypes();
}

void SimulatorStartupDialog::onDataFileSelect(bool)
{
  QString filter = EEPROM_FILES_FILTER % tr("All files (*.*)");
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
