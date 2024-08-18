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
#include "filtereditemmodels.h"
#include "helpers.h"

#include <TimerEdit>

LogicalSwitchesPanel::LogicalSwitchesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                                           CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels),
  selectedIndex(0),
  modelsUpdateCnt(0)
{
  rawSwitchFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                 RawSwitch::LogicalSwitchesContext);
  connectItemModelEvents(rawSwitchFilteredModel);

  const int srcGroups = firmware->getCapability(GvarsInCS) ? 0 : (RawSource::AllSourceGroups & ~RawSource::GVarsGroup);
  rawSourceFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource), srcGroups);
  connectItemModelEvents(rawSourceFilteredModel);

  lsCapability = firmware->getCapability(LogicalSwitches);
  lsCapabilityExt = firmware->getCapability(LogicalSwitchesExt);

  QStringList headerLabels;
  headerLabels << "#" << tr("Function") << tr("V1") << tr("V2") << tr("AND Switch");
  if (lsCapabilityExt) {
    headerLabels << tr("Duration") << tr("Delay");
  }
  headerLabels << tr("Persistent");
  TableLayout * tableLayout = new TableLayout(this, lsCapability, headerLabels);

  const int channelsMax = model.getChannelsMax(true);

  for (int i = 0; i < lsCapability; i++) {
    // The label
    QLabel * label = new QLabel(this);
    label->setProperty("index", i);
    label->setText(RawSwitch(SWITCH_TYPE_VIRTUAL, i + 1).toString());
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    label->setToolTip(tr("Popup menu available"));
    label->setMouseTracking(true);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(label, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
    tableLayout->addWidget(i, 0, label);

    // The function
    cbFunction[i] = new QComboBox(this);
    cbFunction[i]->setProperty("index", i);
    populateFunctionCB(cbFunction[i]);
    connect(cbFunction[i], SIGNAL(currentIndexChanged(int)), this, SLOT(onFunctionChanged()));
    tableLayout->addWidget(i, 1, cbFunction[i]);

    // V1
    QHBoxLayout *v1Layout = new QHBoxLayout();
    cbSource1[i] = new QComboBox(this);
    cbSource1[i]->setProperty("index",i);
    connect(cbSource1[i], SIGNAL(currentIndexChanged(int)), this, SLOT(onV1Changed(int)));
    v1Layout->addWidget(cbSource1[i]);
    cbSource1[i]->setVisible(false);
    dsbValue[i] = new QDoubleSpinBox(this);
    dsbValue[i]->setMaximum(channelsMax);
    dsbValue[i]->setMinimum(-channelsMax);
    dsbValue[i]->setAccelerated(true);
    dsbValue[i]->setDecimals(0);
    dsbValue[i]->setProperty("index", i);
    connect(dsbValue[i], SIGNAL(editingFinished()), this, SLOT(onOffsetChanged()));
    v1Layout->addWidget(dsbValue[i]);
    dsbValue[i]->setVisible(false);
    tableLayout->addLayout(i, 2, v1Layout);

    // V2
    QHBoxLayout *v2Layout = new QHBoxLayout();
    cbSource2[i] = new QComboBox(this);
    cbSource2[i]->setProperty("index", i);
    connect(cbSource2[i], SIGNAL(currentIndexChanged(int)), this, SLOT(onV2Changed(int)));
    v2Layout->addWidget(cbSource2[i]);
    cbSource2[i]->setVisible(false);
    dsbOffset[i] = new QDoubleSpinBox(this);
    dsbOffset[i]->setProperty("index",i);
    dsbOffset[i]->setMaximum(channelsMax);
    dsbOffset[i]->setMinimum(-channelsMax);
    dsbOffset[i]->setAccelerated(true);
    dsbOffset[i]->setDecimals(0);
    connect(dsbOffset[i], SIGNAL(valueChanged(double)), this, SLOT(onOffsetChanged()));
    dsbOffset[i]->setVisible(false);
    v2Layout->addWidget(dsbOffset[i]);
    dsbOffset2[i] = new QDoubleSpinBox(this);
    dsbOffset2[i]->setProperty("index",i);
    dsbOffset2[i]->setMaximum(channelsMax);
    dsbOffset2[i]->setMinimum(-channelsMax);
    dsbOffset2[i]->setAccelerated(true);
    dsbOffset2[i]->setDecimals(0);
    dsbOffset2[i]->setSpecialValueText(" " + tr("(instant)"));
    connect(dsbOffset2[i], SIGNAL(valueChanged(double)), this, SLOT(onOffsetChanged()));
    dsbOffset2[i]->setVisible(false);
    v2Layout->addWidget(dsbOffset2[i]);
    teOffset[i] = new TimerEdit(this);
    teOffset[i]->setProperty("index",i);
    connect(teOffset[i],SIGNAL(editingFinished()),this,SLOT(onOffsetChanged()));
    v2Layout->addWidget(teOffset[i]);
    teOffset[i]->setVisible(false);
    tableLayout->addLayout(i, 3, v2Layout);

    // AND
    cbAndSwitch[i] = new QComboBox(this);
    cbAndSwitch[i]->setProperty("index", i);
    cbAndSwitch[i]->setModel(rawSwitchFilteredModel);
    cbAndSwitch[i]->setVisible(true);
    connect(cbAndSwitch[i], SIGNAL(currentIndexChanged(int)), this, SLOT(onAndSwitchChanged(int)));
    tableLayout->addWidget(i, 4, cbAndSwitch[i]);

    if (lsCapabilityExt) {
      // Duration
      dsbDuration[i] = new QDoubleSpinBox(this);
      dsbDuration[i]->setProperty("index", i);
      dsbDuration[i]->setSingleStep(0.1);
      dsbDuration[i]->setMaximum(25);
      dsbDuration[i]->setMinimum(0);
      dsbDuration[i]->setAccelerated(true);
      dsbDuration[i]->setDecimals(1);
      connect(dsbDuration[i], SIGNAL(valueChanged(double)), this, SLOT(onDurationChanged(double)));
      tableLayout->addWidget(i, 5, dsbDuration[i]);

      // Delay
      dsbDelay[i] = new QDoubleSpinBox(this);
      dsbDelay[i]->setProperty("index", i);
      dsbDelay[i]->setSingleStep(0.1);
      dsbDelay[i]->setMaximum(25);
      dsbDelay[i]->setMinimum(0);
      dsbDelay[i]->setAccelerated(true);
      dsbDelay[i]->setDecimals(1);
      connect(dsbDelay[i], SIGNAL(valueChanged(double)), this, SLOT(onDelayChanged(double)));
      tableLayout->addWidget(i, 6, dsbDelay[i]);
    }

    cbPersist[i] = new QCheckBox(this);
    cbPersist[i]->setProperty("index", i);
    cbPersist[i]->setText(tr(""));
    cbPersist[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(cbPersist[i], SIGNAL(stateChanged(int)), this, SLOT(onPersistChanged()));
    tableLayout->addWidget(i, 7, cbPersist[i]);
  }

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->pushRowsUp(lsCapability+1);
}

LogicalSwitchesPanel::~LogicalSwitchesPanel()
{
  delete rawSourceFilteredModel;
  delete rawSwitchFilteredModel;
}

void LogicalSwitchesPanel::onFunctionChanged()
{
  if (!lock) {
    int i = sender()->property("index").toInt();
    unsigned newFunc = cbFunction[i]->currentData().toUInt();

    if (model->logicalSw[i].func == newFunc)
      return;

    unsigned oldFunc = model->logicalSw[i].func;
    CSFunctionFamily oldFuncFamily = model->logicalSw[i].getFunctionFamily();
    model->logicalSw[i].func = newFunc;
    CSFunctionFamily newFuncFamily = model->logicalSw[i].getFunctionFamily();

    if (oldFuncFamily != newFuncFamily || newFunc == LS_FN_OFF) {
      model->logicalSw[i].clear();
      model->logicalSw[i].func = newFunc;
      if (newFuncFamily == LS_FAMILY_TIMER) {
        model->logicalSw[i].val1 = -119;
        model->logicalSw[i].val2 = -119;
      }
      else if (newFuncFamily == LS_FAMILY_EDGE) {
        model->logicalSw[i].val2 = -129;
      }
    }

    updateLine(i);

    if (oldFunc == LS_FN_OFF || newFunc == LS_FN_OFF)
      updateItemModels();

    emit modified();
  }
}

void LogicalSwitchesPanel::onV1Changed(int value)
{
  if (!lock) {
    int i = sender()->property("index").toInt();
    if (model->logicalSw[i].val1 != cbSource1[i]->itemData(value).toInt()) {
      model->logicalSw[i].val1 = cbSource1[i]->itemData(value).toInt();
      if (model->logicalSw[i].getFunctionFamily() == LS_FAMILY_VOFS) {
        if (!offsetChangedAt(i))
          updateLine(i);
      }
      else {
        emit modified();
      }
    }
  }
}

void LogicalSwitchesPanel::onV2Changed(int value)
{
  if (!lock) {
    int i = sender()->property("index").toInt();
    if (model->logicalSw[i].val2 != cbSource2[i]->itemData(value).toInt()) {
      model->logicalSw[i].val2 = cbSource2[i]->itemData(value).toInt();
      emit modified();
    }
  }
}

void LogicalSwitchesPanel::onAndSwitchChanged(int value)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    if (model->logicalSw[index].andsw != cbAndSwitch[index]->itemData(value).toInt()) {
      model->logicalSw[index].andsw = cbAndSwitch[index]->itemData(value).toInt();
      emit modified();
    }
  }
}

