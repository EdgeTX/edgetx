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

#include "mixes.h"
#include "helpers.h"
#include "filtereditemmodels.h"

MixesPanel::MixesPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  mixInserted(false),
  highlightedSource(0),
  modelPrinter(firmware, generalSettings, model),
  sharedItemModels(sharedItemModels),
  modelsUpdateCnt(0)
{
  connectItemModelEvents(AbstractItemModel::IMID_RawSource);
  connectItemModelEvents(AbstractItemModel::IMID_RawSwitch);
  connectItemModelEvents(AbstractItemModel::IMID_Curve);
  connectItemModelEvents(AbstractItemModel::IMID_GVarRef);

  QGridLayout * mixesLayout = new QGridLayout(this);

  mixersListWidget = new MixersListWidget(this, false); // TODO enum
  QPushButton * qbUp = new QPushButton(this);
  QPushButton * qbDown = new QPushButton(this);
  QPushButton * qbClear = new QPushButton(this);
  qbUp->setText(tr("Move Up"));
  qbUp->setIcon(CompanionIcon("moveup.png"));
  qbUp->setShortcut(QKeySequence(tr("Ctrl+Up")));
  qbDown->setText(tr("Move Down"));
  qbDown->setIcon(CompanionIcon("movedown.png"));
  qbDown->setShortcut(QKeySequence(tr("Ctrl+Down")));
  qbClear->setText(tr("Clear Mixes"));
  qbClear->setIcon(CompanionIcon("clear.png"));

  mixesLayout->addWidget(mixersListWidget,1,1,1,3);
  mixesLayout->addWidget(qbUp,2,1);
  mixesLayout->addWidget(qbClear,2,2);
  mixesLayout->addWidget(qbDown,2,3);

  connect(mixersListWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(mixerlistWidget_customContextMenuRequested(QPoint)));
  connect(mixersListWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(mixerlistWidget_doubleClicked(QModelIndex)));
  connect(mixersListWidget,SIGNAL(mimeDropped(int,const QMimeData*,Qt::DropAction)),this,SLOT(mimeMixerDropped(int,const QMimeData*,Qt::DropAction)));

  connect(qbUp,SIGNAL(pressed()),SLOT(moveMixUp()));
  connect(qbDown,SIGNAL(pressed()),SLOT(moveMixDown()));
  connect(qbClear,SIGNAL(pressed()),SLOT(clearMixes()));

  connect(mixersListWidget,SIGNAL(keyWasPressed(QKeyEvent*)), this, SLOT(mixerlistWidget_KeyPress(QKeyEvent*)));
}

MixesPanel::~MixesPanel()
{
}

void MixesPanel::update()
{
  lock = true;
  mixersListWidget->clear();
  for (int i=0; i < firmware->getCapability(Outputs); ++i) {
    bool filled = false;
    for (int j=0; j < firmware->getCapability(Mixes); ++j) {
      const MixData & mix = model->mixData[j];
      if ((int)mix.destCh == i+1) {
        AddMixerLine(j);
        filled = true;
      }
    }
    if (!filled) {
      AddMixerLine(-i-1);
    }
  }
  lock = false;
}

/**
  @brief Creates new mixer line (list item) and adds it to the list widget

  @note Mixer lines are now HTML formated in order to support bold text.

  @param[in] dest   defines which mixer line to create.
                    If dest < 0 then create empty channel slot fo channel -dest ( dest=-2 -> CH2)
                    if dest >=0 then create used channel based on model mix data from slot dest (dest=4 -> model mix[4])
*/
void MixesPanel::AddMixerLine(int dest)
{
  QByteArray qba(1, (quint8)dest);
  unsigned destId = abs(dest);
  bool newChan = false;
  bool hasSibs = false;
  if (dest >= 0) {
    //add mix data
    const MixData & md = model->mixData[dest];
    qba.append((const char*)&md, sizeof(MixData));
    destId = md.destCh;
    const QVector<const MixData *> mixes = model->mixes(md.destCh-1);
    newChan = (mixes.constFirst() == &md);
    hasSibs = (mixes.constLast() != &md);
  }
  QListWidgetItem *itm = new QListWidgetItem(getMixerText(dest, newChan));
  itm->setData(Qt::UserRole, qba);
  mixersListWidget->addItem(itm, destId, newChan, hasSibs);
}

