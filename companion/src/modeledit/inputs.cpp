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

#include "inputs.h"
#include "expodialog.h"
#include "helpers.h"
#include "filtereditemmodels.h"

InputsPanel::InputsPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  expoInserted(false),
  modelPrinter(firmware, generalSettings, model),
  sharedItemModels(sharedItemModels),
  modelsUpdateCnt(0)
{
  connectItemModelEvents(AbstractItemModel::IMID_RawSource);
  connectItemModelEvents(AbstractItemModel::IMID_RawSwitch);
  connectItemModelEvents(AbstractItemModel::IMID_Curve);
  connectItemModelEvents(AbstractItemModel::IMID_GVarRef);

  inputsCount = firmware->getCapability(VirtualInputs);
  if (inputsCount == 0)
    inputsCount = CPN_MAX_STICKS;

  QGridLayout * exposLayout = new QGridLayout(this);

  ExposlistWidget = new MixersListWidget(this, true);
  QPushButton * qbUp = new QPushButton(this);
  QPushButton * qbDown = new QPushButton(this);
  QPushButton * qbClear = new QPushButton(this);

  qbUp->setText(tr("Move Up"));
  qbUp->setIcon(CompanionIcon("moveup.png"));
  qbUp->setShortcut(QKeySequence(tr("Ctrl+Up")));
  qbDown->setText(tr("Move Down"));
  qbDown->setIcon(CompanionIcon("movedown.png"));
  qbDown->setShortcut(QKeySequence(tr("Ctrl+Down")));
  qbClear->setText(tr("Clear All Inputs"));
  qbClear->setIcon(CompanionIcon("clear.png"));

  exposLayout->addWidget(ExposlistWidget, 1, 1, 1, 3);
  exposLayout->addWidget(qbUp, 2, 1);
  exposLayout->addWidget(qbClear, 2, 2);
  exposLayout->addWidget(qbDown, 2, 3);

  connect(ExposlistWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(expolistWidget_customContextMenuRequested(QPoint)));
  connect(ExposlistWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(expolistWidget_doubleClicked(QModelIndex)));
  connect(ExposlistWidget, SIGNAL(mimeDropped(int, const QMimeData*,Qt::DropAction)), this, SLOT(mimeExpoDropped(int, const QMimeData*, Qt::DropAction)));

  connect(qbUp, SIGNAL(pressed()), SLOT(moveExpoUp()));
  connect(qbDown, SIGNAL(pressed()), SLOT(moveExpoDown()));
  connect(qbClear, SIGNAL(pressed()), SLOT(clearExpos()));

  connect(ExposlistWidget, SIGNAL(keyWasPressed(QKeyEvent*)), this, SLOT(expolistWidget_KeyPress(QKeyEvent*)));
}

InputsPanel::~InputsPanel()
{
}

void InputsPanel::update()
{
  lock = true;
  ExposlistWidget->clear();
  for (int i = 0; i < inputsCount; ++i) {
    bool filled = false;
    for (uint j = 0; j < CPN_MAX_EXPOS; ++j) {
      const ExpoData & ed = model->expoData[j];
      if ((int)ed.chn == i && ed.mode) {
        AddInputLine(j);
        filled = true;
      }
    }
    if (!filled) {
      AddInputLine(-i - 1);
    }
  }
  lock = false;
}


/**
  @brief Creates new input line (list item) and adds it to the list widget

  @note Input lines are now HTML formated because they use same widget as mixers.

  @param[in] dest   defines which input line to create.
                    If dest < 0 then create empty input slot for input -dest ( dest=-6 -> Input05)
                    if dest >=0 then create used input based on model input data from slot dest (dest=4 -> model expoData[4])
*/
void InputsPanel::AddInputLine(int dest)
{
  QByteArray qba(1, (quint8)dest);
  unsigned destId = abs(dest);
  bool newChan = false;
  bool hasSibs = false;
  if (dest >= 0 && dest < CPN_MAX_EXPOS) {
    //add input data
    const ExpoData &md = model->expoData[dest];
    qba.append((const char*)&md, sizeof(ExpoData));
    destId = md.chn + 1;
    newChan = model->isExpoParent(dest);
    hasSibs = (model->hasExpoChildren(dest) || model->hasExpoSiblings(dest));
  }
  QListWidgetItem *itm = new QListWidgetItem(getInputText(dest, newChan));
  itm->setData(Qt::UserRole, qba);
  ExposlistWidget->addItem(itm, destId, newChan, hasSibs);
}