void LogicalSwitchesPanel::onDurationChanged(double duration)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    model->logicalSw[index].duration = (uint8_t)round(duration * 10);
    emit modified();
  }
}

void LogicalSwitchesPanel::onDelayChanged(double delay)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    model->logicalSw[index].delay = (uint8_t)round(delay * 10);
    emit modified();
  }
}

void LogicalSwitchesPanel::onPersistChanged()
{
  if (!lock) {
    QCheckBox *ckb = qobject_cast<QCheckBox*>(sender());
    int index = ckb->property("index").toInt();
    model->logicalSw[index].lsPersist = (ckb->checkState() ? 1 : 0);
    emit modified();
  }
}

void LogicalSwitchesPanel::onOffsetChanged()
{
  offsetChangedAt(sender()->property("index").toInt());
}

bool LogicalSwitchesPanel::offsetChangedAt(int index)
{
  if (lock)
    return false;

  lock = true;
  bool mod = false;
  int value;

  switch (model->logicalSw[index].getFunctionFamily())
  {
    case LS_FAMILY_VOFS:
    {
      RawSource source = RawSource(model->logicalSw[index].val1);
      RawSourceRange range = source.getRange(model, generalSettings, model->logicalSw[index].getRangeFlags());
      double currVal = source.type == SOURCE_TYPE_TIMER ? teOffset[index]->timeInSeconds() : dsbOffset[index]->value();
      value = round((currVal - range.offset) / range.step);
      mod = (mod || value != model->logicalSw[index].val2);
      model->logicalSw[index].val2 = value;
      break;
    }

    case LS_FAMILY_TIMER:
      value = TimToVal(dsbValue[index]->value());
      mod = (mod || value != model->logicalSw[index].val1);
      model->logicalSw[index].val1 = value;
      value = TimToVal(dsbOffset[index]->value());
      mod = (mod || value != model->logicalSw[index].val2);
      model->logicalSw[index].val2 = value;
      break;

    case LS_FAMILY_EDGE:
      if (sender() == dsbOffset[index]) {
        value = TimToVal(dsbOffset[index]->value());
        mod = (mod || value != model->logicalSw[index].val2);
        model->logicalSw[index].val2 = value;
      }
      else {
        value = TimToVal(dsbOffset2[index]->value()) - model->logicalSw[index].val2;
        mod = (mod || value != model->logicalSw[index].val3);
        model->logicalSw[index].val3 = value;
      }
      break;

    default:
      break;
  }
  lock = false;

  if (mod) {
    updateLine(index);
    emit modified();
  }
  return mod;
}

