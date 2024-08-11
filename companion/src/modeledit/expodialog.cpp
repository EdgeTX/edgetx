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

#include "expodialog.h"
#include "ui_expodialog.h"
#include "filtereditemmodels.h"
#include "helpers.h"
#include "namevalidator.h"
#include "sourcenumref.h"

ExpoDialog::ExpoDialog(QWidget *parent, ModelData & model, ExpoData *expoData, GeneralSettings & generalSettings,
                       Firmware * firmware, QString & inputName, CompoundItemModelFactory * sharedItemModels) :
  QDialog(parent),
  ui(new Ui::ExpoDialog),
  model(model),
  generalSettings(generalSettings),
  firmware(firmware),
  ed(expoData),
  inputName(inputName),
  lock(false)
{
  ui->setupUi(this);

  Board::Type board = firmware->getBoard();
  dialogFilteredItemModels = new FilteredItemModelFactory();

  QLabel * lb_fp[CPN_MAX_FLIGHT_MODES] = {ui->lb_FP0, ui->lb_FP1, ui->lb_FP2, ui->lb_FP3, ui->lb_FP4, ui->lb_FP5, ui->lb_FP6, ui->lb_FP7, ui->lb_FP8 };
  QCheckBox * tmp[CPN_MAX_FLIGHT_MODES] = {ui->cb_FP0, ui->cb_FP1, ui->cb_FP2, ui->cb_FP3, ui->cb_FP4, ui->cb_FP5, ui->cb_FP6, ui->cb_FP7, ui->cb_FP8 };
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i] = tmp[i];
  }

  RawSourceType srcType = (firmware->getCapability(VirtualInputs) ? SOURCE_TYPE_VIRTUAL_INPUT : SOURCE_TYPE_INPUT);
  setWindowTitle(tr("Edit %1").arg(RawSource(srcType, ed->chn).toString(&model, &generalSettings)));

  int imId = dialogFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                                (RawSource::AllSourceGroups & ~RawSource::NoneGroup & ~RawSource::ScriptsGroup)),
                                "EditorSource");

  FilteredItemModel *esMdl = dialogFilteredItemModels->getItemModel(imId);

  weightEditor = new SourceNumRefEditor(ed->weight, ui->chkWeightUseSource, ui->sbWeightValue, ui->cboWeightSource, 100, -100, 100, 1.0,
                                        model, esMdl);

  connect(weightEditor, &SourceNumRefEditor::resized, this, [=] () { shrink(); });

  offsetEditor = new SourceNumRefEditor(ed->offset, ui->chkOffsetUseSource, ui->sbOffsetValue, ui->cboOffsetSource, 0, -100, 100, 1.0,
                                        model, esMdl);

  connect(offsetEditor, &SourceNumRefEditor::resized, this, [=] () { shrink(); });

  curveRefFilteredItemModels = new CurveRefFilteredFactory(sharedItemModels,
                                                           firmware->getCapability(HasInputDiff) ? 0 : FilteredItemModel::PositiveFilter);

  curveGroup = new CurveReferenceUIManager(ui->cboCurveType, ui->chkCurveUseSource, ui->sbCurveValue, ui->cboCurveSource, ui->cboCurveFunc,
                                           ui->imgCurve, ed->curve, model, sharedItemModels, curveRefFilteredItemModels, esMdl, this);


  connect(curveGroup, &CurveReferenceUIManager::resized, this, [=] () { shrink(); });

  imId = dialogFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                                         RawSwitch::MixesContext), "RawSwitch");
  ui->switchesCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  ui->switchesCB->setModel(dialogFilteredItemModels->getItemModel(imId));
  ui->switchesCB->setCurrentIndex(ui->switchesCB->findData(ed->swtch.toValue()));

  ui->sideCB->setCurrentIndex(ed->mode - 1);

  if (!firmware->getCapability(FlightModes)) {
    ui->label_phases->hide();
    for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
      lb_fp[i]->hide();
      cb_fp[i]->hide();
    }
  }
  else {
    ui->label_phases->setToolTip(tr("Popup menu available"));
    ui->label_phases->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->label_phases, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(label_phases_customContextMenuRequested(const QPoint &)));
    int mask = 1;
    for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
      if ((ed->flightModes & mask) == 0) {
        cb_fp[i]->setChecked(true);
      }
      mask <<= 1;
    }
    for (int i = firmware->getCapability(FlightModes); i < CPN_MAX_FLIGHT_MODES; i++) {
      lb_fp[i]->hide();
      cb_fp[i]->hide();
    }
  }

  if (firmware->getCapability(VirtualInputs)) {
    ui->inputName->setMaxLength(firmware->getCapability(InputsLength));
    int flags = RawSource::InputSourceGroups & ~RawSource::NoneGroup & ~RawSource::InputsGroup;
    flags |= RawSource::GVarsGroup | RawSource::TelemGroup;
    imId = dialogFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                                                     flags), "RawSource");
    ui->sourceCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui->sourceCB->setModel(dialogFilteredItemModels->getItemModel(imId));
    ui->sourceCB->setCurrentIndex(ui->sourceCB->findData(ed->srcRaw.toValue()));
    if (ui->sourceCB->currentIndex() < 0 && ed->srcRaw.toValue() == 0)
      ui->sourceCB->setCurrentIndex(Helpers::getFirstPosValueIndex(ui->sourceCB));
    ui->inputName->setValidator(new NameValidator(board, this));
    ui->inputName->setText(inputName);
  }
  else {
    ui->inputNameLabel->hide();
    ui->inputName->hide();
    ui->sourceLabel->hide();
    ui->sourceCB->hide();
    ui->trimLabel->hide();
    ui->trimCB->hide();
  }

  dialogFilteredItemModels->registerItemModel(new FilteredItemModel(ExpoData::carryTrimItemModel()), AIM_EXPO_CARRYTRIM);
  ui->trimCB->setModel(dialogFilteredItemModels->getItemModel(AIM_EXPO_CARRYTRIM));

  if (ed->srcRaw.isStick())
    carryTrimFilterFlags = CarryTrimSticksGroup;
  else
    carryTrimFilterFlags = CarryTrimNotSticksGroup;

  dialogFilteredItemModels->getItemModel(AIM_EXPO_CARRYTRIM)->setFilterFlags(carryTrimFilterFlags);
  ui->trimCB->setCurrentIndex(ui->trimCB->findData(ed->carryTrim));

  int expolength = firmware->getCapability(HasExpoNames);
  if (!expolength) {
    ui->lineNameLabel->hide();
    ui->lineName->hide();
  }
  else {
    ui->lineName->setMaxLength(expolength);
  }

  ui->lineName->setValidator(new NameValidator(board, this));
  ui->lineName->setText(ed->name);

  updateScale();
  valuesChanged();

  connect(ui->lineName, SIGNAL(editingFinished()), this, SLOT(valuesChanged()));
  connect(ui->sourceCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesChanged()));
  connect(ui->dsbScale, SIGNAL(editingFinished()), this, SLOT(valuesChanged()));
  connect(ui->trimCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesChanged()));
  connect(ui->switchesCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesChanged()));
  connect(ui->sideCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesChanged()));

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    connect(cb_fp[i], SIGNAL(toggled(bool)), this, SLOT(valuesChanged()));
  }

  if (firmware->getCapability(VirtualInputs)) {
    connect(ui->inputName, SIGNAL(editingFinished()), this, SLOT(valuesChanged()));
  }

  shrink();
}