/**
  @brief Returns HTML formated input representation

  @param dest   defines which input line to create.
                    If dest < 0 then create empty input slot for input -dest ( dest=-6 -> Input05)
                    if dest >=0 then create used input based on model input data from slot dest (dest=4 -> model expoData[4])

  @param newChan  If true, prints the full channel name in leading column, otherwise pads the column with spaces. Assumed true if @a dest < 0.

  @retval string    input line in HTML
*/
QString InputsPanel::getInputText(int dest, bool newChan)
{
  QString str;
  if (dest < 0) {
    str = modelPrinter.printInputName(-dest - 1);
  }
  else {
    const ExpoData &input = model->expoData[dest];
    const int nameChars = (firmware->getCapability(VirtualInputs) ? 10 : 4);
    if (newChan)
      str = QString("%1").arg(modelPrinter.printInputName(input.chn), -nameChars, QChar(' '));
    else
      str = QString(nameChars, QChar(' '));
    str.replace(" ", "&nbsp;");
    str += modelPrinter.printInputLine(input);
  }
  return str;
}

bool InputsPanel::gm_insertExpo(int idx)
{
  if (idx < 0 || idx >= CPN_MAX_EXPOS || model->expoData[CPN_MAX_EXPOS-1].mode > 0) {
    QMessageBox::information(this, CPN_STR_APP_NAME, tr("Not enough available Inputs!"));
    return false;
  }

  int chn = model->expoData[idx].chn;

  ExpoData *newExpo = model->insertInput(idx);
  newExpo->chn = chn;
  newExpo->weight = 100;
  newExpo->mode = INPUT_MODE_BOTH;

  return true;
}

void InputsPanel::gm_deleteExpo(int index, bool clearName)
{
  model->removeInput(index, clearName);
}

void InputsPanel::gm_openExpo(int index)
{
  if (index < 0 || index >= CPN_MAX_EXPOS)
    return;

  ExpoData ed(model->expoData[index]);

  QString inputName;
  if (firmware->getCapability(VirtualInputs))
    inputName = model->inputNames[ed.chn];

  ExpoDialog *dlg = new ExpoDialog(this, *model, &ed, generalSettings, firmware, inputName, sharedItemModels);
  if (dlg->exec())  {
    model->expoData[index] = ed;
    if (firmware->getCapability(VirtualInputs))
      strncpy(model->inputNames[ed.chn], inputName.toLatin1().data(), INPUT_NAME_LEN);
    update();
    updateItemModels();
    emit modified();
  }
  else {
    if (expoInserted) {
      gm_deleteExpo(index);
    }
    expoInserted=false;
    update();
  }
  delete dlg;
}

int InputsPanel::getExpoIndex(unsigned int dch)
{
  unsigned int i = 0;
  while (model->expoData[i].chn <= dch && model->expoData[i].mode && i < CPN_MAX_EXPOS)
    i++;
  if (i == CPN_MAX_EXPOS)
    return -1;
  return i;
}

QList<int> InputsPanel::createExpoListFromSelected()
{
  QList<int> list;
  foreach (QListWidgetItem *item, ExposlistWidget->selectedItems()) {
    int idx = item->data(Qt::UserRole).toByteArray().at(0);
    if (idx >= 0 && idx < CPN_MAX_EXPOS)
      list << idx;
  }
  return list;
}

void InputsPanel::setSelectedByExpoList(QList<int> list)
{
  for (int i = 0; i < ExposlistWidget->count(); i++) {
    int t = ExposlistWidget->item(i)->data(Qt::UserRole).toByteArray().at(0);
    if (list.contains(t))
      ExposlistWidget->item(i)->setSelected(true);
  }
}

