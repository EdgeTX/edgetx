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

#include "gvars.h"
#include "gvardata.h"
#include "flightmodedata.h"
#include "filtereditemmodels.h"
#include "eeprominterface.h"
#include "helpers.h"
#include "namevalidator.h"

GlobalVariablesPanel::GlobalVariablesPanel(QWidget * parent, ModelData & model,
                                           GeneralSettings & generalSettings,
                                           Firmware * firmware,
                                           CompoundItemModelFactory * sharedItemModels) :
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels),
  selectedIndex(0),
  modelsUpdateCnt(0),
  gvars(firmware->getCapability(Gvars)),
  modes(firmware->getCapability(FlightModes)),
  modeName(FlightModeData::modeName()),
  modePrefix(FlightModeData::modeAbbrev())
{
  AbstractStaticItemModel *unitItemModel = GVarData::unitItemModel();
  AbstractStaticItemModel *precItemModel = GVarData::precItemModel();
  CompoundItemModelFactory *gvarModesIMF = new CompoundItemModelFactory(&generalSettings, &model);

  int firstmidx = 0;
  for (int i = 0; i < modes; i++) {
    int id = gvarModesIMF->registerItemModel(FlightModeData::gvarModesItemModel(i));
    if (i == 0)
      firstmidx = id;
  }

  // add mode headers post table construction as they span columns
  QStringList headerLabels;
  headerLabels << "#" << tr("Name") << tr("Unit") << tr("Prec") << tr("Min")
               << tr("Max") << tr("Popup");

  tableLayout = new TableLayout(this, gvars, headerLabels);

  for (int i = 0; i < gvars; i++) {
    int col = 0;
    // label
    QLabel *label = new QLabel(this);
    label->setText(tr("GV%1").arg(i + 1));
    label->setProperty("index", i);
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    label->setToolTip(tr("Popup menu available"));
    label->setMouseTracking(true);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(label, &QWidget::customContextMenuRequested, this,
            &GlobalVariablesPanel::onCustomContextMenuRequested);
    tableLayout->addWidget(i, col++, label);

    // name
    leName[i] = new QLineEdit(this);
    QLineEdit *name = leName[i];
    name->setProperty("index", i);
    name->setValidator(new NameValidator(firmware->getBoard(), this));
    name->setMaxLength(firmware->getCapability(GvarsName));
    connect(name, &QLineEdit::editingFinished, this, &GlobalVariablesPanel::nameEditingFinished);
    tableLayout->addWidget(i, col++, name);

    // unit
    cboUnit[i] = new QComboBox(this);
    QComboBox *unit = cboUnit[i];
    unit->setProperty("index", i);
    unit->setModel(unitItemModel);
    connect(unit, &QComboBox::currentIndexChanged, this, &GlobalVariablesPanel::unitCurrentIndexChanged);
    tableLayout->addWidget(i, col++, unit);

    // precision
    cboPrec[i] = new QComboBox(this);
    QComboBox *prec = cboPrec[i];
    prec->setProperty("index", i);
    prec->setModel(precItemModel);
    connect(prec, &QComboBox::currentIndexChanged, this, &GlobalVariablesPanel::precCurrentIndexChanged);
    tableLayout->addWidget(i, col++, prec);

    // min
    dsbMin[i] = new QDoubleSpinBox(this);
    QDoubleSpinBox *min = dsbMin[i];
    min->setProperty("index", i);
    connect(min, &QDoubleSpinBox::editingFinished, this, &GlobalVariablesPanel::minEditingFinished);
    tableLayout->addWidget(i, col++, min);

    // max
    dsbMax[i] = new QDoubleSpinBox(this);
    QDoubleSpinBox *max = dsbMax[i];
    max->setProperty("index", i);
    connect(max, &QDoubleSpinBox::editingFinished, this, &GlobalVariablesPanel::maxEditingFinished);
    tableLayout->addWidget(i, col++, max);

    // popup
    chkPopup[i] = new QCheckBox(this);
    QCheckBox *popup = chkPopup[i];
    popup->setProperty("index", i);
    connect(popup, &QCheckBox::toggled, this, &GlobalVariablesPanel::popupToggled);
    tableLayout->addWidget(i, col++, popup, Qt::AlignHCenter);

    // radio modes (drive/flight)
    for (int j = 0; j < modes; j++) {
      if (j > 0) {
        // use mode
        chkUseMode[i][j] = new QCheckBox(tr("%1M").arg(modePrefix),this);
        QCheckBox *usemode = chkUseMode[i][j];
        setIndexes(usemode, i, j);
        connect(usemode, &QCheckBox::toggled, this, &GlobalVariablesPanel::useModeToggled);
        tableLayout->addWidget(i, col++, usemode, Qt::AlignHCenter);

        // mode
        cboMode[i][j] = new QComboBox(this);
        QComboBox *mode = cboMode[i][j];
        setIndexes(mode, i, j);
        mode->setModel(gvarModesIMF->getItemModel(firstmidx + j));
        connect(mode, &QComboBox::currentIndexChanged, this, &GlobalVariablesPanel::modeCurrentIndexChanged);
        tableLayout->addWidget(i, col, mode); // do not increment col as either cboMode or dsbValue visible
      }

      // value
      dsbValue[i][j] = new QDoubleSpinBox(this);
      QDoubleSpinBox *value = dsbValue[i][j];
      setIndexes(value, i, j);
      connect(value, &QDoubleSpinBox::editingFinished, this, &GlobalVariablesPanel::valueEditingFinished);
      tableLayout->addWidget(i, col++, value);
    }
  }

  int col = headerLabels.size();

  for (int i = 0; i < modes; i++) {
    int colspan = i > 0 ? 2 : 1;
    tableLayout->addColumnHead(model.flightModeData[i].nameToString(i), col, colspan);
    col += colspan;
  }

  QLabel *lblWarn = new QLabel(this);
  lblWarn->setText(tr("WARNING: changing ranges or precision can affect configured Logical Switches and Special Functions"));
  tableLayout->addWidget(gvars, 1, 1, -1, lblWarn);

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->pushRowsUp(gvars + 2);
  tableLayout->pushColumnsLeft(col);
}

