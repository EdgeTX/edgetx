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
#include "radiointerface.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDir>

QStringList DEFAULT_FOLDERS = { "LOGS", "MODELS", "RADIO" };

RadioBackupDialog::RadioBackupDialog(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::RadioBackupDialog),
  backupFirmware(false)
{
  ui->setupUi(this);

  findRadio();

  connect(ui->cboSDCards, &QComboBox::currentIndexChanged, this, &RadioBackupDialog::buildTree);

  // default name is current profile radio plus date and time plus .etxb extension
  //ui->btnBackupFileSelector->setup();
}

RadioBackupDialog::~RadioBackupDialog()
{
  delete ui;
}

//void RadioBackupDialog::accepted()
//{
  //save choices for use next time and restoration eg last backup file
  //progress dialog
  // do it

  // save radio type to backup file so it can be checked on restore


  // launch progress dialog

//}

void RadioBackupDialog::findRadio()
{
  if (isRadioConnected()) {
    ui->chkFirmware->setEnabled(true);
    loadSDCards();
  } else {
    ui->chkFirmware->setEnabled(true);
    ui->cboSDCards->clear();
  }
}

void RadioBackupDialog::loadSDCards()
{
  ui->cboSDCards->clear();
  ui->cboSDCards->addItems(findSDCards("RADIO"));

  if (ui->cboSDCards->count() > 0) {
    ui->cboSDCards->setCurrentIndex(0);
    buildTree();
  }
}

// Recursive function to populate tree items from a directory path
void RadioBackupDialog::addFilesToTree(const QString &path, QTreeWidgetItem *parentItem)
{
  QDir dir(path);
  QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QFileInfo &fileInfo : list) {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, fileInfo.fileName());

    // add our flags
    Qt::ItemFlags ourflags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsAutoTristate;
    item->setFlags(item->flags() | ourflags);

    // checked by default
    item->setCheckState(0, DEFAULT_FOLDERS.contains(fileInfo.fileName()) ? Qt::Checked : Qt::Unchecked);

    if (parentItem) {
      parentItem->addChild(item);
    } else {
      // Add as top-level if no parent specified
      ui->treeSDCard->addTopLevelItem(item);
    }

    // If a directory and children allowed, recurse into it
    // Qt will ignore adding chidren branches but no point wasting processing time
    if (fileInfo.isDir() && !(item->flags() & Qt::ItemNeverHasChildren)) {
        addFilesToTree(fileInfo.absoluteFilePath(), item);
    }
  }
}

void RadioBackupDialog::buildTree()
{
  ui->treeSDCard->setHeaderLabel(tr("Radio"));
  ui->treeSDCard->setColumnCount(1);
  ui->treeSDCard->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  // root item
  QTreeWidgetItem *root = new QTreeWidgetItem();
  root->setText(0, tr("sd card"));
  root->setFlags(root->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsAutoTristate);
  root->setCheckState(0, Qt::Unchecked);
  ui->treeSDCard->addTopLevelItem(root);
  // add children
  addFilesToTree(ui->cboSDCards->currentText(), root);  // need to split to get root
  ui->treeSDCard->expandItem(root); // expand the first level
}