void InputsPanel::exposDelete(bool prompt)
{
  QList<int> list = createExpoListFromSelected();
  if (list.isEmpty())
    return;

  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete selected Input lines. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  exposDeleteList(list, prompt);
  update();
  updateItemModels();
  emit modified();
}

void InputsPanel::exposCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut selected Input lines. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  exposCopy();
  exposDelete(false);
}

void InputsPanel::exposCopy()
{
  QList<int> list = createExpoListFromSelected();

  QByteArray exData;
  foreach (int idx, list) {
    exData.append((char*)&model->expoData[idx], sizeof(ExpoData));
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_EXPO, exData);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void InputsPanel::mimeExpoDropped(int index, const QMimeData *data, Qt::DropAction action)
{
  int idx = ExposlistWidget->item(index)->data(Qt::UserRole).toByteArray().at(0);
  if (action == Qt::CopyAction) {
    pasteExpoMimeData(data, idx);
  }
  else if (action == Qt::MoveAction) {
    QList<int> list = createExpoListFromSelected();
    exposDeleteList(list, false);
    foreach (const int del, list) {
      if (del < idx)
        --idx;
    }
    pasteExpoMimeData(data, idx);
  }
}

void InputsPanel::pasteExpoMimeData(const QMimeData * mimeData, int destIdx)
{
  if (mimeData->hasFormat(MIMETYPE_EXPO)) {
    int idx; // expo index
    int dch;

    if (destIdx < 0) {
      dch = -destIdx - 1;
      idx = getExpoIndex(dch) - 1; //get expo index to insert
    }
    else {
      idx = destIdx;
      dch = model->expoData[idx].chn;
    }

    QByteArray exData = mimeData->data(MIMETYPE_EXPO);

    int i = 0;
    while (i < exData.size()) {
      idx++;
      if (!gm_insertExpo(idx))
        break;
      ExpoData *ed = &model->expoData[idx];
      memcpy(ed, exData.mid(i, sizeof(ExpoData)).constData(), sizeof(ExpoData));
      const int oldChan = ed->chn;
      ed->chn = dch;
      maybeCopyInputName(oldChan, ed->chn);
      i += sizeof(ExpoData);
    }

    update();
    updateItemModels();
    emit modified();
  }
}

void InputsPanel::exposPaste()
{
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  QListWidgetItem *item = ExposlistWidget->currentItem();
  if (item) {
    pasteExpoMimeData(mimeData, item->data(Qt::UserRole).toByteArray().at(0));
  }
}

void InputsPanel::exposDuplicate()
{
  exposCopy();
  exposPaste();
}

void InputsPanel::expoOpen(QListWidgetItem *item)
{
  if (!item)
    item = ExposlistWidget->currentItem();

  if (item == nullptr)
    return;

  int idx = item->data(Qt::UserRole).toByteArray().at(0);
  if (idx < 0) {
    int ch = -idx - 1;
    idx = getExpoIndex(ch); // get expo index to insert
    if (!gm_insertExpo(idx))
      return;
    model->expoData[idx].chn = ch;
    expoInserted = true;
  }
  else {
    expoInserted = false;
  }
  gm_openExpo(idx);
}

void InputsPanel::expoAdd()
{
  QListWidgetItem *item = ExposlistWidget->currentItem();
  if (item == nullptr)
    return;

  int index = item->data(Qt::UserRole).toByteArray().at(0);

  if (index < 0) {  // if empty then return relevant index
    expoOpen();
  }
  else {
    index++;
    if (!gm_insertExpo(index))
      return;
    model->expoData[index].chn = model->expoData[index-1].chn;
    expoInserted = true;
  }
  gm_openExpo(index);
}

void InputsPanel::expolistWidget_customContextMenuRequested(QPoint pos)
{
  QPoint globalPos = ExposlistWidget->mapToGlobal(pos);

  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  bool hasData = mimeData->hasFormat(MIMETYPE_EXPO);

  selectedIdx = getIndexFromSelected();
  inputIdx = getInputIndexFromSelected();

  QMenu contextMenu;
  QMenu *contextMenuLines = contextMenu.addMenu(tr("Lines"));
  contextMenuLines->addAction(CompanionIcon("add.png"), tr("&Add"), this, SLOT(expoAdd()), tr("Ctrl+A"));
  contextMenuLines->addAction(CompanionIcon("edit.png"), tr("&Edit"), this, SLOT(expoOpen()), tr("Enter"));
  contextMenuLines->addSeparator();
  contextMenuLines->addAction(CompanionIcon("clear.png"), tr("&Delete"), this, SLOT(exposDelete()), tr("Delete"));
  contextMenuLines->addAction(CompanionIcon("copy.png"), tr("&Copy"), this, SLOT(exposCopy()), tr("Ctrl+C"));
  contextMenuLines->addAction(CompanionIcon("cut.png"), tr("&Cut"), this, SLOT(exposCut()), tr("Ctrl+X"));
  contextMenuLines->addAction(CompanionIcon("paste.png"), tr("&Paste"), this, SLOT(exposPaste()), tr("Ctrl+V"))->setEnabled(hasData);
  contextMenuLines->addAction(CompanionIcon("duplicate.png"), tr("Du&plicate"), this, SLOT(exposDuplicate()), tr("Ctrl+U"));
  contextMenuLines->addSeparator();
  contextMenuLines->addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(moveExpoUp()), tr("Ctrl+Up"));
  contextMenuLines->addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(moveExpoDown()), tr("Ctrl+Down"));

  QMenu *contextMenuInputs = contextMenu.addMenu(tr("Input"));
  contextMenuInputs->addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this, SLOT(cmInputInsert()))->setEnabled(cmInputInsertAllowed());
  contextMenuInputs->addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this, SLOT(cmInputDelete()));
  contextMenuInputs->addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(cmInputMoveUp()))->setEnabled(cmInputMoveUpAllowed());
  contextMenuInputs->addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(cmInputMoveDown()))->setEnabled(cmInputMoveDownAllowed());
  contextMenuInputs->addSeparator();
  contextMenuInputs->addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(cmInputClear()))->setEnabled(isExpoIndex(selectedIdx));

  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(clearExpos()));
  contextMenu.addSeparator();
  contextMenu.addActions(ExposlistWidget->actions());
  contextMenu.exec(globalPos);
}