GlobalVariablesPanel::~GlobalVariablesPanel()
{
}

void GlobalVariablesPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, tr("Edit Global Variables"),
          tr("Clear global variable #%1. Are you sure?").arg(selectedIndex + 1),
          QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->gvarClear(selectedIndex);
  updateLine(selectedIndex);
  updateItemModels();
  emit modified();
}

void GlobalVariablesPanel::cmClearAll()
{
  if (QMessageBox::question(this, tr("Edit Global Variables"),
        tr("Clear all global variables. Are you sure?"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < gvars; i++) {
    model->gvarClear(i);
  }

  update();
  updateItemModels();
  emit modified();
}

void GlobalVariablesPanel::cmCopy()
{
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << model->gvarData[selectedIndex];

  for (int j = 0; j < modes; j++) {
    stream << model->flightModeData[j].gvars[selectedIndex];
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_GLOBAL_VARIABLE, data);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void GlobalVariablesPanel::cmCut()
{
  if (QMessageBox::question(this, tr("Edit Global Variables"),
        tr("Cut global variable #%1. Are you sure?").arg(selectedIndex + 1),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  cmCopy();
  cmClear(false);
}

void GlobalVariablesPanel::cmDelete()
{
  if (QMessageBox::question(this, tr("Edit Global Variables"),
        tr("Delete global variable #%1. Are you sure?").arg(selectedIndex + 1),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  model->gvarDelete(selectedIndex);

  for (int i = selectedIndex; i < gvars; i++) {
    updateLine(i);
  }

  updateItemModels();
  emit modified();
}

void GlobalVariablesPanel::cmInsert()
{
  model->gvarInsert(selectedIndex);

  for (int i = selectedIndex; i < gvars; i++) {
    updateLine(i);
  }

  updateItemModels();
  emit modified();
}

void GlobalVariablesPanel::cmMoveDown()
{
  swapData(selectedIndex, selectedIndex + 1);
}

void GlobalVariablesPanel::cmMoveUp()
{
  swapData(selectedIndex, selectedIndex - 1);
}

void GlobalVariablesPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> model->gvarData[selectedIndex];

    for (int j = 0; j < modes; j++) {
      stream >> model->flightModeData[j].gvars[selectedIndex];
    }

    updateLine(selectedIndex);
    updateItemModels();
    emit modified();
  }
}

void GlobalVariablesPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this,
          &GlobalVariablesPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this,
          &GlobalVariablesPanel::onItemModelUpdateComplete);
}

bool GlobalVariablesPanel::deleteAllowed() const
{
  return selectedIndex < gvars - 1;
}

bool GlobalVariablesPanel::getIndexes(QWidget * widget, int &gvarIdx, int &modeIdx) {
  bool ok = false;
  unsigned int i = (unsigned int)widget->property("index").toInt(&ok);

  if (!ok)
    return false;

  modeIdx = (int)Helpers::getBitmappedValue(i, 0, 8);
  gvarIdx = (int)Helpers::getBitmappedValue(i, 1, 8);
  return true;
}

bool GlobalVariablesPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();

  if (mimeData->hasFormat(MIMETYPE_GLOBAL_VARIABLE)) {
    if (data) {
      data->append(mimeData->data(MIMETYPE_GLOBAL_VARIABLE));
    }

    return true;
  }

  return false;
}

void GlobalVariablesPanel::maxEditingFinished()
{
  if (!lock) {
    QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox*>(sender());
    bool ok;
    int gidx = dsb->property("index").toInt(&ok);

    if (ok) {
      model->gvarSetMax(gidx, dsb->value());
      updateLine(gidx);
      emit modified();
    }
  }
}

void GlobalVariablesPanel::minEditingFinished()
{
  if (!lock) {
    QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox*>(sender());
    bool ok;
    int gidx = dsb->property("index").toInt(&ok);

    if (ok) {
      model->gvarSetMin(gidx, dsb->value());
      updateLine(gidx);
      emit modified();
    }
  }
}

void GlobalVariablesPanel::modeCurrentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *cbo = qobject_cast<QComboBox*>(sender());
    int gidx = 0;
    int midx = 0;

    if (getIndexes(cbo, gidx, midx)) {
      model->flightModeData[midx].gvars[gidx] = cbo->currentData().toInt();

      if (model->isGVarLinkedCircular(midx, gidx)) {
        QMessageBox::warning(this, tr("Edit Global Variables"),
            tr("Warning: Global variable links back to itself, %1M0 used.")
              .arg(modePrefix));
        model->flightModeData[midx].gvars[gidx] =
          model->flightModeData->linkedGVarFlightModeZero(midx);
      }

      updateLine(gidx);
      emit modified();
    }
  }
}

