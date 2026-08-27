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

#include "flightmodes.h"
#include "ui_flightmode.h"
#include "filtereditemmodels.h"
#include "helpers.h"
#include "customdebug.h"
#include "namevalidator.h"

FlightModePanel::FlightModePanel(QWidget * parent, ModelData & model, int phaseIdx,
                                 GeneralSettings & generalSettings,
                                 Firmware * firmware,
                                 FilteredItemModel * rawSwitchFilteredModel,
                                 QString radioMode):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::FlightMode),
  phaseIdx(phaseIdx),
  phase(model.flightModeData[phaseIdx]),
  radioMode(radioMode)
{
  ui->setupUi(this);
  connectItemModelEvents(rawSwitchFilteredModel);

  ui->labelName->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->labelName->setToolTip(tr("Popup menu available"));
  connect(ui->labelName, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenuRequested(const QPoint &)));

  board = firmware->getBoard();
  fmCount = firmware->getCapability(FlightModes);
  trimCount = Boards::getCapability(board, Board::NumTrims);
  gvCount = firmware->getCapability(Gvars);

  ui->name->setValidator(new NameValidator(board, this));
  ui->name->setMaxLength(firmware->getCapability(FlightModesName));
  connect(ui->name, SIGNAL(editingFinished()), this, SLOT(phaseName_editingFinished()));

  // Flight mode switch
  if (phaseIdx > 0) {
    ui->swtch->setModel(rawSwitchFilteredModel);
    connect(ui->swtch, SIGNAL(currentIndexChanged(int)), this, SLOT(phaseSwitch_currentIndexChanged(int)));
  }
  else {
    ui->swtch->hide();
  }

  int scale = firmware->getCapability(SlowScale);
  int range = firmware->getCapability(SlowRange);
  ui->fadeIn->setMaximum(float(range) / scale);
  ui->fadeIn->setSingleStep(1.0 / scale);
  ui->fadeIn->setDecimals((scale == 1 ? 0 :1) );
  connect(ui->fadeIn, SIGNAL(editingFinished()), this, SLOT(phaseFadeIn_editingFinished()));
  ui->fadeOut->setMaximum(float(range) / scale);
  ui->fadeOut->setSingleStep(1.0 / scale);
  ui->fadeOut->setDecimals((scale == 1 ? 0 :1));
  connect(ui->fadeOut, SIGNAL(editingFinished()), this, SLOT(phaseFadeOut_editingFinished()));

  // The trims
  QString labels[CPN_MAX_TRIMS];

  for (int i = 0; i < Boards::getCapability(board, Board::Sticks); i++) {
    labels[i] = Boards::getInputName(i);
  }

  for (int i = Boards::getCapability(board, Board::Sticks); i < CPN_MAX_TRIMS; i++) {
    labels[i] = QString("T%1").arg(i + 1);
  }

  trimsLabel << ui->trim1Label << ui->trim2Label << ui->trim3Label << ui->trim4Label << ui->trim5Label << ui->trim6Label << ui->trim7Label << ui->trim8Label;
  trimsUse << ui->trim1Use << ui->trim2Use << ui->trim3Use << ui->trim4Use << ui->trim5Use << ui->trim6Use << ui->trim7Use << ui->trim8Use;
  trimsValue << ui->trim1Value << ui->trim2Value << ui->trim3Value << ui->trim4Value << ui->trim5Value << ui->trim6Value << ui->trim7Value << ui->trim8Value;
  trimsSlider << ui->trim1Slider << ui->trim2Slider << ui->trim3Slider << ui->trim4Slider << ui->trim5Slider << ui->trim6Slider << ui->trim7Slider << ui->trim8Slider;

  for (int i = trimCount; i < CPN_MAX_TRIMS; i++) {
    trimsLabel[i]->hide();
    trimsUse[i]->hide();
    trimsValue[i]->hide();
    trimsSlider[i]->hide();
  }

  for (int i = 0; i < trimCount; i++) {
    trimsLabel[i]->setText(labels[Boards::isAir(board) ? CONVERT_AIRMODE(i + 1) - 1 : CONVERT_SURFACEMODE(i + 1) - 1]);
    QComboBox * cb = trimsUse[i];
    cb->setProperty("index", i);
    cb->addItem(tr("Trim disabled"), -1);
    cb->addItem(tr("3POS toggle switch"), TRIM_MODE_3POS);

    for (int m = 0; m < fmCount; m++) {
      if (m == phaseIdx) {
        cb->addItem(tr("Own Trim"), m * 2);
      }
      else if (phaseIdx > 0) {
        cb->addItem(tr("Use Trim from %1 Mode %2").arg(radioMode).arg(m), m * 2);
        cb->addItem(tr("Use Trim from %1 Mode %2 + Own Trim as an offset").arg(radioMode).arg(m), m * 2 + 1);
      }
    }

    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(phaseTrimUse_currentIndexChanged(int)));

    int trimsMax = firmware->getCapability(ExtendedTrimsRange);
    if (trimsMax == 0 || !model.extendedTrims) {
      trimsMax = firmware->getCapability(TrimsRange);
    }

    trimsValue[i]->setProperty("index", i);
    trimsValue[i]->setRange(-trimsMax, +trimsMax);
    connect(trimsValue[i], SIGNAL(valueChanged(int)), this, SLOT(phaseTrim_valueChanged()));

    trimsSlider[i]->setProperty("index", i);
    trimsSlider[i]->setRange(-trimsMax, +trimsMax);
    int chn = Boards::isAir(board) ? CONVERT_AIRMODE(i + 1) - 1 : CONVERT_SURFACEMODE(i + 1) - 1;

    if (model.throttleReversed && ((Boards::isAir(board) && chn == 2/*TODO constant*/) ||
                                   (Boards::isSurface(board) && chn == 1/*TODO constant*/)))
      trimsSlider[i]->setInvertedAppearance(true);

    connect(trimsSlider[i], SIGNAL(valueChanged(int)), this, SLOT(phaseTrimSlider_valueChanged()));
  }

  disableMouseScrolling();

  update();
}

