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

#include "generaledit.h"
#include "ui_generaledit.h"
#include "helpers.h"
#include "appdata.h"
#include "generalsetup.h"
#include "trainer.h"
#include "calibration.h"
#include "hardware.h"
#include "../modeledit/customfunctions.h"
#include "generaloptions.h"
#include "verticalscrollarea.h"
#include "compounditemmodels.h"

GeneralEdit::GeneralEdit(QWidget * parent, RadioData & radioData, Firmware * firmware) :
  QDialog(parent),
  ui(new Ui::GeneralEdit),
  generalSettings(radioData.generalSettings),
  firmware(firmware)
{
  ui->setupUi(this);
  this->setWindowIcon(CompanionIcon("open.png"));

  QString firmware_id = g.profile[g.id()].fwType();
  QString name=g.profile[g.id()].name();
  if (name.isEmpty()) {
    ui->calstore_PB->setDisabled(true);
  }

  ui->profile_CB->clear();
  for (int i = 0; i < MAX_PROFILES; ++i) {
    QString name=g.profile[i].name();
    if (!name.isEmpty()) {
      ui->profile_CB->addItem(name, i);
      if (i == g.id()) {
        ui->profile_CB->setCurrentIndex(ui->profile_CB->count() - 1);
      }
    }
  }

  editorItemModels = new CompoundItemModelFactory(&generalSettings, nullptr);
  // tabs created below expect these item models to be pre-registered
  editorItemModels->addItemModel(AbstractItemModel::IMID_RawSource);
  editorItemModels->addItemModel(AbstractItemModel::IMID_RawSwitch);
  editorItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncAction);
  editorItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncResetParam);

  addTab(new GeneralSetupPanel(this, generalSettings, firmware), tr("Setup"));
  addTab(new CustomFunctionsPanel(this, nullptr, generalSettings, firmware, editorItemModels), tr("Global Functions"));
  addTab(new TrainerPanel(this, generalSettings, firmware, editorItemModels), tr("Trainer"));
  auto hwpnl = new HardwarePanel(this, generalSettings, firmware, editorItemModels);
  addTab(hwpnl, tr("Hardware"));
  addTab(new CalibrationPanel(this, generalSettings, firmware), tr("Calibration"));
  addTab(new GeneralOptionsPanel(this, generalSettings, firmware), tr("Enabled Features"));

  connect(hwpnl, &HardwarePanel::internalModuleChanged, this, [&] { intModChanged = true; });

  ui->tabWidget->setCurrentIndex( g.generalEditTab() );
}

GeneralEdit::~GeneralEdit()
{
  delete ui;
  delete editorItemModels;  // cleans up item models registered by the tabs
}

void GeneralEdit::closeEvent(QCloseEvent *event)
{
  g.generalEditTab(ui->tabWidget->currentIndex());
  if (intModChanged)
    emit internalModuleChanged(); // for MidiChild to trap
}

void GeneralEdit::addTab(GenericPanel *panel, QString text)
{
  panels << panel;
  QWidget * widget = new QWidget(ui->tabWidget);
  QVBoxLayout *baseLayout = new QVBoxLayout(widget);
  VerticalScrollArea * area = new VerticalScrollArea(widget, panel);
  baseLayout->addWidget(area);
  ui->tabWidget->addTab(widget, text);
  connect(panel, &GenericPanel::modified, this, &GeneralEdit::onTabModified);
}

void GeneralEdit::onTabModified()
{
  emit modified();
}

void GeneralEdit::on_tabWidget_currentChanged(int index)
{
  panels[index]->update();
}