bool GlobalVariablesPanel::moveDownAllowed() const
{
  return selectedIndex < gvars - 1;
}

bool GlobalVariablesPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void GlobalVariablesPanel::nameEditingFinished()
{
  if (!lock) {
    QLineEdit *lineedit = qobject_cast<QLineEdit*>(sender());
    bool ok;
    int gidx = lineedit->property("index").toInt(&ok);

    if (ok) {
      memset(&model->gvarData[gidx].name, 0, sizeof(model->gvarData[gidx].name));
      strcpy(model->gvarData[gidx].name, lineedit->text().toLatin1());
      updateItemModels();
      emit modified();
    }
  }
}

void GlobalVariablesPanel::onCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this,
                        SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this,
                        SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this,
                        SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this,
                        SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this,
                        SLOT(cmInsert()))->setEnabled(model->gvarInsertAllowed(selectedIndex));
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this,
                        SLOT(cmDelete()))->setEnabled(deleteAllowed());
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this,
                        SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this,
                        SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this,
                        SLOT(cmClearAll()));
  contextMenu.exec(globalPos);
}

void GlobalVariablesPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void GlobalVariablesPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;

  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}

void GlobalVariablesPanel::popupToggled(bool checked)
{
  if (!lock) {
    QCheckBox *chk = qobject_cast<QCheckBox*>(sender());
    bool ok;
    int gidx = chk->property("index").toInt(&ok);

    if (ok) {
      model->gvarData[gidx].popup = checked;
      emit modified();
    }
  }
}

void GlobalVariablesPanel::precCurrentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int gidx = comboBox->property("index").toInt();
    model->gvarData[gidx].prec = index;
    updateLine(gidx);
    emit modified();
  }
}

void GlobalVariablesPanel::setIndexes(QWidget * widget, int gvarIdx, int modeIdx) {
  unsigned int i = 0;
  Helpers::setBitmappedValue(i, modeIdx, 0, 8);
  Helpers::setBitmappedValue(i, gvarIdx, 1, 8);
  widget->setProperty("index", i);
}