ExpoDialog::~ExpoDialog()
{
  delete ui;
  delete weightEditor;
  delete offsetEditor;
  delete dialogFilteredItemModels;
  delete curveRefFilteredItemModels;
}

void ExpoDialog::updateScale()
{
  if (firmware->getCapability(VirtualInputs) && ed->srcRaw.type == SOURCE_TYPE_TELEMETRY) {
    RawSourceRange range = ed->srcRaw.getRange(&model, generalSettings);
    ui->dsbScale->setEnabled(true);
    ui->lblScaleUnit->setEnabled(true);
    ui->dsbScale->setDecimals(range.decimals);
    ui->dsbScale->setMinimum(range.min);
    ui->dsbScale->setMaximum(range.max);
    ui->dsbScale->setSingleStep(range.step);
    ui->dsbScale->setValue(range.step * ed->scale);
    ui->lblScaleUnit->setText(range.unit);
  }
  else {
    ui->dsbScale->setValue(0);
    ui->dsbScale->setEnabled(false);
    ui->lblScaleUnit->setText("");
    ui->lblScaleUnit->setEnabled(false);
  }
}

void ExpoDialog::valuesChanged()
{
  if (!lock) {
    lock = true;
    RawSource srcRaw = RawSource(ui->sourceCB->itemData(ui->sourceCB->currentIndex()).toInt());
    if (ed->srcRaw != srcRaw) {
      ed->srcRaw = srcRaw;
      if (ed->srcRaw.isStick())
        carryTrimFilterFlags = CarryTrimSticksGroup;
      else
        carryTrimFilterFlags = CarryTrimNotSticksGroup;
      ed->carryTrim = CARRYTRIM_DEFAULT;
      dialogFilteredItemModels->getItemModel(AIM_EXPO_CARRYTRIM)->setFilterFlags(carryTrimFilterFlags);
      ui->trimCB->setCurrentIndex(ui->trimCB->findData(-ed->carryTrim));
      ed->scale= 0;
      updateScale();
    }

    RawSourceRange range = srcRaw.getRange(&model, generalSettings);
    ed->scale = round(float(ui->dsbScale->value()) / range.step);
    ed->carryTrim = ui->trimCB->itemData(ui->trimCB->currentIndex()).toInt();
    ed->swtch = RawSwitch(ui->switchesCB->itemData(ui->switchesCB->currentIndex()).toInt());
    ed->mode = ui->sideCB->currentIndex() + 1;

    strcpy(ed->name, ui->lineName->text().toLatin1().data());
    if (firmware->getCapability(VirtualInputs)) {
      inputName = ui->inputName->text();
    }

    ed->flightModes = 0;
    for (int i = CPN_MAX_FLIGHT_MODES - 1; i >= 0 ; i--) {
      if (!cb_fp[i]->checkState()) {
        ed->flightModes++;
      }
      ed->flightModes <<= 1;
    }
    ed->flightModes >>= 1;

    lock = false;
  }
}

void ExpoDialog::label_phases_customContextMenuRequested(const QPoint & pos)
{
  QLabel *label = (QLabel *)sender();
  QPoint globalPos = label->mapToGlobal(pos);
  QMenu contextMenu;
  contextMenu.addAction(tr("Clear All"), this, SLOT(fmClearAll()));
  contextMenu.addAction(tr("Set All"), this, SLOT(fmSetAll()));
  contextMenu.addAction(tr("Invert All"), this, SLOT(fmInvertAll()));
  contextMenu.exec(globalPos);
}

void ExpoDialog::fmClearAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(false);
  }
  lock = false;
  valuesChanged();
}

void ExpoDialog::fmSetAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(true);
  }
  lock = false;
  valuesChanged();
}

void ExpoDialog::fmInvertAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(!cb_fp[i]->isChecked());
  }
  lock = false;
  valuesChanged();
}

void ExpoDialog::shrink()
{
  this->adjustSize();
  this->resize(0, 0);
}