FlightModePanel::~FlightModePanel()
{
  delete ui;
}

void FlightModePanel::update()
{
  ui->name->setText(phase.name);
  ui->swtch->setCurrentIndex(ui->swtch->findData(phase.swtch.toValue()));

  int scale = firmware->getCapability(SlowScale);
  ui->fadeIn->setValue(float(phase.fadeIn)/scale);
  ui->fadeOut->setValue(float(phase.fadeOut)/scale);

  for (int i = 0; i < getBoardCapability(firmware->getBoard(), Board::NumTrims); i++) {
    trimUpdate(i);
  }
}

void FlightModePanel::phaseName_editingFinished()
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
  strcpy(phase.name, lineEdit->text().toLatin1());
  emit phaseNameChanged();
  emit modified();
}

void FlightModePanel::phaseSwitch_currentIndexChanged(int index)
{
  if (!lock) {
    bool ok;
    const RawSwitch rs(ui->swtch->itemData(index).toInt(&ok));

    if (ok && phase.swtch.toValue() != rs.toValue()) {
      phase.swtch = rs;
      emit phaseSwitchChanged();
      emit modified();
    }
  }
}

void FlightModePanel::phaseFadeIn_editingFinished()
{
  QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(sender());
  int scale = firmware->getCapability(SlowScale);
  phase.fadeIn = round(spinBox->value()*scale);
  emit modified();
}

void FlightModePanel::phaseFadeOut_editingFinished()
{
  QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(sender());
  int scale = firmware->getCapability(SlowScale);
  phase.fadeOut = round(spinBox->value()*scale);
  emit modified();
}