void GlobalVariablesPanel::setMinMax(QDoubleSpinBox * dsb, int min, int max, int val)
{
  bool ok;
  int gidx = dsb->property("index").toInt(&ok);

  if (ok) {
    float mul = model->gvarData[gidx].multiplierGet();
    dsb->setDecimals(model->gvarData[gidx].prec);
    dsb->setSingleStep(mul);
    dsb->setSuffix(model->gvarData[gidx].unitToString());
    dsb->setMinimum(min * mul);
    dsb->setMaximum(max * mul);
    dsb->setValue(val * mul);
  }
}

void GlobalVariablesPanel::setValue(QDoubleSpinBox * dsb, int val)
{
  int gidx = 0;
  int midx = 0;

  if (getIndexes(dsb, gidx, midx)) {
    float mul = model->gvarData[gidx].multiplierGet();
    dsb->setDecimals(model->gvarData[gidx].prec);
    dsb->setSingleStep(mul);
    dsb->setSuffix(model->gvarData[gidx].unitToString());
    dsb->setMinimum(model->gvarData[gidx].getMinPrec());
    dsb->setMaximum(model->gvarData[gidx].getMaxPrec());
    dsb->setValue(val * mul);
  }
}

void GlobalVariablesPanel::swapData(int index1, int index2)
{
  if ((index1 != index2)) {
    model->gvarSwap(index1, index2);
    updateLine(index1);
    updateLine(index2);
    updateItemModels();
    emit modified();
  }
}

void GlobalVariablesPanel::unitCurrentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int gidx = comboBox->property("index").toInt();
    model->gvarData[gidx].unit = index;
    updateLine(gidx);
    emit modified();
  }
}

void GlobalVariablesPanel::update()
{
  int col = 7;

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    QString text(model->flightModeData[i].nameToString(i));
    tableLayout->updateColumnHeading(col, text);
    i == 0 ? col++ : col += 2;
  }

  for (int i = 0; i < gvars; i++) {
    updateLine(i);
  }
}

void GlobalVariablesPanel::updateLine(int index)
{
  if (index < 0 || index >= gvars)
    return;

  lock = true;
  leName[index]->setText(model->gvarData[index].name);
  cboUnit[index]->setCurrentIndex(model->gvarData[index].unit);
  cboPrec[index]->setCurrentIndex(model->gvarData[index].prec);
  setMinMax(dsbMin[index], GVAR_MIN_VALUE, model->gvarData[index].getMax(),
            model->gvarData[index].getMin());
  setMinMax(dsbMax[index], model->gvarData[index].getMin(), GVAR_MAX_VALUE,
            model->gvarData[index].getMax());
  chkPopup[index]->setChecked(model->gvarData[index].popup);

  for (int j = 0; j < modes; j++) {
    if (!model->isGVarLinked(j, index)) {
      if (j > 0) {
        chkUseMode[index][j]->setChecked(false);
        cboMode[index][j]->setCurrentIndex(0);
        cboMode[index][j]->setVisible(false);
      }

      dsbValue[index][j]->setVisible(true);
      setValue(dsbValue[index][j], model->getGVarValue(j, index));
    } else {
      chkUseMode[index][j]->setChecked(true);
      cboMode[index][j]->setVisible(true);
      cboMode[index][j]->setCurrentIndex(cboMode[index][j]->findData(model->flightModeData[j].gvars[index]));
      dsbValue[index][j]->setValue(0);
      dsbValue[index][j]->setVisible(false);
    }
  }

  lock = false;
}

void GlobalVariablesPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_GVars);
}

void GlobalVariablesPanel::useModeToggled(bool checked)
{
  if (!lock) {
    QCheckBox *chk = qobject_cast<QCheckBox*>(sender());
    int gidx = 0;
    int midx = 0;

    if (getIndexes(chk, gidx, midx)) {
      int val = 0;

      if (checked)
        val = model->flightModeData->linkedGVarFlightModeZero(midx);
      else if (val < model->gvarData[gidx].getMin())
        val = model->gvarData[gidx].getMin();
      else if (val > model->gvarData[gidx].getMax())
        val = model->gvarData[gidx].getMax();

      model->flightModeData[midx].gvars[gidx] = val;
      updateLine(gidx);
      emit modified();
    }
  }
}

void GlobalVariablesPanel::valueEditingFinished()
{
  if (!lock) {
    QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox*>(sender());
    int gidx = 0;
    int midx = 0;

    if (getIndexes(dsb, gidx, midx)) {
      model->flightModeData[midx].gvars[gidx] = dsb->value() *
        model->gvarData[gidx].multiplierSet();
      updateLine(gidx);
      emit modified();
    }
  }
}