/**
  @brief Returns HTML formated mixer representation

  @param[in] dest   defines which mixer line to create.
                    If dest < 0 then create empty channel slot for channel -dest ( dest=-2 -> CH2)
                    if dest >=0 then create used channel based on model mix data from slot dest (dest=4 -> model mix[4])

  @retval string    mixer line in HTML
*/
QString MixesPanel::getMixerText(int dest, bool newChannel)
{
  QString str;
  if (dest < 0) {
    dest = abs(dest);
    str = modelPrinter.printChannelName(dest-1);
    //highlight channel if needed
    if (dest == (int)highlightedSource)
      str = "<b>" + str + "</b>";
  }
  else {
    const MixData & mix = model->mixData[dest];
    //mix->destCh from 1 to 32
    str = modelPrinter.printChannelName(mix.destCh-1);

    if (!newChannel)
      str.fill(' ');
    else if (mix.destCh == highlightedSource)
      str = "<b>" + str + "</b>";

    str += modelPrinter.printMixerLine(mix, !newChannel, highlightedSource);
  }
  return str.replace(" ", "&nbsp;");
}

bool MixesPanel::gm_insertMix(int idx)
{
  if (idx < 0 || idx >= firmware->getCapability(Mixes) || model->mixData[firmware->getCapability(Mixes)-1].destCh > 0) {
    QMessageBox::information(this, "companion", tr("Not enough available mixers!"));
    return false;
  }

  int i = model->mixData[idx].destCh;
  memmove(&model->mixData[idx+1], &model->mixData[idx],
          (firmware->getCapability(Mixes)-(idx+1)) * sizeof(MixData) );
  memset(reinterpret_cast<void *>(&model->mixData[idx]), 0, sizeof(MixData));
  model->mixData[idx].srcRaw = RawSource(SOURCE_TYPE_NONE);
  model->mixData[idx].destCh = i;
  model->mixData[idx].weight = 100;
  return true;
}

void MixesPanel::gm_deleteMix(int index)
{
  memmove(&model->mixData[index], &model->mixData[index+1],
          (firmware->getCapability(Mixes)-(index+1)) * sizeof(MixData));
  memset(reinterpret_cast<void *>(&model->mixData[firmware->getCapability(Mixes)-1]), 0, sizeof(MixData));
}

void MixesPanel::gm_openMix(int index)
{
  if(index < 0 || index>=firmware->getCapability(Mixes)) return;

  MixData mixd(model->mixData[index]);

  MixerDialog *dlg = new MixerDialog(this, *model, &mixd, index, generalSettings, firmware, sharedItemModels);
  if(dlg->exec()) {
    model->mixData[index] = mixd;
    emit modified();
    updateItemModels();
    update();
  }
  else {
    if (mixInserted) {
      gm_deleteMix(index);
    }
    mixInserted = false;
    updateItemModels();
    update();
  }
  delete dlg;
}

int MixesPanel::getMixerIndex(unsigned int dch)
{
  for (int i=0; i < firmware->getCapability(Mixes); i++) {
    if (!model->mixData[i].destCh) {
      // we reached the end of used mixes
      return i;
    }
    if (model->mixData[i].destCh > dch) {
      return i;
    }
  }
  return -1;
}

void MixesPanel::mixerlistWidget_doubleClicked(QModelIndex index)
{
  int idx = mixersListWidget->item(index.row())->data(Qt::UserRole).toByteArray().at(0);
  if (idx<0) {
    int i = -idx;
    idx = getMixerIndex(i); //get mixer index to insert
    if (!gm_insertMix(idx)) return;
    model->mixData[idx].destCh = i;
    mixInserted = true;
  }
  else {
    mixInserted = false;
  }
  gm_openMix(idx);
}

void MixesPanel::mixersDeleteList(QList<int> list)
{
  std::sort(list.begin(), list.end());

  int iDec = 0;
  foreach(int idx, list) {
    gm_deleteMix(idx - iDec);
    iDec++;
  }
}

QList<int> MixesPanel::createMixListFromSelected()
{
  QList<int> list;
  foreach(QListWidgetItem *item, mixersListWidget->selectedItems()) {
    int idx= item->data(Qt::UserRole).toByteArray().at(0);
    if(idx >= 0 && idx<firmware->getCapability(Mixes)) list << idx;
  }
  return list;
}

// TODO duplicated code
void MixesPanel::setSelectedByMixList(QList<int> list)
{
  for(int i=0; i<mixersListWidget->count(); i++) {
    int t = mixersListWidget->item(i)->data(Qt::UserRole).toByteArray().at(0);
    if(list.contains(t)) {
      mixersListWidget->item(i)->setSelected(true);
    }
  }
}


void MixesPanel::mixersDelete(bool prompt)
{
  QList<int> list = createMixListFromSelected();
  if(list.isEmpty())
    return;

  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete selected Mix lines. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  mixersDeleteList(list);
  emit modified();
  updateItemModels();
  update();
}

void MixesPanel::mixersCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut selected Mix lines. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  mixersCopy();
  mixersDelete(false);
}