void InputsPanel::expolistWidget_KeyPress(QKeyEvent *event)
{
  if (event->matches(QKeySequence::SelectAll))
    expoAdd();  //Ctrl A
  if (event->matches(QKeySequence::Delete))
    exposDelete();
  if (event->matches(QKeySequence::Copy))
    exposCopy();
  if (event->matches(QKeySequence::Cut))
    exposCut();
  if (event->matches(QKeySequence::Paste))
    exposPaste();
  if (event->matches(QKeySequence::Underline))
    exposDuplicate();

  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    expoOpen();
  if (event->matches(QKeySequence::MoveToNextLine))
    ExposlistWidget->setCurrentRow(ExposlistWidget->currentRow() + 1);
  if (event->matches(QKeySequence::MoveToPreviousLine))
    ExposlistWidget->setCurrentRow(ExposlistWidget->currentRow() - 1);
}

int InputsPanel::gm_moveExpo(int idx, bool dir) //true=inc=down false=dec=up
{
  if (idx >= CPN_MAX_EXPOS || idx < 0 || (dir && idx >= CPN_MAX_EXPOS - 1) || (!dir && !idx))
    return -1;

  const int tdx = dir ? idx + 1 : idx-1;
  ExpoData temp;
  ExpoData &src = model->expoData[idx];
  ExpoData &tgt = model->expoData[tdx];

  if (memcmp(&src, &temp, sizeof(ExpoData)) == 0) {
    // src expo is empty
    return -1;
  }

  bool tgtempty = (memcmp(&tgt, &temp, sizeof(ExpoData)) == 0);

  if (tgt.chn != src.chn || tgtempty) {
    const int oldChan = src.chn;
    if (dir && src.chn < unsigned(inputsCount - 1))
      src.chn++;
    else if (!dir && src.chn > 0)
      src.chn--;
    maybeCopyInputName(oldChan, src.chn);
    return idx;
  }

  // flip between src and tgt
  memcpy(&temp, &src, sizeof(ExpoData));
  memcpy(&src, &tgt, sizeof(ExpoData));
  memcpy(&tgt, &temp, sizeof(ExpoData));
  return tdx;
}