void GeneralEdit::on_calretrieve_PB_clicked()
{
  Board::Type board = getCurrentBoard();
  int profile_id = ui->profile_CB->itemData(ui->profile_CB->currentIndex()).toInt();
  QString calib = g.profile[profile_id].stickPotCalib();
  int ttlSticks = getBoardCapability(board, Board::Sticks);
  int ttlInputs = getBoardCapability(board, Board::Inputs);
  int ttlSwitches = getBoardCapability(board, Board::Switches);
  if (calib.isEmpty()) {
    return;
  }
  else {
    QString trainercalib = g.profile[profile_id].trainerCalib();
    QString hwtypes = g.profile[profile_id].controlTypes();
    QString controlNames = g.profile[profile_id].controlNames();
    int8_t txVoltageCalibration = (int8_t)g.profile[profile_id].txVoltageCalibration();
    int8_t txCurrentCalibration = (int8_t)g.profile[profile_id].txCurrentCalibration();
    int8_t PPM_Multiplier = (int8_t)g.profile[profile_id].ppmMultiplier();
    uint8_t GSStickMode = (uint8_t)g.profile[profile_id].gsStickMode();
    uint8_t vBatWarn = (uint8_t)g.profile[profile_id].vBatWarn();
    QString DisplaySet = g.profile[profile_id].display();
    QString BeeperSet = g.profile[profile_id].beeper();
    QString HapticSet = g.profile[profile_id].haptic();
    QString SpeakerSet = g.profile[profile_id].speaker();
    QString CountrySet = g.profile[profile_id].countryCode();

    if ((calib.length() == (Boards::getInputsCalibrated() * 12)) && (trainercalib.length() == 16)) {
      QString Byte;
      int16_t byte16;
      bool ok;
      for (int i = 0; i < ttlInputs; i++) {
        if (Boards::isInputCalibrated(i)) {
          Byte = calib.mid(i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) generalSettings.inputConfig[i].calib.mid = byte16;
          Byte = calib.mid(4 + i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) generalSettings.inputConfig[i].calib.spanNeg = byte16;
          Byte = calib.mid(8 + i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) generalSettings.inputConfig[i].calib.spanPos = byte16;
        }
      }
      for (int i = 0; i < ttlSticks; i++) {
        Byte = trainercalib.mid(i * 4, 4);
        byte16 = (int16_t)Byte.toInt(&ok, 16);
        if (ok) generalSettings.trainer.calib[i] = byte16;
      }
      generalSettings.txCurrentCalibration = txCurrentCalibration;
      generalSettings.txVoltageCalibration = txVoltageCalibration;
      generalSettings.vBatWarn=vBatWarn;
      if (getCurrentFirmware()->getCapability(HasBatMeterRange)) {
        generalSettings.vBatMin = (int8_t) g.profile[profile_id].vBatMin();
        generalSettings.vBatMax = (int8_t) g.profile[profile_id].vBatMax();
      }
      generalSettings.PPM_Multiplier = PPM_Multiplier;
    } else {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Wrong data in profile, radio calibration was not retrieved"));
    }
    if (hwtypes.length() == ttlSwitches + ttlInputs) {
      QString Byte;
      int16_t byte16;
      QByteArray qba;
      bool ok;
      for (int i = 0; i < ttlSwitches; i++) {
        Byte = hwtypes.mid(i, 1);
        byte16 = (int16_t)Byte.toInt(&ok, 16);
        qba = controlNames.mid(3 * i, 3).toLatin1();
        if (ok) {
          generalSettings.switchConfig[i].type = (Board::SwitchType)byte16;
          strcpy(generalSettings.switchConfig[i].name, qba.data());
        }
      }
      for (int i = 0; i < getBoardCapability(board, Board::Inputs); i++) {
        Byte = hwtypes.mid(i + ttlSwitches, 1);
        byte16 = (int16_t)Byte.toInt(&ok, 16);
        qba = controlNames.mid(3 * (i + ttlSwitches), 3).toLatin1();
        if (ok) {
          generalSettings.inputConfig[i].flexType = (Board::FlexType)byte16;
          strcpy(generalSettings.inputConfig[i].name, qba.data());
        }
      }
    } else {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Wrong data in profile, Switch/pot config not retrieved"));
    }
    if ((DisplaySet.length()==6) && (BeeperSet.length()==4) && (HapticSet.length()==6) && (SpeakerSet.length()==6)) {
      generalSettings.stickMode=GSStickMode;
      uint8_t byte8u;
      int8_t byte8;
      QString chars;
      bool ok;
      byte8 = (int8_t)DisplaySet.mid(0,2).toInt(&ok, 16);
      if (ok) generalSettings.optrexDisplay = (byte8 == 1 ? true : false);
      byte8u = (uint8_t)DisplaySet.mid(2,2).toUInt(&ok, 16);
      if (ok) generalSettings.contrast = byte8u;
      byte8u = (uint8_t)DisplaySet.mid(4,2).toUInt(&ok, 16);
      if (ok) generalSettings.backlightBright = byte8u;
      byte8 = (int8_t)BeeperSet.mid(0,2).toUInt(&ok, 16);
      if (ok) generalSettings.beeperMode = (GeneralSettings::BeeperMode)byte8;
      byte8 = (int8_t)BeeperSet.mid(2,2).toInt(&ok, 16);
      if (ok) generalSettings.beeperLength = byte8;
      byte8 = (int8_t)HapticSet.mid(0,2).toUInt(&ok, 16);
      if (ok) generalSettings.hapticMode = (GeneralSettings::BeeperMode)byte8;
      byte8 = (int8_t)HapticSet.mid(2,2).toInt(&ok, 16);
      if (ok) generalSettings.hapticStrength = byte8;
      byte8 = (int8_t)HapticSet.mid(4,2).toInt(&ok, 16);
      if (ok) generalSettings.hapticLength = byte8;
      byte8u = (uint8_t)SpeakerSet.mid(0,2).toUInt(&ok, 16);
      if (ok) generalSettings.speakerMode = byte8u;
      byte8u = (uint8_t)SpeakerSet.mid(2,2).toUInt(&ok, 16);
      if (ok) generalSettings.speakerPitch = byte8u;
      byte8u = (uint8_t)SpeakerSet.mid(4,2).toUInt(&ok, 16);
      if (ok) generalSettings.speakerVolume = byte8u;
      if (CountrySet.length() == 6) {
        byte8u = (uint8_t)CountrySet.mid(0,2).toUInt(&ok, 16);
        if (ok) generalSettings.countryCode = byte8u;
        byte8u = (uint8_t)CountrySet.mid(2,2).toUInt(&ok, 16);
        if (ok) generalSettings.imperial = byte8u;
        chars = CountrySet.mid(4,2);
        generalSettings.ttsLanguage[0] = chars[0].toLatin1();
        generalSettings.ttsLanguage[1] = chars[1].toLatin1();
      }
    }
    else {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Wrong data in profile, hw related parameters were not retrieved"));
    }
  }

  emit modified();
}