void MixesPanel::mixersCopy()
{
  QList<int> list = createMixListFromSelected();

  QByteArray mxData;
  foreach(int idx, list) {
    mxData.append((char*)&model->mixData[idx], sizeof(MixData));
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData("application/x-companion-mix", mxData);
  QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void MixesPanel::pasteMixerMimeData(const QMimeData * mimeData, int destIdx)
{
  if (mimeData->hasFormat("application/x-companion-mix")) {
    int idx; // mixer index
    int dch;

    if (destIdx < 0) {
      dch = -destIdx;
      idx = getMixerIndex(dch) - 1; //get mixer index to insert
    }
    else {
      idx = destIdx;
      dch = model->mixData[idx].destCh;
    }

    QByteArray mxData = mimeData->data("application/x-companion-mix");

    int i = 0;
    while (i < mxData.size()) {
      idx++;
      if (idx == firmware->getCapability(Mixes)) break;
      if (!gm_insertMix(idx)) break;

      MixData * md = &model->mixData[idx];
      memcpy(md, mxData.mid(i, sizeof(MixData)).constData(), sizeof(MixData));
      md->destCh = dch;
      i += sizeof(MixData);
    }

    emit modified();
    updateItemModels();
    update();
  }
}


void MixesPanel::mixersPaste()
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  QListWidgetItem * item = mixersListWidget->currentItem();
  if (item) {
    pasteMixerMimeData(mimeData, item->data(Qt::UserRole).toByteArray().at(0));
  }
}

void MixesPanel::mixersDuplicate()
{
  mixersCopy();
  mixersPaste();
}

void MixesPanel::mixerOpen()
{
  QListWidgetItem *item = mixersListWidget->currentItem();
  if (item == nullptr)
    return;

  int idx = item->data(Qt::UserRole).toByteArray().at(0);
  if(idx < 0) {
    int i = -idx;
    idx = getMixerIndex(i); //get mixer index to insert
    if (!gm_insertMix(idx)) return;
    model->mixData[idx].destCh = i;
    mixInserted = true;
  }
  else {
    mixInserted = false;
  }

  gm_openMix(idx);
}

void MixesPanel::mixerHighlight()
{
  QListWidgetItem *item = mixersListWidget->currentItem();
  if (item == nullptr)
    return;

  int idx = item->data(Qt::UserRole).toByteArray().at(0);
  int dest;
  if (idx<0) {
    dest = -idx;
  }
  else {
    dest = model->mixData[idx].destCh;
  }
  highlightedSource = ( (int)highlightedSource ==  dest) ? 0 : dest;
  // qDebug() << "MixesPanel::mixerHighlight(): " << highlightedSource ;
  update();
}

void MixesPanel::mixerAdd()
{
  if (!mixersListWidget->currentItem()) return;

  int index = mixersListWidget->currentItem()->data(Qt::UserRole).toByteArray().at(0);

  if(index < 0) {  // if empty then return relavent index
    int i = -index;
    index = getMixerIndex(i); //get mixer index to insert
    if (!gm_insertMix(index)) return;
    model->mixData[index].destCh = i;
    mixInserted = true;
  }
  else {
    index++;
    if (!gm_insertMix(index)) return;
    model->mixData[index].destCh = model->mixData[index-1].destCh;
    mixInserted = true;
  }
  gm_openMix(index);
}

void MixesPanel::mixerlistWidget_customContextMenuRequested(QPoint pos)
{
  QPoint globalPos = mixersListWidget->mapToGlobal(pos);

  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  bool hasData = mimeData->hasFormat("application/x-companion-mix");

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("add.png"),       tr("&Add"), this,              SLOT(mixerAdd()),        tr("Ctrl+A"));
  contextMenu.addAction(CompanionIcon("edit.png"),      tr("&Edit"), this,             SLOT(mixerOpen()),       tr("Enter"));
  contextMenu.addAction(CompanionIcon("fuses.png"),     tr("&Toggle highlight"), this, SLOT(mixerHighlight()),  tr("Ctrl+T"));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"),     tr("&Delete"), this,           SLOT(mixersDelete()),    tr("Delete"));
  contextMenu.addAction(CompanionIcon("copy.png"),      tr("&Copy"), this,             SLOT(mixersCopy()),      tr("Ctrl+C"));
  contextMenu.addAction(CompanionIcon("cut.png"),       tr("C&ut"), this,              SLOT(mixersCut()),       tr("Ctrl+X"));
  contextMenu.addAction(CompanionIcon("paste.png"),     tr("&Paste"), this,            SLOT(mixersPaste()),     tr("Ctrl+V"))->setEnabled(hasData);
  contextMenu.addAction(CompanionIcon("duplicate.png"), tr("Du&plicate"), this,        SLOT(mixersDuplicate()), tr("Ctrl+U"));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("moveup.png"),    tr("Move Up"), this,           SLOT(moveMixUp()),       tr("Ctrl+Up"));
  contextMenu.addAction(CompanionIcon("movedown.png"),  tr("Move Down"), this,         SLOT(moveMixDown()),     tr("Ctrl+Down"));
  contextMenu.addSeparator();
  contextMenu.addActions(mixersListWidget->actions());

  contextMenu.exec(globalPos);
}