void LogicalSwitchesPanel::updateTimerParam(QDoubleSpinBox *sb, int timer, double minimum, double maximum)
{
  sb->setVisible(true);
  sb->setDecimals(1);
  sb->setMinimum(minimum);
  sb->setMaximum(maximum);
  float value = ValToTim(timer);
  if (value >= 60)
    sb->setSingleStep(1);
  else if (value >= 2)
    sb->setSingleStep(0.5);
  else
    sb->setSingleStep(0.1);
  sb->setValue(value);
}

#define SOURCE1_VISIBLE  0x1
#define SOURCE2_VISIBLE  0x2
#define VALUE1_VISIBLE   0x4
#define VALUE2_VISIBLE   0x8
#define VALUE3_VISIBLE   0x10
#define VALUE_TO_VISIBLE 0x20
#define DELAY_ENABLED    0x40
#define DURATION_ENABLED 0x80
#define LINE_ENABLED     0x100
#define PERSIST_ENABLED  0x200

void LogicalSwitchesPanel::updateLine(int i)
{
  const bool savelock = lock;
  lock = true;
  unsigned int mask = 0;

  cbFunction[i]->setCurrentIndex(cbFunction[i]->findData(model->logicalSw[i].func));
  cbAndSwitch[i]->setCurrentIndex(cbAndSwitch[i]->findData(RawSwitch(model->logicalSw[i].andsw).toValue()));
  dsbOffset[i]->setSuffix("");

  if (!model->logicalSw[i].isEmpty()) {
    mask = LINE_ENABLED | DELAY_ENABLED | DURATION_ENABLED;

    switch (model->logicalSw[i].getFunctionFamily())
    {
      case LS_FAMILY_VOFS:
      {
        mask |= SOURCE1_VISIBLE;
        RawSource source = RawSource(model->logicalSw[i].val1);
        RawSourceRange range = source.getRange(model, generalSettings, model->logicalSw[i].getRangeFlags());
        double value = range.step * model->logicalSw[i].val2 + range.offset;  /* TODO+source.getRawOffset(model)*/
        cbSource1[i]->setModel(rawSourceFilteredModel);
        cbSource1[i]->setCurrentIndex(cbSource1[i]->findData(source.toValue()));
        if (source.type == SOURCE_TYPE_TIMER) {
          mask |= VALUE_TO_VISIBLE;
          teOffset[i]->setTimeRange(range.min, range.max);
          teOffset[i]->setSingleStep(range.step);
          teOffset[i]->setPageStep(range.step * 60);
          teOffset[i]->setShowSeconds(range.step != 60);
          teOffset[i]->setTime((int)value);
        }
        else {
          mask |= VALUE2_VISIBLE;
          if (!range.unit.isEmpty())
            dsbOffset[i]->setSuffix(" " + range.unit);
          dsbOffset[i]->setDecimals(range.decimals);
          dsbOffset[i]->setMinimum(range.min);
          dsbOffset[i]->setMaximum(range.max);
          dsbOffset[i]->setSingleStep(range.step);
          dsbOffset[i]->setValue(value);
        }

        break;
      }

      case LS_FAMILY_STICKY:  // no break
        mask |= PERSIST_ENABLED;
        cbPersist[i]->setChecked(model->logicalSw[i].lsPersist);
      case LS_FAMILY_VBOOL:
        mask |= SOURCE1_VISIBLE | SOURCE2_VISIBLE;
        cbSource1[i]->setModel(rawSwitchFilteredModel);
        cbSource1[i]->setCurrentIndex(cbSource1[i]->findData(model->logicalSw[i].val1));
        cbSource2[i]->setModel(rawSwitchFilteredModel);
        cbSource2[i]->setCurrentIndex(cbSource2[i]->findData(model->logicalSw[i].val2));
        break;

      case LS_FAMILY_EDGE:
        mask |= SOURCE1_VISIBLE | VALUE2_VISIBLE | VALUE3_VISIBLE;
        mask &= ~DELAY_ENABLED;
        cbSource1[i]->setModel(rawSwitchFilteredModel);
        cbSource1[i]->setCurrentIndex(cbSource1[i]->findData(model->logicalSw[i].val1));
        updateTimerParam(dsbOffset[i], model->logicalSw[i].val2, 0.0);
        updateTimerParam(dsbOffset2[i], model->logicalSw[i].val2 + model->logicalSw[i].val3, ValToTim(TimToVal(dsbOffset[i]->value()) - 1), 275.0);
        dsbOffset2[i]->setSuffix((model->logicalSw[i].val3) ? "" : tr(" (infinite)"));
        break;

      case LS_FAMILY_VCOMP:
        mask |= SOURCE1_VISIBLE | SOURCE2_VISIBLE;
        cbSource1[i]->setModel(rawSourceFilteredModel);
        cbSource1[i]->setCurrentIndex(cbSource1[i]->findData(model->logicalSw[i].val1));
        cbSource2[i]->setModel(rawSourceFilteredModel);
        cbSource2[i]->setCurrentIndex(cbSource2[i]->findData(model->logicalSw[i].val2));
        break;

      case LS_FAMILY_TIMER:
        mask |= VALUE1_VISIBLE | VALUE2_VISIBLE;
        updateTimerParam(dsbValue[i], model->logicalSw[i].val1, 0.1);
        updateTimerParam(dsbOffset[i], model->logicalSw[i].val2, 0.1);
        break;
    }
  }

  cbSource1[i]->setVisible(mask & SOURCE1_VISIBLE);
  cbSource2[i]->setVisible(mask & SOURCE2_VISIBLE);
  dsbValue[i]->setVisible(mask & VALUE1_VISIBLE);
  dsbOffset[i]->setVisible(mask & VALUE2_VISIBLE);
  dsbOffset2[i]->setVisible(mask & VALUE3_VISIBLE);
  teOffset[i]->setVisible(mask & VALUE_TO_VISIBLE);
  cbAndSwitch[i]->setVisible(mask & LINE_ENABLED);
  cbPersist[i]->setVisible(mask & PERSIST_ENABLED);
  if (lsCapabilityExt) {
    dsbDuration[i]->setVisible(mask & DURATION_ENABLED);
    dsbDelay[i]->setVisible(mask & DELAY_ENABLED);
    if (mask & DURATION_ENABLED)
      dsbDuration[i]->setValue(model->logicalSw[i].duration / 10.0);
    if (mask & DELAY_ENABLED)
      dsbDelay[i]->setValue(model->logicalSw[i].delay / 10.0);
  }

  lock = savelock;
}

