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

FlightModePanel::FlightModePanel(QWidget * parent, ModelData & model, int phaseIdx, GeneralSettings & generalSettings, Firmware * firmware,
                                 FilteredItemModel * rawSwitchFilteredModel, QString radioType):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::FlightMode),
  phaseIdx(phaseIdx),
  phase(model.flightModeData[phaseIdx]),
  radioType(radioType)
{
  ui->setupUi(this);
  connectItemModelEvents(rawSwitchFilteredModel);

  ui->labelName->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->labelName->setToolTip(tr("Popup menu available"));
  connect(ui->labelName, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenuRequested(const QPoint &)));

  board = firmware->getBoard();
  fmCount = firmware->getCapability(FlightModes);
  gvCount = firmware->getCapability(Gvars);
  reCount = firmware->getCapability(RotaryEncoders);
  trimCount = Boards::getCapability(board, Board::NumTrims);

  // Flight mode name
  if (fmCount) {
    ui->name->setValidator(new NameValidator(board, this));
    ui->name->setMaxLength(firmware->getCapability(FlightModesName));
    connect(ui->name, SIGNAL(editingFinished()), this, SLOT(phaseName_editingFinished()));
  }
  else {
    ui->name->setDisabled(true);
  }

  // Flight mode switch
  if (phaseIdx > 0) {
    ui->swtch->setModel(rawSwitchFilteredModel);
    connect(ui->swtch, SIGNAL(currentIndexChanged(int)), this, SLOT(phaseSwitch_currentIndexChanged(int)));
  }
  else {
    ui->swtch->hide();
  }

  // FadeIn / FadeOut
  if (firmware->getCapability(FlightModesHaveFades)) {
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
  }
  else {
    ui->fadeIn->setDisabled(true);
    ui->fadeOut->setDisabled(true);
  }

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
    if (IS_HORUS_OR_TARANIS(board)) {
      cb->addItem(tr("Trim disabled"), -1);
      cb->addItem(tr("3POS toggle switch"), TRIM_MODE_3POS);
    }
    for (int m = 0; m < fmCount; m++) {
      if (m == phaseIdx) {
        cb->addItem(tr("Own Trim"), m * 2);
      }
      else if (phaseIdx > 0) {
        cb->addItem(tr("Use Trim from %1 Mode %2").arg(radioType).arg(m), m * 2);
        if (IS_HORUS_OR_TARANIS(board)) {
          cb->addItem(tr("Use Trim from %1 Mode %2 + Own Trim as an offset").arg(radioType).arg(m), m * 2 + 1);
        }
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

  // Rotary encoders
  if (reCount > 0) {
    QGridLayout *reLayout = new QGridLayout(ui->reGB);
    for (int i = 0; i < reCount; i++) {
      // RE label
      QLabel *label = new QLabel(ui->reGB);
      label->setText(tr("Rotary Encoder %1").arg(i + 1));
      reLayout->addWidget(label, i, 0, 1, 1);
      if (phaseIdx > 0) {
        // RE link to another RE
        reUse[i] = new QComboBox(ui->reGB);
        reUse[i]->setProperty("index", i);
        populateGvarUseCB(reUse[i], phaseIdx);
        connect(reUse[i], SIGNAL(currentIndexChanged(int)), this, SLOT(phaseREUse_currentIndexChanged(int)));
        reLayout->addWidget(reUse[i], i, 1, 1, 1);
      }
      // RE value
      reValues[i] = new QSpinBox(ui->reGB);
      reValues[i]->setProperty("index", i);
      reValues[i]->setMinimum(RENC_MIN_VALUE);
      reValues[i]->setMaximum(RENC_MAX_VALUE);
      connect(reValues[i], SIGNAL(editingFinished()), this, SLOT(phaseREValue_editingFinished()));
      reLayout->addWidget(reValues[i], i, 2, 1, 1);
    }
  }
  else {
    ui->reGB->hide();
  }

  // GVars
  if (gvCount > 0 && (firmware->getCapability(GvarsFlightModes) || phaseIdx == 0) ) {
    QGridLayout *gvLayout = new QGridLayout(ui->gvGB);

    // Column headers
    int headerCol = 1;

    if (firmware->getCapability(GvarsName)) {
      QLabel *nameLabel = new QLabel(ui->gvGB);
      nameLabel->setText(tr("Name"));
      gvLayout->addWidget(nameLabel, 0, headerCol++, 1, 1);
    }

    if (phaseIdx > 0) {
      QLabel *sourceLabel = new QLabel(ui->gvGB);
      sourceLabel->setText(tr("Value source"));
      gvLayout->addWidget(sourceLabel, 0, headerCol++, 1, 1);
    }

    QLabel *valueLabel = new QLabel(ui->gvGB);
    valueLabel->setText(tr("Value"));
    gvLayout->addWidget(valueLabel, 0, headerCol++, 1, 1);

    if (IS_HORUS_OR_TARANIS(board) && phaseIdx == 0) {
      QLabel *unitLabel = new QLabel(ui->gvGB);
      unitLabel->setText(tr("Unit"));
      gvLayout->addWidget(unitLabel, 0, headerCol++, 1, 1);

      QLabel *precLabel = new QLabel(ui->gvGB);
      precLabel->setText(tr("Prec"));
      gvLayout->addWidget(precLabel, 0, headerCol++, 1, 1);

      QLabel *minLabel = new QLabel(ui->gvGB);
      minLabel->setText(tr("Min"));
      gvLayout->addWidget(minLabel, 0, headerCol++, 1, 1);

      QLabel *maxLabel = new QLabel(ui->gvGB);
      maxLabel->setText(tr("Max"));
      gvLayout->addWidget(maxLabel, 0, headerCol++, 1, 1);

      QLabel *popupLabel = new QLabel(ui->gvGB);
      popupLabel->setText(tr("Popup enabled"));
      gvLayout->addWidget(popupLabel, 0, headerCol++, 1, 1);
    }

    for (int i = 0; i < gvCount; i++) {
      int col = 0;
      // GVar label
      QLabel *label = new QLabel(ui->gvGB);
      label->setText(tr("GV%1").arg(i + 1));
      label->setProperty("index", i);
      label->setContextMenuPolicy(Qt::CustomContextMenu);
      label->setToolTip(tr("Popup menu available"));
      connect(label, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(gvOnCustomContextMenuRequested(const QPoint &)));
      gvLayout->addWidget(label, i + 1, col++, 1, 1);
      // GVar name
      int nameLen = firmware->getCapability(GvarsName);
      if (nameLen > 0) {
        gvNames[i] = new QLineEdit(ui->gvGB);
        gvNames[i]->setProperty("index", i);
        gvNames[i]->setValidator(new NameValidator(board, this));
        gvNames[i]->setMaxLength(nameLen);
        connect(gvNames[i], SIGNAL(editingFinished()), this, SLOT(GVName_editingFinished()));
        gvLayout->addWidget(gvNames[i], i + 1, col++, 1, 1);
      }
      if (phaseIdx > 0) {
        // GVar link to another GVar
        gvUse[i] = new QComboBox(ui->gvGB);
        gvUse[i]->setProperty("index", i);
        populateGvarUseCB(gvUse[i], phaseIdx);
        connect(gvUse[i], SIGNAL(currentIndexChanged(int)), this, SLOT(phaseGVUse_currentIndexChanged(int)));
        gvLayout->addWidget(gvUse[i], i + 1, col++, 1, 1);
      }
      // GVar value
      gvValues[i] = new QDoubleSpinBox(ui->gvGB);
      gvValues[i]->setProperty("index", i);
      connect(gvValues[i], SIGNAL(editingFinished()), this, SLOT(phaseGVValue_editingFinished()));
      gvLayout->addWidget(gvValues[i], i + 1, col++, 1, 1);

      if (IS_HORUS_OR_TARANIS(board) && phaseIdx == 0) {
        // GVar unit
        gvUnit[i] = new QComboBox(ui->gvGB);
        gvUnit[i]->setProperty("index", i);
        populateGvarUnitCB(gvUnit[i]);
        connect(gvUnit[i], SIGNAL(currentIndexChanged(int)), this, SLOT(phaseGVUnit_currentIndexChanged(int)));
        gvLayout->addWidget(gvUnit[i], i + 1, col++, 1, 1);

        // GVar precision
        gvPrec[i] = new QComboBox(ui->gvGB);
        gvPrec[i]->setProperty("index", i);
        populateGvarPrecCB(gvPrec[i]);
        connect(gvPrec[i], SIGNAL(currentIndexChanged(int)), this, SLOT(phaseGVPrec_currentIndexChanged(int)));
        gvLayout->addWidget(gvPrec[i], i + 1, col++, 1, 1);

        // GVar min
        gvMin[i] = new QDoubleSpinBox(ui->gvGB);
        gvMin[i]->setProperty("index", i);
        connect(gvMin[i], SIGNAL(editingFinished()), this, SLOT(phaseGVMin_editingFinished()));
        gvLayout->addWidget(gvMin[i], i + 1, col++, 1, 1);

        // GVar max
        gvMax[i] = new QDoubleSpinBox(ui->gvGB);
        gvMax[i]->setProperty("index", i);
        connect(gvMax[i], SIGNAL(editingFinished()), this, SLOT(phaseGVMax_editingFinished()));
        gvLayout->addWidget(gvMax[i], i + 1, col++, 1, 1);

        // Popups
        gvPopups[i] = new QCheckBox(ui->gvGB);
        gvPopups[i]->setProperty("index", i);
        //gvPopups[i]->setText(tr("Popup enabled"));
        connect(gvPopups[i], SIGNAL(toggled(bool)), this, SLOT(phaseGVPopupToggled(bool)));
        gvLayout->addWidget(gvPopups[i], i + 1, col++, 1, 1);
      }
    }
  }
  else {
    ui->gvGB->hide();
    gvCount = 0;
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

  for (int i = 0; i < gvCount; i++) {
    updateGVar(i);
  }

  for (int i = 0; i < reCount; i++) {
    updateRotaryEncoder(i);
  }
}

void FlightModePanel::updateGVar(int index)
{
  lock = true;
  if (firmware->getCapability(GvarsName) > 0) {
    gvNames[index]->setText(model->gvarData[index].name);
  }
  if (phaseIdx > 0) {
    if (model->isGVarLinked(phaseIdx, index))
      gvUse[index]->setCurrentIndex(phase.gvars[index] - GVAR_MAX_VALUE);
    else
      gvUse[index]->setCurrentIndex(0);
  }
  gvValues[index]->setDisabled(model->isGVarLinked(phaseIdx, index));
  setGVSB(gvValues[index], model->gvarData[index].getMin(), model->gvarData[index].getMax(), model->getGVarValue(phaseIdx, index));
  if (IS_HORUS_OR_TARANIS(board) && phaseIdx == 0) {
    gvUnit[index]->setCurrentIndex(model->gvarData[index].unit);
    gvPrec[index]->setCurrentIndex(model->gvarData[index].prec);
    setGVSB(gvMin[index], GVAR_MIN_VALUE, model->gvarData[index].getMax(), model->gvarData[index].getMin());
    setGVSB(gvMax[index], model->gvarData[index].getMin(), GVAR_MAX_VALUE, model->gvarData[index].getMax());
    gvPopups[index]->setChecked(model->gvarData[index].popup);
  }
  lock = false;
}

void FlightModePanel::populateGvarUseCB(QComboBox * b, unsigned int phase)
{
  b->addItem(tr("Own value"));
  for (int i = 0; i < getCurrentFirmware()->getCapability(FlightModes); i++) {
    if (i != (int)phase) {
      b->addItem(tr("%1 Mode %2 value").arg(radioType).arg(i));
    }
  }
}

void FlightModePanel::setGVSB(QDoubleSpinBox * sb, int min, int max, int val)
{
  int idx = sb->property("index").toInt();
  float mul = model->gvarData[idx].multiplierGet();
  sb->setDecimals(model->gvarData[idx].prec);
  sb->setSingleStep(mul);
  sb->setSuffix(model->gvarData[idx].unitToString());
  sb->setMinimum(min * mul);
  sb->setMaximum(max * mul);
  sb->setValue(val * mul);
}

void FlightModePanel::updateRotaryEncoder(int index)
{
  lock = true;
  if (phaseIdx > 0) {
    if (model->isREncLinked(phaseIdx, index))
      reUse[index]->setCurrentIndex(phase.rotaryEncoders[index] - RENC_MAX_VALUE);
    else
      reUse[index]->setCurrentIndex(0);
  }
  reValues[index]->setDisabled(model->isREncLinked(phaseIdx, index));
  reValues[index]->setValue(model->getREncValue(phaseIdx, index));
  lock = false;
}

// TODO: pull units from GVarData
void FlightModePanel::populateGvarUnitCB(QComboBox * cb)
{
  cb->clear();
  cb->addItem("");
  cb->addItem("%");
}

// TODO: pull precision string from GVarData
void FlightModePanel::populateGvarPrecCB(QComboBox * cb)
{
  cb->clear();
  cb->addItem(tr("0._"));
  cb->addItem(tr("0.0"));
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
    if (IS_HORUS_OR_TARANIS(board))
      trimsUse[trim]->setCurrentIndex(2 + 2 * phase.trimRef[chn] + phase.trimMode[chn] - (phase.trimRef[chn] > phaseIdx ? 1 : 0));
    else
      trimsUse[trim]->setCurrentIndex(phase.trimRef[chn]);
    if (phaseIdx == 0 || phase.trimRef[chn] == phaseIdx || (IS_HORUS_OR_TARANIS(board) && phase.trimMode[chn] != 0)) {
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

void FlightModePanel::phaseGVValue_editingFinished()
{
  if (!lock) {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    int gvar = spinBox->property("index").toInt();
    phase.gvars[gvar] = spinBox->value() * model->gvarData[gvar].multiplierSet();
    updateGVar(gvar);
    emit modified();
  }
}

void FlightModePanel::GVName_editingFinished()
{
  if (!lock) {
    QLineEdit *lineedit = qobject_cast<QLineEdit*>(sender());
    int gvar = lineedit->property("index").toInt();
    memset(&model->gvarData[gvar].name, 0, sizeof(model->gvarData[gvar].name));
    strcpy(model->gvarData[gvar].name, lineedit->text().toLatin1());
    emit gvNameChanged();
    emit modified();
  }
}

void FlightModePanel::phaseGVUse_currentIndexChanged(int index)
{
  if (!lock) {
    lock = true;
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int gvar = comboBox->property("index").toInt();
    if (index == 0) {
      phase.gvars[gvar] = 0;
    }
    else {
      phase.gvars[gvar] = GVAR_MAX_VALUE + index;
    }
    if (model->isGVarLinkedCircular(phaseIdx, gvar))
      QMessageBox::warning(this, "Companion", tr("Warning: Global variable links back to itself. %1 Mode 0 value used.").arg(radioType));
    updateGVar(gvar);
    emit modified();
    lock = false;
  }
}

void FlightModePanel::phaseGVUnit_currentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int gvar = comboBox->property("index").toInt();
    model->gvarData[gvar].unit = index;
    updateGVar(gvar);
    emit modified();
  }
}

void FlightModePanel::phaseGVPrec_currentIndexChanged(int index)
{
  if (!lock) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int gvar = comboBox->property("index").toInt();
    model->gvarData[gvar].prec = index;
    updateGVar(gvar);
    emit modified();
  }
}

void FlightModePanel::phaseGVMin_editingFinished()
{
  if (!lock) {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    int gvar = spinBox->property("index").toInt();
    model->gvarData[gvar].setMin(spinBox->value());
    if (!model->isGVarLinked(phaseIdx, gvar)) {
      if (model->getGVarValuePrec(phaseIdx, gvar) < spinBox->value()) {
        phase.gvars[gvar] = model->gvarData[gvar].getMin();
      }
    }
    for (int x = 1; x < fmCount; x++) {
      if (!model->isGVarLinked(x, gvar)) {
        if (model->flightModeData[x].gvars[gvar] < model->gvarData[gvar].getMin()) {
          model->flightModeData[x].gvars[gvar] = model->gvarData[gvar].getMin();
        }
      }
    }
    updateGVar(gvar);
    emit modified();
  }
}

void FlightModePanel::phaseGVMax_editingFinished()
{
  if (!lock) {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    int gvar = spinBox->property("index").toInt();
    model->gvarData[gvar].setMax(spinBox->value());
    if (!model->isGVarLinked(phaseIdx, gvar)) {
      if (model->getGVarValuePrec(phaseIdx, gvar) > spinBox->value()) {
        phase.gvars[gvar] = model->gvarData[gvar].getMax();
      }
    }
    for (int x = 1; x < fmCount; x++) {
      if (!model->isGVarLinked(x, gvar)) {
        if (model->flightModeData[x].gvars[gvar] > model->gvarData[gvar].getMax()) {
          model->flightModeData[x].gvars[gvar] = model->gvarData[gvar].getMax();
        }
      }
    }
    updateGVar(gvar);
    emit modified();
  }
}

void FlightModePanel::phaseGVPopupToggled(bool checked)
{
  if (!lock) {
    QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
    int gvar = cb->property("index").toInt();
    model->gvarData[gvar].popup = checked;
    emit modified();
  }
}

void FlightModePanel::phaseREValue_editingFinished()
{
  if (!lock) {
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(sender());
    int re = spinBox->property("index").toInt();
    phase.rotaryEncoders[re] = spinBox->value();
    emit modified();
  }
}

void FlightModePanel::phaseREUse_currentIndexChanged(int index)
{
  if (!lock) {
    lock = true;
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    int re = comboBox->property("index").toInt();
    if (index == 0) {
      phase.rotaryEncoders[re] = 0;
    }
    else {
      phase.rotaryEncoders[re] = RENC_MAX_VALUE + index;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;
    }
    update();
    if (model->isREncLinkedCircular(phaseIdx, re))
      QMessageBox::warning(this, "Companion", tr("Warning: Rotary encoder links back to itself. %1 Mode 0 value used.").arg(radioType));
    emit modified();
    lock = false;
  }
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
    }
    else {
      if (IS_HORUS_OR_TARANIS(board)) {
        phase.trimMode[chn] = data % 2;
        phase.trimRef[chn] = data / 2;
      }
      else {
        phase.trimMode[chn] = 0;
        phase.trimRef[chn] = data / 2;
      }
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
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear %1 Mode. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
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
  }
  else {
    QComboBox * pswtch = ui->swtch;
    RawSwitch item = RawSwitch(SWITCH_TYPE_NONE);
    pswtch->setCurrentIndex(pswtch->findText(item.toString()));
    for (int i = 0; i < gvCount; i++) {
      phase.gvars[i] = phase.linkedGVarFlightModeZero(phaseIdx);
    }
    for (int i = 0; i < reCount; i++) {
      phase.rotaryEncoders[i] = phase.linkedREncFlightModeZero(phaseIdx);
    }
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_CLEAR, phaseIdx);

  update();
  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all %1 Modes. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int j = 0; j < fmCount; j++) {
    FlightModeData *fm = &model->flightModeData[j];
    fm->clear(j);
    for (int i = 0; i < gvCount; i++) {
      fm->gvars[i] = fm->linkedGVarFlightModeZero(j);
    }
    for (int i = 0; i < reCount; i++) {
      fm->rotaryEncoders[i] = fm->linkedREncFlightModeZero(j);
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
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut %1 Mode. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void FlightModePanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete %1 Mode. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
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

    for (int i = 0; i < reCount; i++) {
      if (model->isREncLinked(j, i)) {
        int idx = model->getREncFlightModeIndex(j, i);
        if (phaseIdx == 0 && j == 0)
          model->setREncFlightModeIndexToValue(j, i, j);
        else if (idx > phaseIdx)
          model->setREncFlightModeIndexToValue(j, i, idx - 1);
        else if (idx == phaseIdx)
          model->setREncFlightModeIndexToValue(j, i, 0);
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

  memmove(&model->flightModeData[phaseIdx + 1], &model->flightModeData[phaseIdx], (CPN_MAX_FLIGHT_MODES - (phaseIdx + 1)) * sizeof(FlightModeData));
  model->flightModeData[phaseIdx].clear(phaseIdx);

  for (int j = maxidx; j > 0; j--) {
    FlightModeData *fm = &model->flightModeData[j];

    for (int i = 0; i < trimCount; i++) {
      if (phaseIdx == 0 && j == 1)
        fm->trimRef[i] = j;
      else if (fm->trimRef[i] >= phaseIdx) {
        if (fm->trimRef[i] < maxidx)
          fm->trimRef[i]++;
        else {
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

    for (int i = 0; i < reCount; i++) {
      if (model->isREncLinked(j, i)) {
        int idx = model->getREncFlightModeIndex(j, i);
        if (phaseIdx == 0 && j == 1)
          model->setREncFlightModeIndexToValue(j, i, j);
        else if (idx >= phaseIdx)
          model->setREncFlightModeIndexToValue(j, i, idx + 1);
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

    //  TODO fix up linked REs but need to know source FM index to make adjustments
    for (int i = 0; i < reCount; i++) {
      if (model->isREncLinked(phaseIdx, i)) {
        int linkedidx = model->getREncFlightModeIndex(phaseIdx, i);
        if (phaseIdx == 0 || linkedidx == phaseIdx)
          model->setREncFlightModeIndexToValue(phaseIdx, i, phaseIdx);
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
    }
    else if (idx2 == 0) {
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

  for (int i = 0; i < reCount; i++) {
    if (model->isREncLinked(idx1, i)) {
      int linkedidx = model->getREncFlightModeIndex(idx1, i);
      if (linkedidx == idx1)
        model->setREncFlightModeIndexToValue(idx1, i, idx2 - shift);
      else if (linkedidx == idx2)
        model->setREncFlightModeIndexToValue(idx1, i, idx1 + shift);
      else
        model->setREncFlightModeIndexToValue(idx1, i, linkedidx);
    }

    if (model->isREncLinked(idx2, i)) {
      int linkedidx = model->getREncFlightModeIndex(idx2, i);
      if (linkedidx == idx1)
        model->setREncFlightModeIndexToValue(idx2, i, idx2 - shift);
      else if (linkedidx == idx2)
        model->setREncFlightModeIndexToValue(idx2, i, idx1 + shift);
      else
        model->setREncFlightModeIndexToValue(idx2, i, linkedidx);
    }
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_FLIGHT_MODE, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
  emit phaseDataChanged();
  emit modified();
}

void FlightModePanel::gvOnCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  gvIdx = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this, SLOT(gvCmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this, SLOT(gvCmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this, SLOT(gvCmPaste()))->setEnabled(gvHasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(gvCmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this, SLOT(gvCmInsert()))->setEnabled(gvInsertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this, SLOT(gvCmDelete()))->setEnabled(gvDeleteAllowed());
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(gvCmMoveUp()))->setEnabled(gvMoveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(gvCmMoveDown()))->setEnabled(gvMoveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(gvCmClearAll()));

  contextMenu.exec(globalPos);
}

bool FlightModePanel::gvHasClipboardData() const
{
  if ((phaseIdx == 0  && gvHasDefnClipboardData() && gvHasAllValuesClipboardData()) || (phaseIdx > 0 && gvHasValueClipboardData()))
    return true;
  return false;
}

bool FlightModePanel::gvHasDefnClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (phaseIdx == 0 && mimeData->hasFormat(MIMETYPE_GVAR_PARAMS)) {
    if (data) {
      data->clear();
      data->append(mimeData->data(MIMETYPE_GVAR_PARAMS));
    }
    return true;
  }
  return false;
}

bool FlightModePanel::gvHasValueClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_GVAR_VALUE)) {
    if (data) {
      data->clear();
      data->append(mimeData->data(MIMETYPE_GVAR_VALUE));
    }
    return true;
  }
  return false;
}

bool FlightModePanel::gvHasAllValuesClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (phaseIdx == 0 && mimeData->hasFormat(MIMETYPE_GVAR_ALL_VALUES)) {
    if (data) {
      data->clear();
      data->append(mimeData->data(MIMETYPE_GVAR_ALL_VALUES));
    }
    return true;
  }
  return false;
}

bool FlightModePanel::gvDeleteAllowed() const
{
  return phaseIdx == 0;
}

bool FlightModePanel::gvInsertAllowed() const
{
  bool ret = true;
  if (phaseIdx == 0) {
    if (!model->gvarData[gvCount - 1].isEmpty())
      ret = false;
    for (int i = 0; i < fmCount; i++) {
      if (model->flightModeData[i].gvars[gvCount - 1] != phase.linkedGVarFlightModeZero(i))
        ret = false;
    }
  }
  else
    ret = false;

  return ret;
}

bool FlightModePanel::gvMoveDownAllowed() const
{
  return (phaseIdx == 0 && gvIdx < gvCount - 1);
}

bool FlightModePanel::gvMoveUpAllowed() const
{
  return (phaseIdx == 0 && gvIdx > 0);
}

void FlightModePanel::gvCmClear(bool prompt)
{
  if (phaseIdx == 0) {
    if (prompt) {
      if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Global Variable across all %1 Modes. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;
    }
    model->gvarData[gvIdx].clear();
    for (int i = 0; i < fmCount; i++) {
      model->flightModeData[i].gvars[gvIdx] = model->flightModeData[i].linkedGVarFlightModeZero(i);
    }
    model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_CLEAR, gvIdx);
  }
  else {
    if (prompt) {
      if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Global Variable. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;
    }
    phase.gvars[gvIdx] = phase.linkedGVarFlightModeZero(phaseIdx);
  }
  updateGVar(gvIdx);
  emit gvNameChanged();
  emit modified();
}

void FlightModePanel::gvCmClearAll()
{
  if (phaseIdx == 0) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Global Variables for all %1 Modes. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
    for (int i = 0; i < gvCount; i++) {
      model->gvarData[i].clear();
      for (int j = 0; j < fmCount; j++) {
        model->flightModeData[j].gvars[i] = model->flightModeData[j].linkedGVarFlightModeZero(j);
      }
      model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_CLEAR, i);
    }
  }
  else {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Global Variables for this %1 Mode. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
    for (int i = 0; i < gvCount; i++) {
      phase.gvars[i] = phase.linkedGVarFlightModeZero(phaseIdx);
    }
  }
  for (int i = 0; i < gvCount; i++) {
    updateGVar(i);
  }
  emit gvNameChanged();
  emit modified();
}

void FlightModePanel::gvCmCopy()
{
  QMimeData *mimeData = new QMimeData;
  QByteArray data;
  if (phaseIdx == 0) {
    data.append((char*)&model->gvarData[gvIdx], sizeof(GVarData));
    mimeData->setData(MIMETYPE_GVAR_PARAMS, data);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    data.clear();
    for (int j = 0; j < fmCount; j++) {
      data.append((char*)&model->flightModeData[j].gvars[gvIdx], sizeof(phase.gvars[0]));
    }
    mimeData->setData(MIMETYPE_GVAR_ALL_VALUES, data);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    mimeData->removeFormat(MIMETYPE_GVAR_VALUE);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
  }
  else {
    mimeData->removeFormat(MIMETYPE_GVAR_PARAMS);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    mimeData->removeFormat(MIMETYPE_GVAR_ALL_VALUES);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    data.clear();
    int val = phase.gvars[gvIdx];
    if (model->isGVarLinked(phaseIdx, gvIdx))
      val = GVAR_MAX_VALUE + 1 + model->getGVarFlightModeIndex(phaseIdx, gvIdx);  //  store index in case paste is to another flight mode
    data.setNum(val);
    mimeData->setData(MIMETYPE_GVAR_VALUE, data);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
  }
}

void FlightModePanel::gvCmCut()
{
  if (phaseIdx == 0) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Global Variable across all %1 Modes. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }
  else {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Global Variable. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  gvCmCopy();
  gvCmClear(false);
}

void FlightModePanel::gvCmDelete()
{
  if (phaseIdx > 0)
    return;

  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Global Variable. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  int maxidx = gvCount - 1;
  for (int i = gvIdx; i < maxidx; i++) {
    memcpy(&model->gvarData[i], &model->gvarData[i + 1], sizeof(GVarData));
    for (int j = 0; j < fmCount; j++) {
      model->flightModeData[j].gvars[i] = model->flightModeData[j].gvars[i + 1];
    }
  }

  model->gvarData[maxidx].clear();
  for (int j = 0; j < fmCount; j++) {
    model->flightModeData[j].gvars[maxidx] = model->flightModeData[j].linkedGVarFlightModeZero(j);
  }

  model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_SHIFT, gvIdx, 0, -1);

  for (int i = 0; i < gvCount; i++) {
    updateGVar(i);
  }
  emit gvNameChanged();
  emit modified();
}

void FlightModePanel::gvCmInsert()
{
  for (int i = (gvCount - 1); i > gvIdx; i--) {
    memcpy(&model->gvarData[i], &model->gvarData[i - 1], sizeof(GVarData));
    for (int j = 0; j < fmCount; j++) {
      model->flightModeData[j].gvars[i] = model->flightModeData[j].gvars[i - 1];
    }
  }
  model->gvarData[gvIdx].clear();
  for (int j = 0; j < fmCount; j++) {
    model->flightModeData[j].gvars[gvIdx] = model->flightModeData[j].linkedGVarFlightModeZero(j);
  }
  model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_SHIFT, gvIdx, 0, 1);
  for (int i = 0; i < gvCount; i++) {
    updateGVar(i);
  }
  emit gvNameChanged();
  emit modified();
}

void FlightModePanel::gvCmMoveDown()
{
  if (phaseIdx == 0)
    gvSwapData(gvIdx, gvIdx + 1);
}

void FlightModePanel::gvCmMoveUp()
{
  if (phaseIdx == 0)
    gvSwapData(gvIdx, gvIdx - 1);
}

void FlightModePanel::gvCmPaste()
{
  QByteArray data;
  if (phaseIdx == 0) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Paste to selected Global Variable across all %1 Modes. Are you sure?").arg(radioType), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
    if (gvHasDefnClipboardData(&data)) {
      memcpy(&model->gvarData[gvIdx], data.constData(), sizeof(GVarData));
    }
    if (gvHasAllValuesClipboardData(&data)) {
      for (int j = 0; j < fmCount; j++) {
        int *gvar = &model->flightModeData[j].gvars[gvIdx];
        memcpy(gvar, data.mid(j * sizeof(phase.gvars[0]), sizeof(phase.gvars[0])).constData(), sizeof(phase.gvars[0]));
      }
    }
  }
  else if (gvHasValueClipboardData(&data)) {
    int val = data.toInt();
    if (val > GVAR_MAX_VALUE) {
      if (phaseIdx > 0)
        model->setGVarFlightModeIndexToValue(phaseIdx, gvIdx, val - (GVAR_MAX_VALUE + 1));    //  index stored in clipboard
      else
        phase.gvars[gvIdx] = 0;
    }
    else
      phase.gvars[gvIdx] = val;
  }
  updateGVar(gvIdx);
  emit gvNameChanged();
  emit modified();
}

void FlightModePanel::gvSwapData(int idx1, int idx2)
{
  if (phaseIdx == 0 && idx1 != idx2) {
    GVarData datatmp = model->gvarData[idx2];
    GVarData *data1 = &model->gvarData[idx1];
    GVarData *data2 = &model->gvarData[idx2];
    memcpy(data2, data1, sizeof(GVarData));
    memcpy(data1, &datatmp, sizeof(GVarData));
    for (int j = 0; j < fmCount; j++) {
      int valtmp = model->flightModeData[j].gvars[idx2];
      model->flightModeData[j].gvars[idx2] = model->flightModeData[j].gvars[idx1];
      model->flightModeData[j].gvars[idx1] = valtmp;
    }
    model->updateAllReferences(ModelData::REF_UPD_TYPE_GLOBAL_VARIABLE, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
    updateGVar(idx1);
    updateGVar(idx2);
    emit gvNameChanged();
    emit modified();
  }
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

FlightModesPanel::FlightModesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                                   CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels)
{
  rawSwitchFilteredModel = new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch), RawSwitch::MixesContext);
  connectItemModelEvents(rawSwitchFilteredModel);

  modesCount = firmware->getCapability(FlightModes);
  radioType = Boards::getRadioTypeString(firmware->getBoard());

  QGridLayout * gridLayout = new QGridLayout(this);
  tabWidget = new QTabWidget(this);

  for (int i = 0; i < modesCount; i++) {
    FlightModePanel * tab = new FlightModePanel(tabWidget, model, i, generalSettings, firmware, rawSwitchFilteredModel, radioType);
    tab->setProperty("index", i);
    connect(tab, &FlightModePanel::modified,           this, &FlightModesPanel::modified);
    connect(tab, &FlightModePanel::phaseDataChanged,   this, &FlightModesPanel::onPhaseNameChanged);
    connect(tab, &FlightModePanel::phaseNameChanged,   this, &FlightModesPanel::onPhaseNameChanged);
    connect(tab, &FlightModePanel::phaseSwitchChanged, this, &FlightModesPanel::updateItemModels);
    connect(tab, &FlightModePanel::gvNameChanged,      this, &FlightModesPanel::updateItemModels);

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
  QString result = tr("%1 Mode %2").arg(radioType).arg(index);
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
  sharedItemModels->update(AbstractItemModel::IMUE_GVars);
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
