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
#include "progressdialog.h"

#include <QFileSystemModel>

RadioRestoreDlg::RadioRestoreDlg(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::RadioRestoreDialog),
  mdlSDCard(nullptr),
  restoreFirmware(false)
{
  setupUi();

  if (!loadBackup())
    reject();

  scanForRadio();

  ui->btnBackupFileSelector->setup();


}

void accepted()
{
  // save choices for use next time and restoration eg last backup file
  // progress dialog
  // do it

}

bool RadioRestoreDlg::loadBackup()
{
  // select backup file using last saved settings
  QString backupFile;

  ui->btnBackupSelect.setup();


  if (backupFile.isEmpty())
    return false;

  // if found populate treeview
  QFileSystemModel *mdlSDCard = new QFileSystemModel();
  mdlSDCard->setRootPath(ui->leBackupPath.text());
  mdlSDCard->setOptions(QFileSystemModel::DontWatchForChanges);
  mdlSDCard->setReadOnly(true);
  mdlSDCard->sort(0);

  ui->tvwBackup->setModel(mdlSDCard);

  return true;
}

bool RadioRestoreDlg::scanForRadio()
{

  return true;
}