void LogicalSwitchesPanel::populateFunctionCB(QComboBox *b)
{
  int order[] = {
    LS_FN_OFF,
    LS_FN_VEQUAL, // added at the end to avoid everything renumbered
    LS_FN_VALMOSTEQUAL, // added at the end to avoid everything renumbered
    LS_FN_VPOS,
    LS_FN_VNEG,
    // LS_FN_RANGE,
    LS_FN_APOS,
    LS_FN_ANEG,
    LS_FN_AND,
    LS_FN_OR,
    LS_FN_XOR,
    LS_FN_EDGE,
    LS_FN_EQUAL,
    LS_FN_NEQUAL,
    LS_FN_GREATER,
    LS_FN_LESS,
    LS_FN_EGREATER,
    LS_FN_ELESS,
    LS_FN_DPOS,
    LS_FN_DAPOS,
    LS_FN_TIMER,
    LS_FN_STICKY
  };

  b->clear();
  for (int i = 0; i < LS_FN_MAX; i++) {
    int func = order[i];
    if (func == LS_FN_NEQUAL || func == LS_FN_EGREATER || func == LS_FN_ELESS)
      continue;
    b->addItem(LogicalSwitchData(func).funcToString(), func);
  }
  b->setMaxVisibleItems(10);
}

void LogicalSwitchesPanel::update()
{
  for (int i = 0; i < lsCapability; i++) {
    updateLine(i);
  }
}

void LogicalSwitchesPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    memcpy(&model->logicalSw[selectedIndex], data.constData(), sizeof(LogicalSwitchData));
    updateLine(selectedIndex);
    updateItemModels();
    emit modified();
  }
}

void LogicalSwitchesPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Logical Switch. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  memmove(&model->logicalSw[selectedIndex], &model->logicalSw[selectedIndex + 1], (CPN_MAX_LOGICAL_SWITCHES - (selectedIndex + 1)) * sizeof(LogicalSwitchData));
  model->logicalSw[lsCapability - 1].clear();

  model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, -1);
  update();
  updateItemModels();
  emit modified();
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
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Logical Switch. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void LogicalSwitchesPanel::onCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"),this,SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"),this,SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"),this,SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"),this,SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"),this,SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"),this,SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"),this,SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"),this,SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"),this,SLOT(cmClearAll()));

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
  return ((selectedIndex < lsCapability - 1) && (model->logicalSw[lsCapability - 1].isEmpty()));
}

bool LogicalSwitchesPanel::moveDownAllowed() const
{
  return selectedIndex < lsCapability - 1;
}

bool LogicalSwitchesPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void LogicalSwitchesPanel::cmMoveUp()
{
  swapData(selectedIndex, selectedIndex - 1);
}

void LogicalSwitchesPanel::cmMoveDown()
{
  swapData(selectedIndex, selectedIndex + 1);
}

void LogicalSwitchesPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Logical Switch. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->logicalSw[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_CLEAR, selectedIndex);
  updateLine(selectedIndex);
  updateItemModels();
  emit modified();
}

void LogicalSwitchesPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Logical Switches. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < lsCapability; i++) {
    model->logicalSw[i].clear();
    model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_CLEAR, i);
  }
  update();
  updateItemModels();
  emit modified();
}

void LogicalSwitchesPanel::cmInsert()
{
  memmove(&model->logicalSw[selectedIndex + 1], &model->logicalSw[selectedIndex], (CPN_MAX_LOGICAL_SWITCHES - (selectedIndex + 1)) * sizeof(LogicalSwitchData));
  model->logicalSw[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, 1);
  update();
  updateItemModels();
  emit modified();
}

void LogicalSwitchesPanel::swapData(int idx1, int idx2)
{
  if ((idx1 != idx2) && (!model->logicalSw[idx1].isEmpty() || !model->logicalSw[idx2].isEmpty())) {
    LogicalSwitchData lstmp = model->logicalSw[idx2];
    LogicalSwitchData *lsw1 = &model->logicalSw[idx1];
    LogicalSwitchData *lsw2 = &model->logicalSw[idx2];
    memcpy(lsw2, lsw1, sizeof(LogicalSwitchData));
    memcpy(lsw1, &lstmp, sizeof(LogicalSwitchData));
    model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
    updateLine(idx1);
    updateLine(idx2);
    updateItemModels();
    emit modified();
  }
}

void LogicalSwitchesPanel::updateItemModels()
{
  lock = true;
  sharedItemModels->update(AbstractItemModel::IMUE_LogicalSwitches);
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
