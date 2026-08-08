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

#include "radiorestoredialog.h"
#include "ui_radiorestoredialog.h"

#include <QFileSystemModel>

RadioRestoreDialog::RadioRestoreDialog(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::RadioRestoreDialog),
  mdlSDCard(nullptr),
  restoreFirmware(false)
{
  ui->setupUi(this);

  if (!loadBackup())
    reject();

  scanForRadio();

  //ui->btnBackupFileSelector->setup();


}

RadioRestoreDialog::~RadioRestoreDialog()
{
  delete ui;
}

//void RadioRestoreDialog::accepted()
//{
  // save choices for use next time and restoration eg last backup file
  // progress dialog
  // do it

//}

bool RadioRestoreDialog::loadBackup()
{
  // select backup file using last saved settings
  QString backupFile;

  //ui->btnBackupSelect.setup();

  // check backup radio type in backup file to current profile
  // Warn if different and prompt to continue or reselect backup file or exit

  if (backupFile.isEmpty())
    return false;

  // if found populate treeview
  QFileSystemModel *mdlSDCard = new QFileSystemModel();
  mdlSDCard->setRootPath(ui->leBackupFile->text());
  mdlSDCard->setOptions(QFileSystemModel::DontWatchForChanges);
  mdlSDCard->setReadOnly(true);
  mdlSDCard->sort(0);

  ui->tvwSDCard->setModel(mdlSDCard);

  return true;
}

bool RadioRestoreDialog::scanForRadio()
{

  return true;
}

