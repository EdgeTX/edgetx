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

#include "mixerdialog.h"
#include "ui_mixerdialog.h"
#include "radiodata.h"
#include "filtereditemmodels.h"
#include "helpers.h"
#include "namevalidator.h"
#include "curvereferencewidget.h"
#include "rawsourcewidget.h"

MixerDialog::MixerDialog(QWidget *parent, ModelData & model, MixData * mixdata,
            int index, GeneralSettings & generalSettings, Firmware * firmware,
            CompoundItemModelFactory * sharedItemModels) :
  QDialog(parent),
  ui(new Ui::MixerDialog),
  model(model),
  generalSettings(generalSettings),
  firmware(firmware),
  md(mixdata),
  lock(false),
  dlgFIM(new FilteredItemModelFactory())
{
  ui->setupUi(this);

  Board::Type board = firmware->getBoard();

  QLabel *lb_fp[CPN_MAX_FLIGHT_MODES] = { ui->lb_FP0, ui->lb_FP1, ui->lb_FP2,
    ui->lb_FP3, ui->lb_FP4, ui->lb_FP5, ui->lb_FP6, ui->lb_FP7, ui->lb_FP8 };

  QCheckBox *tmp[CPN_MAX_FLIGHT_MODES] = {ui->cb_FP0, ui->cb_FP1, ui->cb_FP2,
    ui->cb_FP3, ui->cb_FP4, ui->cb_FP5, ui->cb_FP6, ui->cb_FP7, ui->cb_FP8 };

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i] = tmp[i];
  }

  this->setWindowTitle(tr("DEST -> %1").arg(RawSource(SOURCE_TYPE_CH,
                       md->destCh).toString(&model, &generalSettings)));

  ui->wgtSource->init(&model, sharedItemModels, &mixdata->srcRaw,
                      (RawSource::AllSourceGroups & ~RawSource::NoneGroup));
  connect(ui->wgtSource, &RawSourceWidget::resized, this, [&] () { shrink(); });

  ui->wgtWeight->init(&model, sharedItemModels, &mixdata->weight,
                      (RawSource::AllSourceGroups & ~RawSource::NoneGroup &
                       ~RawSource::ScriptsGroup),
                      UI_FLAG_LIST_VALUE, RawSource(SOURCE_TYPE_NUMBER));
  connect(ui->wgtWeight, &RawSourceWidget::resized, this, [=] () { shrink(); });

  ui->wgtOffset->init(&model, sharedItemModels, &mixdata->offset,
                      (RawSource::AllSourceGroups & ~RawSource::NoneGroup &
                       ~RawSource::ScriptsGroup),
                      UI_FLAG_LIST_VALUE, RawSource(SOURCE_TYPE_NUMBER));
  connect(ui->wgtOffset, &RawSourceWidget::resized, this, [=] () { shrink(); });

  ui->wgtCurve->init(&model, sharedItemModels, &mixdata->curve,
                      (RawSource::AllSourceGroups & ~RawSource::NoneGroup &
                       ~RawSource::ScriptsGroup),
                      UI_FLAG_ALL);
  connect(ui->wgtCurve, &CurveReferenceWidget::resized, this, [=] () { shrink(); });

  if (index == 0 || model.mixData[index - 1].destCh != mixdata->destCh) {
    ui->mltpxCB->hide();
    ui->mltpxLbl->hide();
  }

  ui->chkIncTrim->setChecked(!md->carryTrim);

  ui->mixerName->setMaxLength(MIXDATA_NAME_LEN);
  ui->mixerName->setValidator(new NameValidator(board, this));
  ui->mixerName->setFixedWidth(Helpers::calcQLineEditWidth(MIXDATA_NAME_LEN + 2/*abitary*/));
  ui->mixerName->setText(md->name);

  if (!firmware->getCapability(FlightModes)) {
    ui->label_phases->hide();

    for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
      lb_fp[i]->hide();
      cb_fp[i]->hide();
    }
  }
  else {
    ui->label_phases->setToolTip(tr("Click to access popup menu"));
    ui->label_phases->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->label_phases, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(label_phases_customContextMenuRequested(const QPoint &)));
    int mask = 1;

    for (int i = 0; i < CPN_MAX_FLIGHT_MODES ; i++) {
      if ((md->flightModes & mask) == 0) {
        cb_fp[i]->setChecked(true);
      }

      mask <<= 1;
    }

    for (int i = firmware->getCapability(FlightModes); i < CPN_MAX_FLIGHT_MODES; i++) {
      lb_fp[i]->hide();
      cb_fp[i]->hide();
    }
  }

  int imId = dlgFIM->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                             RawSwitch::MixesContext),
                                                             "RawSwitch");
  ui->switchesCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  ui->switchesCB->setModel(dlgFIM->getItemModel(imId));
  ui->switchesCB->setCurrentIndex(ui->switchesCB->findData(md->swtch.toValue()));
  ui->warningCB->setModel(MixData::mixWarnItemModel());
  ui->warningCB->setCurrentIndex(ui->warningCB->findData(md->mixWarn));
  ui->mltpxCB->setModel(MixData::mltpxItemModel());
  ui->mltpxCB->setCurrentIndex(ui->mltpxCB->findData(md->mltpx));
  ui->delayPrecCB->setModel(MixData::precisionItemModel());
  ui->delayPrecCB->setCurrentIndex(ui->delayPrecCB->findData(md->delayPrec));
  ui->speedPrecCB->setModel(MixData::precisionItemModel());
  ui->speedPrecCB->setCurrentIndex(ui->speedPrecCB->findData(md->speedPrec));

  float range = firmware->getCapability(SlowRange);
  int scale = firmware->getCapability(SlowScale);

  if (md->delayPrec)
    scale = scale * 10;

  ui->delayDownSB->setMaximum(range / scale);
  ui->delayDownSB->setSingleStep(1.0 / scale);
  ui->delayDownSB->setDecimals(md->delayPrec + 1);
  ui->delayDownSB->setValue((float)md->delayDown / scale);
  ui->delayUpSB->setMaximum(range / scale);
  ui->delayUpSB->setSingleStep(1.0 / scale);
  ui->delayUpSB->setDecimals(md->delayPrec + 1);
  ui->delayUpSB->setValue((float)md->delayUp / scale);

  scale = firmware->getCapability(SlowScale);

  if (md->speedPrec)
    scale = scale * 10;

  ui->slowDownSB->setMaximum(range / scale);
  ui->slowDownSB->setSingleStep(1.0 / scale);
  ui->slowDownSB->setDecimals(md->speedPrec + 1);
  ui->slowDownSB->setValue((float)md->speedDown / scale);
  ui->slowUpSB->setMaximum(range / scale);
  ui->slowUpSB->setSingleStep(1.0 / scale);
  ui->slowUpSB->setDecimals(md->speedPrec + 1);
  ui->slowUpSB->setValue((float)md->speedUp/scale);

  valuesChanged();

  connect(ui->mixerName, &QLineEdit::editingFinished, this, &MixerDialog::valuesChanged);
  connect(ui->chkIncTrim, &QCheckBox::stateChanged, this, &MixerDialog::valuesChanged);
  connect(ui->switchesCB, &QComboBox::currentIndexChanged, this, &MixerDialog::valuesChanged);
  connect(ui->warningCB, &QComboBox::currentIndexChanged, this, &MixerDialog::valuesChanged);
  connect(ui->mltpxCB, &QComboBox::currentIndexChanged, this, &MixerDialog::valuesChanged);
  connect(ui->delayPrecCB, &QComboBox::currentIndexChanged, this, &MixerDialog::valuesChanged);
  connect(ui->delayDownSB, &QDoubleSpinBox::editingFinished, this, &MixerDialog::valuesChanged);
  connect(ui->delayUpSB, &QDoubleSpinBox::editingFinished, this, &MixerDialog::valuesChanged);
  connect(ui->speedPrecCB, &QComboBox::currentIndexChanged, this, &MixerDialog::valuesChanged);
  connect(ui->slowDownSB, &QDoubleSpinBox::editingFinished, this, &MixerDialog::valuesChanged);
  connect(ui->slowUpSB, &QDoubleSpinBox::editingFinished, this, &MixerDialog::valuesChanged);

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    connect(cb_fp[i], &QCheckBox::toggled, this, &MixerDialog::valuesChanged);
  }

  shrink();
}

