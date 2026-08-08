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

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDir>


RadioBackupDialog::RadioBackupDialog(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::RadioBackupDialog),
  backupFirmware(false)
{
  ui->setupUi(this);
  scanForRadio();

  // default name is current profile radio plus date and time plus .etxb extension
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

  // save radio type to backup file so it can be checked on restore


  // launch progress dialog

//}

void RadioBackupDialog::scanForRadio()
{
  bool found = false;
  QString radioPath("/utils/edgetx/vscode/radios/v30/tx16s/sdcard");


  // look for the SD Card refer flashing
  // see if can determine if driver is EdgeTX
  // maybe try to read firmware or some attributes

  //if (!found)
  //  return;


  // if found populate treeview

  ui->treeSDCard->setHeaderLabel(tr("Directories"));
  ui->treeSDCard->setColumnCount(1);
  QTreeWidgetItem *root = new QTreeWidgetItem(ui->treeSDCard);
  root->setText(0, tr("SD Card"));
  root->setFlags(root->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsAutoTristate);
  root->setCheckState(0, Qt::Unchecked);
  addFilesToTree(radioPath, root);
  ui->treeSDCard->expandItem(root);

}

// Recursive function to populate tree items from a directory path
void RadioBackupDialog::addFilesToTree(const QString &path, QTreeWidgetItem *parentItem)
{
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &fileInfo : list) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, fileInfo.fileName());

        // Make the item user-checkable
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsAutoTristate);
        item->setCheckState(0, Qt::Unchecked);

        if (parentItem) {
            parentItem->addChild(item);
        } else {
            // Add as top-level if no parent specified
            // treeWidget->addTopLevelItem(item);
        }

        // If it is a directory, recurse into it
        if (fileInfo.isDir()) {
            addFilesToTree(fileInfo.absoluteFilePath(), item);
        }
    }
}