void InputsPanel::moveExpoList(bool down)
{
  QList<int> list = createExpoListFromSelected();
  QList<int> highlightList;
  bool mod = false;
  foreach (int idx, list) {
    const int newIdx = gm_moveExpo(idx, down);
    if (newIdx > -1) {
      highlightList << newIdx;
      mod = true;
    }
  }
  if (mod) {
    update();
    updateItemModels();
    emit modified();
  }
  setSelectedByExpoList(highlightList);
}

void InputsPanel::moveExpoUp()
{
  moveExpoList(false);
}

void InputsPanel::moveExpoDown()
{
  moveExpoList(true);
}

void InputsPanel::expolistWidget_doubleClicked(QModelIndex index)
{
  expoOpen(ExposlistWidget->item(index.row()));
}

void InputsPanel::exposDeleteList(QList<int> list, bool clearName)
{
  std::sort(list.begin(), list.end());

  int iDec = 0;
  foreach (int idx, list) {
    gm_deleteExpo(idx-iDec, clearName);
    iDec++;
  }
}

void InputsPanel::clearExpos()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Input lines. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    model->clearInputs();
    for (int i = 0; i < inputsCount; i++) {
      model->updateAllReferences(ModelData::REF_UPD_TYPE_INPUT, ModelData::REF_UPD_ACT_CLEAR, i);
    }
    update();
    updateItemModels();
    emit modified();
  }
}

void InputsPanel::maybeCopyInputName(int srcChan, int destChan)
{
  if (srcChan == destChan)
    return;

  // check to see if source input channel is now empty
  const bool srcEmpty = (model->expos(srcChan).size() == 0);

  if (srcEmpty) {
    // if destination input name is empty, copy it from source expo input
    if (!strlen(model->inputNames[destChan]))
      strncpy(model->inputNames[destChan], model->inputNames[srcChan], INPUT_NAME_LEN);
    // clear the emptry source channel name
    model->inputNames[srcChan][0] = 0;
  }
}

bool InputsPanel::cmInputInsertAllowed() const
{
  return ((!model->hasExpos(inputsCount - 1)) && (inputIdx < inputsCount - 1));
}

bool InputsPanel::cmInputMoveDownAllowed() const
{
  return (inputIdx >= 0 && (inputIdx < inputsCount - 1));
}

bool InputsPanel::cmInputMoveUpAllowed() const
{
  return inputIdx > 0;
}

void InputsPanel::cmInputClear()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all lines for the selected Input. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = CPN_MAX_EXPOS - 1; i >= 0; i--) {
    ExpoData *ed = &model->expoData[i];
    if (!ed->isEmpty()) {
      if ((int)ed->chn == inputIdx)
        model->removeInput(i);
    }
  }
  model->updateAllReferences(ModelData::REF_UPD_TYPE_INPUT, ModelData::REF_UPD_ACT_CLEAR, inputIdx);
  update();
  updateItemModels();
  emit modified();
}

void InputsPanel::cmInputDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete all lines for the selected Input. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = CPN_MAX_EXPOS - 1; i >= 0; i--) {
    ExpoData *ed = &model->expoData[i];
    if (!ed->isEmpty()) {
      if ((int)ed->chn == inputIdx)
        model->removeInput(i);
      else if ((int)ed->chn > inputIdx)
        ed->chn--;
    }
  }

  for (int i = inputIdx; i < inputsCount; i++) {
    strncpy(model->inputNames[i], model->inputNames[i + 1], sizeof(model->inputNames[i]) - 1);
  }
  model->inputNames[inputsCount - 1][0] = 0;

  model->updateAllReferences(ModelData::REF_UPD_TYPE_INPUT, ModelData::REF_UPD_ACT_SHIFT, inputIdx, 0, -1);
  update();
  updateItemModels();
  emit modified();
}