void FlightModePanel::trimUpdate(unsigned int trim)
{
  lock = true;
  int chn = Boards::isAir(board) ? CONVERT_AIRMODE(trim + 1) - 1 : CONVERT_SURFACEMODE(trim + 1) - 1;
  int value = model->getTrimValue(phaseIdx, chn);
  trimsSlider[trim]->setValue(value);
  trimsValue[trim]->setValue(value);

  if (phase.trimMode[chn] < 0) {
    trimsUse[trim]->setCurrentIndex(0);
    trimsValue[trim]->setEnabled(false);
    trimsSlider[trim]->setEnabled(false);
  } else if (phase.trimMode[chn] == TRIM_MODE_3POS) {
    trimsUse[trim]->setCurrentIndex(1);
    trimsValue[trim]->setEnabled(false);
    trimsSlider[trim]->setEnabled(false);
  }
  else {
    trimsUse[trim]->setCurrentIndex(2 + 2 * phase.trimRef[chn] + phase.trimMode[chn] - (phase.trimRef[chn] > phaseIdx ? 1 : 0));

    if (phaseIdx == 0 || phase.trimRef[chn] == phaseIdx || phase.trimMode[chn] != 0) {
      trimsValue[trim]->setEnabled(true);
      trimsSlider[trim]->setEnabled(true);
    }
    else {
      trimsValue[trim]->setEnabled(false);
      trimsSlider[trim]->setEnabled(false);
    }
  }
  lock = false;
}

void FlightModePanel::phaseTrimUse_currentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int trim = comboBox->property("index").toInt();
    int chn = Boards::isAir(board) ? CONVERT_AIRMODE(trim + 1) - 1 : CONVERT_SURFACEMODE(trim + 1) - 1;
    int data = comboBox->itemData(index).toInt();

    if (data < 0) {
      phase.trimMode[chn] = -1;
      phase.trimRef[chn] = 0;
      phase.trim[chn] = 0;
    } else if (data == TRIM_MODE_3POS) {
      phase.trimMode[chn] = TRIM_MODE_3POS;
      phase.trimRef[chn] = 0;
      phase.trim[chn] = 0;
    } else {
      phase.trimMode[chn] = data % 2;
      phase.trimRef[chn] = data / 2;
      phase.trim[chn] = 0;
    }

    trimUpdate(trim);
    emit modified();
  }
}

void FlightModePanel::phaseTrim_valueChanged()
{
  if (!lock) {
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(sender());
    int trim = spinBox->property("index").toInt();
    int chn = Boards::isAir(board) ? CONVERT_AIRMODE(trim + 1) - 1 : CONVERT_SURFACEMODE(trim + 1) - 1;
    int value = spinBox->value();
    model->setTrimValue(phaseIdx, chn, value);
    lock = true;
    trimsSlider[trim]->setValue(value);
    lock = false;
    emit modified();
  }
}

void FlightModePanel::phaseTrimSlider_valueChanged()
{
  if (!lock) {
    QSlider *slider = qobject_cast<QSlider*>(sender());
    int trim = slider->property("index").toInt();
    int chn = Boards::isAir(board) ? CONVERT_AIRMODE(trim + 1) - 1 : CONVERT_SURFACEMODE(trim + 1) - 1;
    int value = slider->value();
    model->setTrimValue(phaseIdx, chn, value);
    lock = true;
    trimsValue[trim]->setValue(value);
    lock = false;
    emit modified();
  }
}

void FlightModePanel::onCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  QPoint globalPos = label->mapToGlobal(pos);

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
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(cmClearAll()))->setEnabled(phaseIdx == 0);

  contextMenu.exec(globalPos);
}

bool FlightModePanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();

  if (mimeData->hasFormat(MIMETYPE_FLIGHTMODE)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_FLIGHTMODE));
    return true;
  }

  return false;
}

bool FlightModePanel::insertAllowed() const
{
  return ((phaseIdx < fmCount - 1) && (model->flightModeData[fmCount - 1].isEmpty(fmCount - 1)));
}

bool FlightModePanel::moveDownAllowed() const
{
  return phaseIdx < fmCount - 1;
}

bool FlightModePanel::moveUpAllowed() const
{
  return phaseIdx > 0;
}

void FlightModePanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear %1 Mode. Are you sure?").arg(radioMode), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  phase.clear(phaseIdx);

  if (phaseIdx == 0) {
    for (int i = 0; i < gvCount; i++) {
      model->gvarData[i].clear();

      for (int j = 0; j < fmCount; j++) {
        FlightModeData *fm = &model->flightModeData[j];
        fm->gvars[i] = fm->linkedGVarFlightModeZero(j);
      }

      model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_CLEAR, i);
    }
  } else {
    QComboBox * pswtch = ui->swtch;
    RawSwitch item = RawSwitch(SWITCH_TYPE_NONE);
    pswtch->setCurrentIndex(pswtch->findText(item.toString()));

    for (int i = 0; i < gvCount; i++) {
      phase.gvars[i] = phase.linkedGVarFlightModeZero(phaseIdx);
    }
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_CLEAR, phaseIdx);

  update();
  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all %1 Modes. Are you sure?").arg(radioMode), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int j = 0; j < fmCount; j++) {
    FlightModeData *fm = &model->flightModeData[j];
    fm->clear(j);

    for (int i = 0; i < gvCount; i++) {
      fm->gvars[i] = fm->linkedGVarFlightModeZero(j);
    }

    model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_CLEAR, j);
  }

  for (int i = 0; i < gvCount; i++) {
    model->gvarData[i].clear();
    model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_CLEAR, i);
  }

  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::cmCopy()
{
  QByteArray data;
  data.append((char*)&model->flightModeData[phaseIdx], sizeof(FlightModeData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_FLIGHTMODE, data);
  QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void FlightModePanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut %1 Mode. Are you sure?").arg(radioMode),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  cmCopy();
  cmClear(false);
}

void FlightModePanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete %1 Mode. Are you sure?").arg(radioMode),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  const int maxidx = fmCount - 1;

  memmove(&model->flightModeData[phaseIdx], &model->flightModeData[phaseIdx + 1], (CPN_MAX_FLIGHT_MODES - (phaseIdx + 1)) * sizeof(FlightModeData));
  model->flightModeData[maxidx].clear(maxidx);

  for (int j = 0; j < maxidx; j++) {
    FlightModeData *fm = &model->flightModeData[j];

    if (phaseIdx == 0 && j == 0)
      fm->swtch = RawSwitch(0);

    for (int i = 0; i < trimCount; i++) {
      if (phaseIdx == 0 && j == 0) {
        fm->trimMode[i] = 0;
        fm->trimRef[i] = 0;
      }
      else if (fm->trimRef[i] > phaseIdx && fm->trimMode[i] > -1) {
        fm->trimRef[i]--;
      }
      else if (fm->trimRef[i] == phaseIdx) {
        fm->trimMode[i] = 0;
        fm->trimRef[i] = 0;
        fm->trim[i] = 0;
      }
    }

    for (int i = 0; i < gvCount; i++) {
      if (model->isGVarLinked(j, i)) {
        int idx = model->getGVarFlightModeIndex(j, i);
        if (phaseIdx == 0 && j == 0)
          model->setGVarFlightModeIndexToValue(j, i, j);
        else if (idx > phaseIdx)
          model->setGVarFlightModeIndexToValue(j, i, idx - 1);
        else if (idx == phaseIdx)
          model->setGVarFlightModeIndexToValue(j, i, 0);
      }
    }

  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_SHIFT, phaseIdx, 0, -1);

  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::cmInsert()
{
  const int maxidx = fmCount - 1;

  memmove(&model->flightModeData[phaseIdx + 1], &model->flightModeData[phaseIdx],
          (CPN_MAX_FLIGHT_MODES - (phaseIdx + 1)) * sizeof(FlightModeData));
  model->flightModeData[phaseIdx].clear(phaseIdx);

  for (int j = maxidx; j > 0; j--) {
    FlightModeData *fm = &model->flightModeData[j];

    for (int i = 0; i < trimCount; i++) {
      if (phaseIdx == 0 && j == 1) {
        fm->trimRef[i] = j;
      } else if (fm->trimRef[i] >= phaseIdx) {
        if (fm->trimRef[i] < maxidx) {
          fm->trimRef[i]++;
        } else {
          fm->trimMode[i] = 0;
          fm->trimRef[i] = 0;
          fm->trim[i] = 0;
        }
      }
    }

    for (int i = 0; i < gvCount; i++) {
      if (model->isGVarLinked(j, i)) {
        int idx = model->getGVarFlightModeIndex(j, i);
        if (phaseIdx == 0 && j == 1)
          model->setGVarFlightModeIndexToValue(j, i, j);
        else if (idx >= phaseIdx)
          model->setGVarFlightModeIndexToValue(j, i, idx + 1);
      }
    }
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_SHIFT, phaseIdx, 0, 1);

  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::cmMoveDown()
{
  swapData(phaseIdx, phaseIdx + 1);
}

void FlightModePanel::cmMoveUp()
{
  swapData(phaseIdx, phaseIdx - 1);
}

void FlightModePanel::cmPaste()
{
  QByteArray data;

  if (hasClipboardData(&data)) {
    FlightModeData *fm = &model->flightModeData[phaseIdx];
    memcpy(fm, data.constData(), sizeof(FlightModeData));

    if (phaseIdx == 0)
      fm->swtch = RawSwitch(0);

    for (int i = 0; i < trimCount; i++) {
      if (phaseIdx == 0) {
        fm->trimMode[i] = 0;
        fm->trimRef[i] = 0;
      }
    }

    //  TODO fix up linked GVs but need to know source FM index to make adjustments
    for (int i = 0; i < gvCount; i++) {
      if (model->isGVarLinked(phaseIdx, i)) {
        int linkedidx = model->getGVarFlightModeIndex(phaseIdx, i);

        if (phaseIdx == 0 || linkedidx == phaseIdx)
          model->setGVarFlightModeIndexToValue(phaseIdx, i, phaseIdx);
      }
    }

    emit phaseDataChanged();
    emit modified();
  }
}

void FlightModePanel::swapData(int idx1, int idx2)
{
  if (idx1 == idx2)
    return;

  const int shift = idx2 - idx1;

  FlightModeData fmdtmp = model->flightModeData[idx2];
  FlightModeData *fmd1 = &model->flightModeData[idx1];
  FlightModeData *fmd2 = &model->flightModeData[idx2];
  memcpy(fmd2, fmd1, sizeof(FlightModeData));
  memcpy(fmd1, &fmdtmp, sizeof(FlightModeData));

  if (idx1 == 0)
    fmd1->swtch = RawSwitch(0);
  else if (idx2 == 0)
    fmd2->swtch = RawSwitch(0);

  for (int i = 0; i < trimCount; i++) {
    if (fmd1->trimRef[i] == idx1)
      fmd1->trimRef[i] = idx2;
    else if (fmd1->trimRef[i] == idx2)
      fmd1->trimRef[i] = idx1;

    if (fmd2->trimRef[i] == idx1)
      fmd2->trimRef[i] = idx2;
    else if (fmd2->trimRef[i] == idx2)
      fmd2->trimRef[i] = idx1;

    if (idx1 == 0) {
      fmd1->trimMode[i] = 0;
      fmd1->trimRef[i] = 0;
    } else if (idx2 == 0) {
      fmd2->trimMode[i] = 0;
      fmd2->trimRef[i] = 0;
    }
  }

  for (int i = 0; i < gvCount; i++) {
    if (model->isGVarLinked(idx1, i)) {
      int linkedidx = model->getGVarFlightModeIndex(idx1, i);

      if (linkedidx == idx1)
        model->setGVarFlightModeIndexToValue(idx1, i, idx2 - shift);
      else if (linkedidx == idx2)
        model->setGVarFlightModeIndexToValue(idx1, i, idx1 + shift);
      else
        model->setGVarFlightModeIndexToValue(idx1, i, linkedidx);
    }

    if (model->isGVarLinked(idx2, i)) {
      int linkedidx = model->getGVarFlightModeIndex(idx2, i);

      if (linkedidx == idx1)
        model->setGVarFlightModeIndexToValue(idx2, i, idx2 - shift);
      else if (linkedidx == idx2)
        model->setGVarFlightModeIndexToValue(idx2, i, idx1 + shift);
      else
        model->setGVarFlightModeIndexToValue(idx2, i, linkedidx);
    }
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &FlightModePanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &FlightModePanel::onItemModelUpdateComplete);
}

void FlightModePanel::onItemModelAboutToBeUpdated()
{
  lock = true;
}

void FlightModePanel::onItemModelUpdateComplete()
{
  update();
  lock = false;
}

void FlightModePanel::onThrottleReverseChanged()
{
  for (int i = 0; i < CPN_MAX_TRIMS; i++) {
    int chn = Boards::isAir(board) ? CONVERT_AIRMODE(i + 1) - 1 : CONVERT_SURFACEMODE(i + 1) - 1;

    if (model->throttleReversed && ((Boards::isAir(board) && chn == 2/*TODO constant*/) ||
                                    (Boards::isSurface(board) && chn == 1/*TODO constant*/)))
      trimsSlider[i]->setInvertedAppearance(true);
    else
      trimsSlider[i]->setInvertedAppearance(false);
  }
}

/**********************************************************/

FlightModesPanel::FlightModesPanel(QWidget * parent, ModelData & model,
                                   GeneralSettings & generalSettings,
                                   Firmware * firmware,
                                   CompoundItemModelFactory * sharedItemModels,
                                   QString radioMode):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels),
  radioMode(radioMode)
{
  rawSwitchFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch), RawSwitch::MixesContext);
  connectItemModelEvents(rawSwitchFilteredModel);

  modesCount = firmware->getCapability(FlightModes);

  QGridLayout * gridLayout = new QGridLayout(this);
  tabWidget = new QTabWidget(this);

  for (int i = 0; i < modesCount; i++) {
    FlightModePanel * tab = new FlightModePanel(tabWidget, model, i, generalSettings, firmware, rawSwitchFilteredModel, radioMode);
    tab->setProperty("index", i);
    connect(tab, &FlightModePanel::modified,           this, &FlightModesPanel::modified);
    connect(tab, &FlightModePanel::phaseDataChanged,   this, &FlightModesPanel::onPhaseNameChanged);
    connect(tab, &FlightModePanel::phaseNameChanged,   this, &FlightModesPanel::onPhaseNameChanged);
    connect(tab, &FlightModePanel::phaseSwitchChanged, this, &FlightModesPanel::updateItemModels);

    connect(this, &FlightModesPanel::updated,             tab, &FlightModePanel::update);
    connect(this, &FlightModesPanel::refreshThrottleTrim, tab, &FlightModePanel::onThrottleReverseChanged);
    tabWidget->addTab(tab, getTabName(i));
    panels << tab;
  }

  connect(tabWidget, &QTabWidget::currentChanged, this, &FlightModesPanel::onTabIndexChanged);
  gridLayout->addWidget(tabWidget, 0, 0, 1, 1);
  onTabIndexChanged(0);
}

FlightModesPanel::~FlightModesPanel()
{
  delete rawSwitchFilteredModel;
}

QString FlightModesPanel::getTabName(int index)
{
  QString result = tr("%1 Mode %2").arg(radioMode).arg(index);
  const char *name = model->flightModeData[index].name;
  if (firmware->getCapability(FlightModesName) && strlen(name) > 0) {
    result += tr(" (%1)").arg(name);
  }
  else if (index == 0) {
    result += tr(" (default)");
  }

  return result;
}

void FlightModesPanel::onPhaseNameChanged()
{
  int index = sender()->property("index").toInt();
  tabWidget->setTabText(index, getTabName(index));
  updateItemModels();
}

void FlightModesPanel::update()
{
  emit updated();
}

void FlightModesPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &FlightModesPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &FlightModesPanel::onItemModelUpdateComplete);
}

void FlightModesPanel::onItemModelAboutToBeUpdated()
{
}

void FlightModesPanel::onItemModelUpdateComplete()
{
}

void FlightModesPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_FlightModes);
}

void FlightModesPanel::onTabIndexChanged(int index)
{
  if (index < panels.size())
    panels.at(index)->update();
}

void FlightModesPanel::onThrottleReverseChanged()
{
  emit refreshThrottleTrim();
}