void MixesPanel::mimeMixerDropped(int index, const QMimeData *data, Qt::DropAction action)
{
  int idx = mixersListWidget->item(index)->data(Qt::UserRole).toByteArray().at(0);
  //qDebug() << "MixesPanel::mimeMixerDropped()" << index << data;
  if (action == Qt::CopyAction) {
    pasteMixerMimeData(data, idx);
  }
  else if (action == Qt::MoveAction) {
    QList<int> list = createMixListFromSelected();
    mixersDeleteList(createMixListFromSelected());
    foreach (const int del, list) {
      if (del < idx)
        --idx;
    }
    pasteMixerMimeData(data, idx);
  }
}

void MixesPanel::mixerlistWidget_KeyPress(QKeyEvent *event)
{
  if(event->matches(QKeySequence::SelectAll)) mixerAdd();  //Ctrl A
  if(event->matches(QKeySequence::Delete))    mixersDelete();
  if(event->matches(QKeySequence::Copy))      mixersCopy();
  if(event->matches(QKeySequence::Cut))       mixersCut();
  if(event->matches(QKeySequence::Paste))     mixersPaste();
  if(event->matches(QKeySequence::Underline)) mixersDuplicate();
  if(event->matches(QKeySequence::AddTab))    mixerHighlight();

  if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) mixerOpen();

  if(event->matches(QKeySequence::MoveToNextLine)) {
    mixersListWidget->setCurrentRow(mixersListWidget->currentRow()+1);
  }
  if(event->matches(QKeySequence::MoveToPreviousLine)) {
    mixersListWidget->setCurrentRow(mixersListWidget->currentRow()-1);
  }
}

int MixesPanel::gm_moveMix(int idx, bool dir) //true=inc=down false=dec=up
{
  if(idx > firmware->getCapability(Mixes) || (idx == firmware->getCapability(Mixes) && dir)) return idx;

  MixData &src=model->mixData[idx];

  if (idx == 0 && !dir) {
    //special case: topmost mixer moving up
    if (src.destCh > 1) src.destCh--;
    return idx;
  }

  int tdx = dir ? idx+1 : idx-1;
  MixData & tgt = model->mixData[tdx];

  unsigned int outputs = firmware->getCapability(Outputs);
  if(src.destCh == 0 || src.destCh > outputs || tgt.destCh > outputs) return idx;

  if (tgt.destCh != src.destCh) {
    if (dir  && src.destCh < outputs) src.destCh++;
    if (!dir && src.destCh > 0)       src.destCh--;
    return idx;
  }

  //flip between idx and tgt
  MixData temp;
  memcpy(&temp, &src, sizeof(MixData));
  memcpy(&src, &tgt, sizeof(MixData));
  memcpy(&tgt, &temp, sizeof(MixData));
  return tdx;
}

void MixesPanel::moveMixUp()
{
  QList<int> list = createMixListFromSelected();
  QList<int> highlightList;
  foreach(int idx, list) {
    highlightList << gm_moveMix(idx, false);
  }
  emit modified();
  updateItemModels();
  update();
  setSelectedByMixList(highlightList);
}

void MixesPanel::moveMixDown()
{
  QList<int> list = createMixListFromSelected();
  QList<int> highlightList;
  foreach(int idx, list) {
    highlightList << gm_moveMix(idx, true);
  }
  emit modified();
  updateItemModels();
  update();
  setSelectedByMixList(highlightList);
}

void MixesPanel::clearMixes()
{
  if (QMessageBox::question(this, tr("Clear Mixes?"), tr("Really clear all the mixes?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    model->clearMixes();
    emit modified();
    updateItemModels();
    update();
  }
}

void MixesPanel::connectItemModelEvents(const int id)
{
  AbstractDynamicItemModel * itemModel = qobject_cast<AbstractDynamicItemModel *>(sharedItemModels->getItemModel(id));
  if (itemModel) {
    connect(itemModel, &AbstractDynamicItemModel::aboutToBeUpdated, this, &MixesPanel::onItemModelAboutToBeUpdated);
    connect(itemModel, &AbstractDynamicItemModel::updateComplete, this, &MixesPanel::onItemModelUpdateComplete);
  }
}

void MixesPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void MixesPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}

void MixesPanel::updateItemModels()
{
  lock = true;
  sharedItemModels->update(AbstractItemModel::IMUE_Channels);
}