void InputsPanel::cmInputInsert()
{
  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    ExpoData *ed = &model->expoData[i];
    if ((int)ed->chn >= inputIdx)
      ed->chn++;
  }

  for (int i = inputsCount - 1; i > inputIdx; i--) {
    strncpy(model->inputNames[i], model->inputNames[i - 1], sizeof(model->inputNames[i]) - 1);
  }
  model->inputNames[inputIdx][0] = 0;

  model->updateAllReferences(ModelData::REF_UPD_TYPE_INPUT, ModelData::REF_UPD_ACT_SHIFT, inputIdx, 0, 1);
  update();
  updateItemModels();
  emit modified();
}

void InputsPanel::cmInputMoveDown()
{
  cmInputSwapData(inputIdx, inputIdx + 1);
}

void InputsPanel::cmInputMoveUp()
{
  cmInputSwapData(inputIdx - 1, inputIdx);
}

void InputsPanel::cmInputSwapData(int idx1, int idx2)
{
  if (idx1 >= idx2 || (!model->hasExpos(idx1) && !model->hasExpos(idx2)))
    return;
  //  save expos
  int expoidx = -1;
  QVector<ExpoData> edtmp;
  int i;
  for (i = 0; i < CPN_MAX_EXPOS; i++) {
    ExpoData *ed = &model->expoData[i];
    if ((int)ed->chn == idx1) {
      edtmp << model->expoData[i];
      if (expoidx < 0)
        expoidx = i;
    }
    else if ((int)ed->chn > idx1)
      break;
  }
  //  move expos up
  const int offset = i - expoidx;
  int expocnt = 0;
  for (int j = i; j < CPN_MAX_EXPOS; j++) {
    ExpoData *ed = &model->expoData[j];
    if ((int)ed->chn == idx2) {
      ExpoData *dest = &model->expoData[j - offset];
      memcpy(dest, &model->expoData[j], sizeof(ExpoData));
      dest->chn = idx1;
      expocnt++;
    }
    else if ((int)ed->chn > idx2)
      break;
  }
  //  copy back saved expos
  int cnt = 0;
  foreach (ExpoData ed, edtmp) {
    ExpoData *dest = &model->expoData[expoidx + expocnt + cnt];
    memcpy(dest, &ed, sizeof(ExpoData));
    dest->chn = idx2;
    cnt++;
  }

  //  swap names
  QByteArray *tname = new QByteArray(model->inputNames[idx2]);
  strncpy(model->inputNames[idx2], model->inputNames[idx1], sizeof(model->inputNames[idx2]) - 1);
  strncpy(model->inputNames[idx1], tname->data(), sizeof(model->inputNames[idx1]) - 1);

  model->updateAllReferences(ModelData::REF_UPD_TYPE_INPUT, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
  update();
  updateItemModels();
  emit modified();
}

bool InputsPanel::isInputIndex(const int index)
{
  if (index < 0)
    return true;
  else
    return false;
}

bool InputsPanel::isExpoIndex(const int index)
{
  return !isInputIndex(index);
}

int InputsPanel::getIndexFromSelected()
{
  return ExposlistWidget->currentItem()->data(Qt::UserRole).toByteArray().at(0);
}

int InputsPanel::getInputIndexFromSelected()
{
  const int selidx = getIndexFromSelected();
  int idx;

  if (isInputIndex(selidx))
    idx = -selidx - 1;
  else {
    const ExpoData *ed = &model->expoData[selidx];
    idx = ed->chn;
  }
  return idx;
}

void InputsPanel::connectItemModelEvents(const int id)
{
  AbstractDynamicItemModel * itemModel = qobject_cast<AbstractDynamicItemModel *>(sharedItemModels->getItemModel(id));
  if (itemModel) {
    connect(itemModel, &AbstractDynamicItemModel::aboutToBeUpdated, this, &InputsPanel::onItemModelAboutToBeUpdated);
    connect(itemModel, &AbstractDynamicItemModel::updateComplete, this, &InputsPanel::onItemModelUpdateComplete);
  }
}

void InputsPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void InputsPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}

void InputsPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Inputs);
}
