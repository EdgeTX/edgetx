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

#include "firmwarepicker.h"
#include "ui_firmwarepicker.h"
#include "eeprominterface.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <algorithm>

FirmwarePicker::FirmwarePicker(QWidget * parent, const QString & type) :
  QDialog(parent),
  ui(new Ui::FirmwarePicker),
  m_oldtype(type),
  m_currentItem(nullptr)
{
  ui->setupUi(this);

  ui->treeWidget->header()->hide();
  ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  QList<QStringList> list;

  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    if (firmware->isBase()) {
      QStringList item;
      item.append(firmware->getFullName());
      item.append(Boards::getManufacturer(firmware->getBoard()));
      item.append(firmware->getShortName());
      item.append(firmware->getId());
      list.append(item);
    }
  }

  // sort by full name
  std::sort(list.begin(), list.end(), [] (const QStringList &a, const QStringList &b)
  {
    return QString::compare(a.at(0), b.at(0), Qt::CaseInsensitive) < 0;
  });

  ui->treeWidget->setColumnCount(1);
  ui->treeWidget->header()->hide();
  ui->treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  QTreeWidgetItem *lastManu = nullptr;

  for (int i = 0; i < list.count(); i++) {
    if (!lastManu || lastManu->text(0) != list.at(i).at(1)) {
      QTreeWidgetItem * manu = new QTreeWidgetItem();
      manu->setText(0, list.at(i).at(1));
      manu->setFlags(manu->flags() &~ Qt::ItemIsSelectable);
      ui->treeWidget->addTopLevelItem(manu);
      lastManu = manu;
    }

    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0, list.at(i).at(2));                 // short name
    item->setData(0, Qt::UserRole, list.at(i).at(3));   // id

    lastManu->addChild(item);

    if (m_oldtype == list.at(i).at(3))
      m_currentItem = item;
  }

  if (m_currentItem) {
    QTreeWidgetItem *parentItem = m_currentItem->parent();

    while (parentItem != nullptr) {
        parentItem->setExpanded(true);
        parentItem = parentItem->parent();
    }
  }

  // running adjustSize inline does not give enogh time for widgets to have updated their geometry
  QTimer::singleShot(0, this, &FirmwarePicker::adjustSize);
  // need to allow time for adjustSize to complete otherwise this has no effect
  // this is an abitary delay through trial and error
  QTimer::singleShot(100, this, [this] () {
    if (m_currentItem) {
      ui->treeWidget->scrollToItem(m_currentItem, QAbstractItemView::PositionAtCenter);
      ui->treeWidget->setCurrentItem(m_currentItem);
    }
  });
}

FirmwarePicker::~FirmwarePicker()
{
  delete ui;
}

void FirmwarePicker::accept()
{
  if (ui->treeWidget->selectedItems().count() > 0) {
    m_currentItem = ui->treeWidget->selectedItems().first();
    QString newtype = m_currentItem->data(0, Qt::UserRole).toString();

    if (newtype != m_oldtype)
      emit firmwareTypeChanged(newtype);
  }

  QDialog::accept();
}

void FirmwarePicker::shrink()
{
  adjustSize();

  if (m_currentItem)
    ui->treeWidget->scrollToItem(m_currentItem);
}