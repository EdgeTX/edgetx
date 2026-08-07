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

#include "radiobackupdialog.h"
#include "ui_radiobackupdialog.h"

#include <QFileSystemModel>

RadioBackupDialog::RadioBackupDialog(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::RadioBackupDialog),
  mdlSDCard(new QFileSystemModel()),
  backupFirmware(false)
{
  ui->setupUi(this);
  scanForRadio();

  //ui->btnBackupFileSelector->setup();


}

RadioBackupDialog::~RadioBackupDialog()
{
  delete ui;
}

//void RadioBackupDialog::accepted()
//{
  // save choices for use next time and restoration eg last backup file
  // progress dialog
  // do it

//}

void RadioBackupDialog::scanForRadio()
{
  bool found = false;
  QString radioPath;

  // look for the SD Card refer flashing
  // see if can determine if driver is EdgeTX
  // maybe try to read firmware or some attributes

  if (!found)
    return;

  // if found populate treeview
  QFileSystemModel *fsys = new QFileSystemModel();
  fsys->setRootPath(radioPath);
  fsys->setOptions(QFileSystemModel::DontWatchForChanges);
  fsys->setReadOnly(true);
  fsys->sort(0);

  ui->tvwSDCard->setModel(fsys);

}

