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

#include "logicalswitches.h"
#include "logicalswitchestablemodel.h"
#include "logicalswitchesdelegates.h"
#include "filtereditemmodels.h"
#include "compounditemmodels.h"
#include "helpers.h"

#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMessageBox>

LogicalSwitchesPanel::LogicalSwitchesPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings,
                                           Firmware * firmware, CompoundItemModelFactory * sharedItemModels) :
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels),
  selectedIndex(0),
  modelsUpdateCnt(0)
{
  rawSwitchFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                  RawSwitch::LogicalSwitchesContext);
  connectItemModelEvents(rawSwitchFilteredModel);

  rawSourceFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource));
  connectItemModelEvents(rawSourceFilteredModel);

  m_tableModel = new LogicalSwitchesTableModel(&model, generalSettings, firmware,
                                                rawSwitchFilteredModel, rawSourceFilteredModel, this);

  m_tableView = new QTableView(this);
  m_tableView->setModel(m_tableModel);
  m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
  m_tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
  m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_tableView->verticalHeader()->hide();
  m_tableView->horizontalHeader()->setStretchLastSection(true);
  m_tableView->setAlternatingRowColors(true);

  // Set delegates per column
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_FUNCTION,
                                         new LogicalSwitchFunctionDelegate(this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_V1,
                                         new LogicalSwitchV1Delegate(m_tableModel, this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_V2,
                                         new LogicalSwitchV2Delegate(m_tableModel, this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_AND_SWITCH,
                                         new LogicalSwitchComboDelegate(m_tableModel, this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_DURATION,
                                         new LogicalSwitchNumericDelegate(LogicalSwitchesTableModel::DURATION_ENABLED, this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_DELAY,
                                         new LogicalSwitchNumericDelegate(LogicalSwitchesTableModel::DELAY_ENABLED, this));
  m_tableView->setItemDelegateForColumn(LogicalSwitchesTableModel::COL_PERSISTENT,
                                         new LogicalSwitchPersistDelegate(this));

  auto * layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tableView);

  m_tableView->resizeColumnsToContents();

  connect(m_tableView, &QWidget::customContextMenuRequested, this, &LogicalSwitchesPanel::onCustomContextMenuRequested);
  connect(m_tableModel, &LogicalSwitchesTableModel::functionFamilyChanged, this, &LogicalSwitchesPanel::onFunctionFamilyChanged);
  connect(m_tableModel, &LogicalSwitchesTableModel::v1SourceTypeChanged, this, &LogicalSwitchesPanel::onV1SourceTypeChanged);
  connect(m_tableModel, &LogicalSwitchesTableModel::logicalSwitchStateChanged, this, &LogicalSwitchesPanel::onLogicalSwitchStateChanged);
  connect(m_tableModel, &LogicalSwitchesTableModel::modified, this, &ModelPanel::modified);
}

LogicalSwitchesPanel::~LogicalSwitchesPanel()
{
  delete rawSourceFilteredModel;
  delete rawSwitchFilteredModel;
}

void LogicalSwitchesPanel::update()
{
  m_tableModel->refreshAllRows();
  m_tableView->resizeColumnsToContents();
}

void LogicalSwitchesPanel::onFunctionFamilyChanged(int row)
{
  // Close any open editor since the widget type may need to change
  m_tableView->closePersistentEditor(m_tableView->currentIndex());
  m_tableModel->refreshAllRows();
  m_tableView->resizeColumnsToContents();
}

void LogicalSwitchesPanel::onV1SourceTypeChanged(int row)
{
  m_tableView->closePersistentEditor(m_tableView->currentIndex());
  emit m_tableModel->dataChanged(m_tableModel->index(row, LogicalSwitchesTableModel::COL_V2),
                                  m_tableModel->index(row, LogicalSwitchesTableModel::COL_V2));
}

void LogicalSwitchesPanel::onLogicalSwitchStateChanged()
{
  lock = true;
  sharedItemModels->update(AbstractItemModel::IMUE_LogicalSwitches);
  lock = false;
}

void LogicalSwitchesPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &LogicalSwitchesPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &LogicalSwitchesPanel::onItemModelUpdateComplete);
}

void LogicalSwitchesPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void LogicalSwitchesPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}

// ---- Context Menu ----

void LogicalSwitchesPanel::onCustomContextMenuRequested(QPoint pos)
{
  QModelIndex idx = m_tableView->indexAt(pos);
  if (!idx.isValid()) return;

  selectedIndex = idx.row();
  QPoint globalPos = m_tableView->viewport()->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this, SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this, SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this, SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this, SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this, SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(cmClearAll()));

  contextMenu.exec(globalPos);
}

bool LogicalSwitchesPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_LOGICAL_SWITCH)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_LOGICAL_SWITCH));
    return true;
  }
  return false;
}

bool LogicalSwitchesPanel::insertAllowed() const
{
  return ((selectedIndex < m_tableModel->rowCount() - 1) &&
          (model->logicalSw[m_tableModel->rowCount() - 1].isEmpty()));
}

bool LogicalSwitchesPanel::moveDownAllowed() const
{
  return selectedIndex < m_tableModel->rowCount() - 1;
}

bool LogicalSwitchesPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void LogicalSwitchesPanel::cmCopy()
{
  QByteArray data;
  data.append((char*)&model->logicalSw[selectedIndex], sizeof(LogicalSwitchData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_LOGICAL_SWITCH, data);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void LogicalSwitchesPanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Logical Switch. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void LogicalSwitchesPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    m_tableModel->pasteRow(selectedIndex, data.constData(), data.size());
    update();
  }
}

void LogicalSwitchesPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Logical Switch. Are you sure?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }
  m_tableModel->clearRow(selectedIndex);
  update();
}

void LogicalSwitchesPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Logical Switches. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  m_tableModel->clearAllRows();
  update();
}

void LogicalSwitchesPanel::cmInsert()
{
  m_tableModel->lsInsert(selectedIndex);
  update();
}

void LogicalSwitchesPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Logical Switch. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  m_tableModel->lsDelete(selectedIndex);
  update();
}

void LogicalSwitchesPanel::cmMoveUp()
{
  m_tableModel->swapRows(selectedIndex, selectedIndex - 1);
  update();
}

void LogicalSwitchesPanel::cmMoveDown()
{
  m_tableModel->swapRows(selectedIndex, selectedIndex + 1);
  update();
}