MixerDialog::~MixerDialog()
{
  delete ui;
}

void MixerDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);

  switch (e->type()) {
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

void MixerDialog::valuesChanged()
{
  if (!lock) {
    lock = true;
    md->carryTrim = !ui->chkIncTrim->isChecked();
    md->swtch     = RawSwitch(ui->switchesCB->itemData(ui->switchesCB->currentIndex()).toInt());
    md->mixWarn   = ui->warningCB->itemData(ui->warningCB->currentIndex()).toInt();
    md->mltpx     = (MltpxValue)ui->mltpxCB->itemData(ui->mltpxCB->currentIndex()).toInt();

    float range = firmware->getCapability(SlowRange);

    // Update values based on old precision (in case it changed)
    int scale = firmware->getCapability(SlowScale);
    if (md->delayPrec) scale = scale * 10;
    md->delayDown = round(ui->delayDownSB->value() * scale);
    md->delayUp   = round(ui->delayUpSB->value() * scale);

    // Get new precision and update controls
    md->delayPrec = ui->delayPrecCB->itemData(ui->delayPrecCB->currentIndex()).toInt();
    scale = firmware->getCapability(SlowScale);
    if (md->delayPrec) scale = scale * 10;
    ui->delayDownSB->setMaximum(range / scale);
    ui->delayDownSB->setSingleStep(1.0 / scale);
    ui->delayDownSB->setDecimals((scale == 1 ? 0 : scale == 10 ? 1 : 2));
    ui->delayDownSB->setValue((float)md->delayDown / scale);
    ui->delayUpSB->setMaximum(range / scale);
    ui->delayUpSB->setSingleStep(1.0 / scale);
    ui->delayUpSB->setDecimals((scale == 1 ? 0 : scale == 10 ? 1 : 2));
    ui->delayUpSB->setValue((float)md->delayUp / scale);

    // Update values based on old precision (in case it changed)
    scale = firmware->getCapability(SlowScale);
    if (md->speedPrec) scale = scale * 10;
    md->speedDown = round(ui->slowDownSB->value() * scale);
    md->speedUp   = round(ui->slowUpSB->value() * scale);

    // Get new precion and update controls
    md->speedPrec = ui->speedPrecCB->itemData(ui->speedPrecCB->currentIndex()).toInt();
    scale = firmware->getCapability(SlowScale);
    if (md->speedPrec) scale = scale * 10;
    ui->slowDownSB->setMaximum(range / scale);
    ui->slowDownSB->setSingleStep(1.0 / scale);
    ui->slowDownSB->setDecimals((scale == 1 ? 0 : scale == 10 ? 1 : 2));
    ui->slowDownSB->setValue((float)md->speedDown / scale);
    ui->slowUpSB->setMaximum(range / scale);
    ui->slowUpSB->setSingleStep(1.0 / scale);
    ui->slowUpSB->setDecimals((scale == 1 ? 0 : scale == 10 ? 1 : 2));
    ui->slowUpSB->setValue((float)md->speedUp / scale);

    strcpy(md->name, ui->mixerName->text().toLatin1());

    md->flightModes = 0;
    for (int i = CPN_MAX_FLIGHT_MODES - 1; i >= 0 ; i--) {
      if (!cb_fp[i]->checkState()) {
        md->flightModes++;
      }
      md->flightModes <<= 1;
    }
    md->flightModes >>= 1;

    lock = false;
  }
}

void MixerDialog::label_phases_customContextMenuRequested(const QPoint & pos)
{
  QLabel *label = (QLabel *)sender();
  QPoint globalPos = label->mapToGlobal(pos);
  QMenu contextMenu;
  contextMenu.addAction(tr("Clear All"), this, SLOT(fmClearAll()));
  contextMenu.addAction(tr("Set All"), this, SLOT(fmSetAll()));
  contextMenu.addAction(tr("Invert All"), this, SLOT(fmInvertAll()));
  contextMenu.exec(globalPos);
}

void MixerDialog::fmClearAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(false);
  }
  lock = false;
  valuesChanged();
}

void MixerDialog::fmSetAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(true);
  }
  lock = false;
  valuesChanged();
}

void MixerDialog::fmInvertAll()
{
  lock = true;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    cb_fp[i]->setChecked(!cb_fp[i]->isChecked());
  }
  lock = false;
  valuesChanged();
}

void MixerDialog::shrink()
{
  this->adjustSize();
  this->resize(0, 0);
}