void GeneralEdit::on_calstore_PB_clicked()
{
  Board::Type board = getCurrentBoard();
  int profile_id = ui->profile_CB->itemData(ui->profile_CB->currentIndex()).toInt();

  QString name = g.profile[profile_id].name();
  if (name.isEmpty()) {
    ui->calstore_PB->setDisabled(true);
    return;
  }
  else {
    QString calib = g.profile[profile_id].stickPotCalib();
    QString hwtypes = g.profile[profile_id].controlTypes();
    QString controlNames = g.profile[profile_id].controlNames();
    if (!(calib.isEmpty())) {
      int ret = QMessageBox::question(this, CPN_STR_APP_NAME,
                      tr("Do you want to store calibration in %1 profile<br>overwriting existing calibration?").arg(name) ,
                      QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::No) {
        return;
      }
    }

    int ttlSticks = getBoardCapability(board, Board::Sticks);
    int ttlInputs = getBoardCapability(board, Board::Inputs);
    int ttlSwitches = getBoardCapability(board, Board::Switches);
    calib.clear();

    for (int i = 0; i < ttlInputs; i++) {
      if (Boards::isInputCalibrated(i)) {
        calib.append(QString("%1").arg((uint16_t)generalSettings.inputConfig[i].calib.mid, 4, 16, QChar('0')));
        calib.append(QString("%1").arg((uint16_t)generalSettings.inputConfig[i].calib.spanNeg, 4, 16, QChar('0')));
        calib.append(QString("%1").arg((uint16_t)generalSettings.inputConfig[i].calib.spanPos, 4, 16, QChar('0')));
      }
    }
    g.profile[profile_id].stickPotCalib( calib );
    calib.clear();
    for (int i = 0; i < ttlSticks; i++) {
      calib.append(QString("%1").arg((uint16_t)generalSettings.trainer.calib[i], 4, 16, QChar('0')));
    }
    g.profile[profile_id].trainerCalib( calib );
    hwtypes.clear();
    controlNames.clear();
    for (int i = 0; i < ttlSwitches; i++) {
      hwtypes.append(QString("%1").arg((uint16_t)generalSettings.switchConfig[i].type, 1));
      controlNames.append(QString("%1").arg(generalSettings.switchConfig[i].name, -3));
    }
    for (int i = 0; i < ttlInputs; i++) {
      hwtypes.append(QString("%1").arg((uint16_t)generalSettings.inputConfig[i].flexType, 1));
      controlNames.append(QString("%1").arg(generalSettings.inputConfig[i].name, -3));
    }
    g.profile[profile_id].controlTypes( hwtypes );
    g.profile[profile_id].controlNames( controlNames );
    g.profile[profile_id].txVoltageCalibration( generalSettings.txVoltageCalibration );
    g.profile[profile_id].txCurrentCalibration( generalSettings.txCurrentCalibration );
    g.profile[profile_id].vBatWarn( generalSettings.vBatWarn );
    if (getCurrentFirmware()->getCapability(HasBatMeterRange)) {
      g.profile[profile_id].vBatMin( generalSettings.vBatMin );
      g.profile[profile_id].vBatMax( generalSettings.vBatMax );
    }
    g.profile[profile_id].ppmMultiplier( generalSettings.PPM_Multiplier );
    g.profile[profile_id].gsStickMode( generalSettings.stickMode );
    g.profile[profile_id].display( QString("%1%2%3").arg((generalSettings.optrexDisplay ? 1:0), 2, 16, QChar('0')).arg((uint8_t)generalSettings.contrast, 2, 16, QChar('0')).arg((uint8_t)generalSettings.backlightBright, 2, 16, QChar('0')) );
    g.profile[profile_id].beeper( QString("%1%2").arg(((uint8_t)generalSettings.beeperMode), 2, 16, QChar('0')).arg((uint8_t)generalSettings.beeperLength, 2, 16, QChar('0')));
    g.profile[profile_id].haptic( QString("%1%2%3").arg(((uint8_t)generalSettings.hapticMode), 2, 16, QChar('0')).arg((int8_t)generalSettings.hapticStrength, 2, 16, QChar('0')).arg((uint8_t)generalSettings.hapticLength, 2, 16, QChar('0')));
    g.profile[profile_id].speaker( QString("%1%2%3").arg((uint8_t)generalSettings.speakerMode, 2, 16, QChar('0')).arg((uint8_t)generalSettings.speakerPitch, 2, 16, QChar('0')).arg((uint8_t)generalSettings.speakerVolume, 2, 16, QChar('0')));
    g.profile[profile_id].countryCode( QString("%1%2%3").arg((uint8_t)generalSettings.countryCode, 2, 16, QChar('0')).arg((uint8_t)generalSettings.imperial, 2, 16, QChar('0')).arg(generalSettings.ttsLanguage));

    QDateTime dateTime = QDateTime::currentDateTime();
    g.profile[profile_id].timeStamp(dateTime.toString("yyyy-MM-dd hh:mm"));
    QMessageBox::information(this, CPN_STR_APP_NAME, tr("Calibration and HW parameters saved."));
  }
}
