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

#include "generalsetup.h"
#include "ui_generalsetup.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"
#include "autocombobox.h"
#include "namevalidator.h"

constexpr char FIM_HATSMODE[]       {"Hats Mode"};
constexpr char FIM_STICKMODE[]      {"Stick Mode"};
constexpr char FIM_TEMPLATESETUP[]  {"Template Setup"};

GeneralSetupPanel::GeneralSetupPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
GeneralPanel(parent, generalSettings, firmware),
ui(new Ui::GeneralSetup)
{
  ui->setupUi(this);

  Board::Type board = firmware->getBoard();

  panelFilteredModels = new FilteredItemModelFactory();

  panelFilteredModels->registerItemModel(new FilteredItemModel(GeneralSettings::hatsModeItemModel()), FIM_HATSMODE);
  panelFilteredModels->registerItemModel(new FilteredItemModel(GeneralSettings::stickModeItemModel()), FIM_STICKMODE);
  panelFilteredModels->registerItemModel(new FilteredItemModel(GeneralSettings::templateSetupItemModel(),
                                                               Boards::isAir(board) ? GeneralSettings::RadioTypeContextAir :
                                                                                      GeneralSettings::RadioTypeContextSurface),
                                         FIM_TEMPLATESETUP);

  QLabel *pmsl[] = {ui->ro_label, ui->ro1_label, ui->ro2_label, ui->ro3_label, ui->ro4_label, ui->ro5_label, ui->ro6_label, ui->ro7_label, ui->ro8_label, NULL};
  QSlider *tpmsld[] = {ui->chkSA, ui->chkSB, ui->chkSC, ui->chkSD, ui->chkSE, ui->chkSF, ui->chkSG, ui->chkSH, NULL};

  if (IS_TARANIS(board)) {
    if (firmware->getId().contains("readonly")) {
      uint16_t switchstate = generalSettings.switchUnlockStates;
      ui->chkSA->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSB->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSC->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSD->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSE->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSF->setValue((switchstate & 0x3) / 2);
      switchstate >>= 2;
      ui->chkSG->setValue(switchstate & 0x3);
      switchstate >>= 2;
      ui->chkSH->setValue(switchstate & 0x3);
    }
    else {
      for (int i = 0; pmsl[i]; i++) {
        pmsl[i]->hide();
      }
      for (int i = 0; tpmsld[i]; i++) {
        tpmsld[i]->hide();
      }
      this->layout()->removeItem(ui->TaranisReadOnlyUnlock);
    }
  }
  else {
    for (int i = 0; pmsl[i]; i++) {
      pmsl[i]->hide();
    }
    for (int i = 0; tpmsld[i]; i++) {
      tpmsld[i]->hide();
    }
    this->layout()->removeItem(ui->TaranisReadOnlyUnlock);
  }

  lock = true;

  populateBacklightCB();

  if (!firmware->getCapability(MultiLangVoice)) {
    ui->VoiceLang_label->hide();
    ui->voiceLang_CB->hide();
  }
  else {
    populateVoiceLangCB();
  }

  if (!firmware->getCapability(MavlinkTelemetry)) {
    ui->mavbaud_CB->hide();
    ui->mavbaud_label->hide();
  }
  else {
    ui->mavbaud_CB->setCurrentIndex(generalSettings.mavbaud);
    // TODO why ??? populateVoiceLangCB(ui->voiceLang_CB, generalSettings.ttsLanguage);
  }

  if (!firmware->getCapability(HasSoundMixer)) {
    ui->beepVolume_SL->hide();
    ui->beepVolume_label->hide();
    ui->varioVolume_SL->hide();
    ui->varioVolume_label->hide();
    ui->bgVolume_SL->hide();
    ui->bgVolume_label->hide();
    ui->wavVolume_SL->hide();
    ui->wavVolume_label->hide();
    ui->varioP0_label->hide();
    ui->varioP0_SB->hide();
    ui->varioPMax_label->hide();
    ui->varioPMax_SB->hide();
    ui->varioR0_label->hide();
    ui->varioR0_SB->hide();
  }
  else {
    ui->beepVolume_SL->setValue(generalSettings.beepVolume);
    ui->varioVolume_SL->setValue(generalSettings.varioVolume);
    ui->bgVolume_SL->setValue(generalSettings.backgroundVolume);
    ui->wavVolume_SL->setValue(generalSettings.wavVolume);
    ui->varioP0_SB->setValue(700 + (generalSettings.varioPitch * 10));
    updateVarioPitchRange();
    ui->varioPMax_SB->setValue(700 + (generalSettings.varioPitch * 10) + 1000 + (generalSettings.varioRange * 10));
    ui->varioR0_SB->setValue(500 + (generalSettings.varioRepeat * 10));
  }

  if (!firmware->getCapability(HasFAIMode)) {
    ui->faimode_CB->hide();
    ui->label_faimode->hide();
  }
  else {
    ui->faimode_CB->setChecked(generalSettings.fai);
  }

  if (!firmware->getCapability(RotaryEncoderNavigation)) {
    ui->rotEncMode_CB->hide();
    ui->rotEncMode_label->hide();
  }
  else {
    populateRotEncModeCB();
  }

  if (!firmware->getCapability(HasPxxCountry)) {
    ui->countrycode_label->hide();
    ui->countrycode_CB->hide();
  }
  else {
    ui->countrycode_CB->setCurrentIndex(generalSettings.countryCode);
  }

  if (!firmware->getCapability(HasGeneralUnits)) {
    ui->units_label->hide();
    ui->units_CB->hide();
  }
  else {
    ui->units_CB->setCurrentIndex(generalSettings.imperial);
  }

  ui->ppm_units_CB->setCurrentIndex(generalSettings.ppmunit);

  ui->gpsFormatCB->setCurrentIndex(generalSettings.gpsFormat);

  ui->timezoneLE->setTime((generalSettings.timezone * 3600) + (generalSettings.timezoneMinutes/*quarter hours*/ * 15 * 60));

  if (IS_HORUS_OR_TARANIS(board)) {
    ui->adjustRTC->setChecked(generalSettings.adjustRTC);
  }
  else {
    ui->adjustRTC->hide();
  }

  if (IS_STM32(board)) {
    ui->usbModeCB->setCurrentIndex(generalSettings.usbMode);
  }
  else {
    ui->usbModeLabel->hide();
    ui->usbModeCB->hide();
  }

  if (IS_FLYSKY_EL18(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_PL18(board)) {
    ui->hatsModeCB->setModel(panelFilteredModels->getItemModel(FIM_HATSMODE));
    ui->hatsModeCB->setField(generalSettings.hatsMode, this);
  }
  else {
    ui->hatsModeLabel->hide();
    ui->hatsModeCB->hide();
  }

  if (firmware->getCapability(HasSwitchableJack)) {
    ui->jackModeCB->setCurrentIndex(generalSettings.jackMode);
  }
  else {
    ui->jackModeLabel->hide();
    ui->jackModeCB->hide();
  }

  if (!firmware->getCapability(OptrexDisplay)) {
    ui->label_displayType->hide();
    ui->displayTypeCB->setDisabled(true);
    ui->displayTypeCB->hide();
  }
  else {
    ui->displayTypeCB->setCurrentIndex(generalSettings.optrexDisplay);
  }

  if (!firmware->getCapability(HasVolume)) {
    ui->volume_SL->hide();
    ui->volume_SL->setDisabled(true);
    ui->label_volume->hide();
  }
  else {
    ui->volume_SL->setMaximum(firmware->getCapability(MaxVolume));
  }

  if (!firmware->getCapability(HasBrightness)) {
    ui->BLBright_SB->hide();
    ui->BLBright_SB->setDisabled(true);
    ui->label_BLBright->hide();
  }

  if (!IS_FAMILY_HORUS_OR_T16(board)) {
    ui->OFFBright_SB->hide();
    ui->OFFBright_SB->setDisabled(true);
    ui->label_OFFBright->hide();
  }

  if (!IS_JUMPER_T18(board)) {
    ui->keysBl_ChkB->hide();
    ui->keysBl_ChkB->setDisabled(true);
    ui->label_KeysBl->hide();
  }

  if (!firmware->getCapability(SoundMod)) {
    ui->soundModeCB->setDisabled(true);
    ui->label_soundMode->hide();
    ui->soundModeCB->hide();
  }

  if (!firmware->getCapability(SoundPitch)) {
    ui->speakerPitchSB->setDisabled(true);
    ui->label_speakerPitch->hide();
    ui->speakerPitchSB->hide();
  }

  if (!firmware->getCapability(Haptic)) {
    ui->hapticStrength->setDisabled(true);
    ui->hapticmodeCB->setDisabled(true);
  }

  int reCount = firmware->getCapability(RotaryEncoders);
  if (reCount == 0) {
    ui->re_label->hide();
    ui->re_CB->hide();
  }
  else {
    populateRotEncCB(reCount);
  }

  if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
    ui->backlightautoSB->setMinimum(5);
  }

  ui->contrastSB->setMinimum(firmware->getCapability(MinContrast));
  ui->contrastSB->setMaximum(firmware->getCapability(MaxContrast));
  ui->contrastSB->setValue(generalSettings.contrast);

  ui->battwarningDSB->setValue((double)generalSettings.vBatWarn / 10);
  ui->backlightautoSB->setValue(generalSettings.backlightDelay * 5);
  ui->inactimerSB->setValue(generalSettings.inactivityTimer);

  ui->memwarnChkB->setChecked(!generalSettings.disableMemoryWarning); // Default is zero=checked
  ui->alarmwarnChkB->setChecked(!generalSettings.disableAlarmWarning); // Default is zero=checked

  ui->rssiPowerOffWarnChkB->setChecked(!generalSettings.disableRssiPoweroffAlarm); // Default is zero=checked
  ui->trainerPowerOffWarnChkB->setChecked(!generalSettings.disableTrainerPoweroffAlarm); // Default is zero=checked

  ui->splashScreenDuration->setCurrentIndex(3 - generalSettings.splashMode);
  if (IS_FAMILY_HORUS_OR_T16(firmware->getBoard())) {
    ui->splashScreenDuration->setItemText(0, QCoreApplication::translate("GeneralSetup", "1s", nullptr));
  }

  if (!firmware->getCapability(PwrButtonPress)) {
    ui->pwrOnDelayLabel->hide();
    ui->pwrOnDelay->hide();
    ui->pwrOffDelayLabel->hide();
    ui->pwrOffDelay->hide();
    ui->pwrOffIfInactiveLabel->hide();
    ui->pwrOffIfInactiveSB->hide();
  }
  else if (!IS_TARANIS(board)) {
    ui->pwrOnDelayLabel->hide();
    ui->pwrOnDelay->hide();
  }

  ui->pwrOnOffHaptic_CB->setChecked(!generalSettings.disablePwrOnOffHaptic); // Default is zero=checked

  ui->registrationId->setValidator(new NameValidator(board, this));
  ui->registrationId->setMaxLength(REGISTRATION_ID_LEN);

  ui->stickmodeCB->setModel(panelFilteredModels->getItemModel(FIM_STICKMODE));
  ui->channelorderCB->setModel(panelFilteredModels->getItemModel(FIM_TEMPLATESETUP));

  setValues();

  lock = false;

  for (int i = 0; tpmsld[i]; i++) {
    connect(tpmsld[i], SIGNAL(valueChanged(int)),this,SLOT(unlockSwitchEdited()));
  }

  if (!IS_HORUS_OR_TARANIS(board)) {
    ui->stickReverse1->setChecked(generalSettings.stickReverse & (1 << 0));
    ui->stickReverse2->setChecked(generalSettings.stickReverse & (1 << 1));
    ui->stickReverse3->setChecked(generalSettings.stickReverse & (1 << 2));
    ui->stickReverse4->setChecked(generalSettings.stickReverse & (1 << 3));
    connect(ui->stickReverse1, SIGNAL(toggled(bool)), this, SLOT(stickReverseEdited()));
    connect(ui->stickReverse2, SIGNAL(toggled(bool)), this, SLOT(stickReverseEdited()));
    connect(ui->stickReverse3, SIGNAL(toggled(bool)), this, SLOT(stickReverseEdited()));
    connect(ui->stickReverse4, SIGNAL(toggled(bool)), this, SLOT(stickReverseEdited()));
  }
  else {
    ui->stickReverseLB->hide();
    ui->stickReverse1->hide();
    ui->stickReverse2->hide();
    ui->stickReverse3->hide();
    ui->stickReverse4->hide();
  }

  if (IS_TARANIS_PLUS(board)) {
    ui->backlightColor_SL->setValue(generalSettings.backlightColor);
  }
  else {
    ui->backlightColor_label->hide();
    ui->backlightColor_SL->hide();
    ui->backlightColor1_label->hide();
    ui->backlightColor2_label->hide();
  }

  ui->switchesDelay->setValue(10 * (generalSettings.switchesDelay + 15));
  ui->blAlarm_ChkB->setChecked(generalSettings.alarmsFlash);

  if (!firmware->getCapability(HasBatMeterRange)) {
    ui->batMeterRangeLabel->hide();
    ui->HasBatMeterMinRangeLabel->hide();
    ui->HasBatMeterMaxRangeLabel->hide();
    ui->vBatMinDSB->hide();
    ui->vBatMaxDSB->hide();
  }

  if (Boards::getCapability(board, Board::Surface)) {
    ui->stickModeLabel->hide();
    ui->stickmodeCB->hide();
  }

  if (!firmware->getCapability(HasVario)) {
    ui->varioVolume_label->hide();
    ui->varioVolume_SL->hide();
    ui->varioP0_label->hide();
    ui->varioP0_SB->hide();
    ui->varioPMax_label->hide();
    ui->varioPMax_SB->hide();
    ui->varioR0_label->hide();
    ui->varioR0_SB->hide();
  }

  disableMouseScrolling();
}

GeneralSetupPanel::~GeneralSetupPanel()
{
  delete ui;
  delete panelFilteredModels;

}

void GeneralSetupPanel::on_timezoneLE_textEdited(const QString &text)
{
  if (!lock) {
    int secs = ui->timezoneLE->timeInSeconds();
    generalSettings.timezone = secs / 3600;
    generalSettings.timezoneMinutes = (secs % 3600) / (15 * 60); // timezoneMinutes in quarter hours
    emit modified();
  }
}

void GeneralSetupPanel::populateBacklightCB()
{
  QComboBox * b = ui->backlightswCB;
  const QStringList strings = { tr("OFF"), tr("Keys"), tr("Controls"), tr("Keys + Controls"), tr("ON") };

  b->clear();
  int startValue = (Boards::getCapability(firmware->getBoard(), Board::LcdDepth) >= 8) ? 1 : 0;

  for (int i = startValue; i < strings.size(); i++) {
    b->addItem(strings[i], 0);
    if (generalSettings.backlightMode == i) {
      b->setCurrentIndex(b->count() - 1);
    }
  }
}

void GeneralSetupPanel::populateVoiceLangCB()
{
  QComboBox * b = ui->voiceLang_CB;
  //  Note: these align with the radio NOT computer locales - TODO harmonise with ISO and one list!!!
  static QString strings[][2] = {
    { tr("Chinese"), "cn" },
    { tr("Czech"), "cz" },
    { tr("Danish"), "da" },
    { tr("Dutch"), "nl" },
    { tr("English"), "en" },
    { tr("French"), "fr" },
    { tr("German"), "de" },
    { tr("Hebrew"), "he" },
    { tr("Hungarian"), "hu" },
    { tr("Italian"), "it" },
    { tr("Japanese"), "jp" },
    { tr("Polish"), "pl" },
    { tr("Portuguese"), "pt" },
    { tr("Russian"), "ru" },
    { tr("Slovak"), "sk" },
    { tr("Spanish"), "es" },
    { tr("Swedish"), "se" },
    { tr("Ukrainian"), "ua" },
    { NULL, NULL }};

  b->clear();
  for (int i = 0; strings[i][0] != NULL; i++) {
    b->addItem(strings[i][0],strings[i][1]);
    if (generalSettings.ttsLanguage == strings[i][1]) {
      b->setCurrentIndex(b->count() - 1);
    }
  }
}

void GeneralSetupPanel::on_backlightswCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.backlightMode = ui->backlightswCB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_usbModeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.usbMode = ui->usbModeCB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_jackModeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.jackMode = ui->jackModeCB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_backlightColor_SL_valueChanged()
{
  if (!lock) {
    generalSettings.backlightColor = ui->backlightColor_SL->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_mavbaud_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.mavbaud = ui->mavbaud_CB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_voiceLang_CB_currentIndexChanged(int index)
{
  if (!lock) {
    QString code = ui->voiceLang_CB->itemData(index).toString();
    for (int i = 0; i < 2; i++) {
      generalSettings.ttsLanguage[i] = code.at(i).toLatin1();
    }
    generalSettings.ttsLanguage[2] = '\0';
    emit modified();
  }
}

void GeneralSetupPanel::updateVarioPitchRange()
{
  ui->varioPMax_SB->setMaximum(700 + (generalSettings.varioPitch * 10) + 1000 + 800);
  ui->varioPMax_SB->setMinimum(700 + (generalSettings.varioPitch * 10) + 1000 - 800);
}

void GeneralSetupPanel::populateRotEncCB(int reCount)
{
  QString strings[] = { tr("No"), tr("RotEnc A"), tr("Rot Enc B"), tr("Rot Enc C"), tr("Rot Enc D"), tr("Rot Enc E")};
  QComboBox * b = ui->re_CB;

  b->clear();
  for (int i = 0; i <= reCount; i++) {
    b->addItem(strings[i]);
  }
  b->setCurrentIndex(generalSettings.reNavigation);
}

int pwrDelayFromYaml(int delay)
{
  static int8_t vals[] = { 1, 4, 3, 2, 0 };
  return vals[delay + 2];
}

int pwrDelayToYaml(int delay)
{
  static int8_t vals[] = { 2, -2, 1, 0, -1 };
  return vals[delay];
}

void GeneralSetupPanel::setValues()
{
  Board::Type board = firmware->getBoard();
  ui->beeperCB->setCurrentIndex(generalSettings.beeperMode+2);
  ui->channelorderCB->setCurrentIndex(ui->channelorderCB->findData(generalSettings.templateSetup));
  ui->stickmodeCB->setCurrentIndex(ui->stickmodeCB->findData(generalSettings.stickMode));
  if (firmware->getCapability(Haptic)) {
    ui->hapticLengthCB->setCurrentIndex(generalSettings.hapticLength + 2);
  }
  else {
    ui->label_HL->hide();
    ui->hapticLengthCB->hide();
  }
  ui->OFFBright_SB->setMinimum(firmware->getCapability(BacklightLevelMin));
  if (generalSettings.backlightOffBright > 100 - generalSettings.backlightBright)
    generalSettings.backlightOffBright = 100 - generalSettings.backlightBright;
  ui->BLBright_SB->setValue(100 - generalSettings.backlightBright);
  ui->OFFBright_SB->setValue(generalSettings.backlightOffBright);
  ui->BLBright_SB->setMinimum(ui->OFFBright_SB->value());
  ui->OFFBright_SB->setMaximum(ui->BLBright_SB->value());
  ui->soundModeCB->setCurrentIndex(generalSettings.speakerMode);
  ui->volume_SL->setValue(generalSettings.speakerVolume + 12);
  ui->beeperlenCB->setCurrentIndex(generalSettings.beeperLength + 2);
  ui->speakerPitchSB->setValue(generalSettings.speakerPitch);
  ui->hapticStrength->setValue(generalSettings.hapticStrength);
  ui->hapticmodeCB->setCurrentIndex(generalSettings.hapticMode + 2);

  if (firmware->getCapability(HasBatMeterRange)) {
    ui->vBatMinDSB->setValue((double)(generalSettings.vBatMin + 90) / 10);
    ui->vBatMaxDSB->setValue((double)(generalSettings.vBatMax + 120) / 10);
  }

  ui->pwrOnDelay->setCurrentIndex(pwrDelayFromYaml(generalSettings.pwrOnSpeed));
  ui->pwrOffDelay->setCurrentIndex(pwrDelayFromYaml(generalSettings.pwrOffSpeed));
  ui->pwrOffIfInactiveSB->setValue(generalSettings.pwrOffIfInactive);

  ui->registrationId->setText(generalSettings.registrationId);

  ui->startSoundCB->setChecked(!generalSettings.dontPlayHello);

  if (Boards::getCapability(board, Board::HasColorLcd)) {
    ui->modelQuickSelect_CB->setChecked(generalSettings.modelQuickSelect);
    ui->modelSelectLayout_CB->setCurrentIndex(generalSettings.modelSelectLayout);
    ui->labelSingleSelect_CB->setCurrentIndex(generalSettings.labelSingleSelect);
    ui->labelMultiMode_CB->setCurrentIndex(generalSettings.labelMultiMode);
    ui->favMultiMode_CB->setCurrentIndex(generalSettings.favMultiMode);
    showLabelSelectOptions();
  } else {
    ui->label_modelQuickSelect->hide();
    ui->modelQuickSelect_CB->hide();
    ui->label_modelSelectLayout->hide();
    ui->modelSelectLayout_CB->hide();
    ui->label_labelSingleSelect->hide();
    ui->labelSingleSelect_CB->hide();
    ui->label_labelMultiMode->hide();
    ui->labelMultiMode_CB->hide();
    ui->label_favMultiMode->hide();
    ui->favMultiMode_CB->hide();
  }
}

void GeneralSetupPanel::showLabelSelectOptions()
{
  if (generalSettings.labelSingleSelect == 0) {
    ui->label_labelMultiMode->show();
    ui->labelMultiMode_CB->show();
    if (generalSettings.labelMultiMode == 1) {
      ui->label_favMultiMode->show();
      ui->favMultiMode_CB->show();
    } else {
      ui->label_favMultiMode->hide();
      ui->favMultiMode_CB->hide();
    }
  } else {
    ui->label_labelMultiMode->hide();
    ui->labelMultiMode_CB->hide();
    ui->label_favMultiMode->hide();
    ui->favMultiMode_CB->hide();
  }
}

void GeneralSetupPanel::on_faimode_CB_stateChanged(int)
{
  if (!lock) {
    if (ui->faimode_CB->isChecked()) {
      int ret = QMessageBox::question(this, CPN_STR_APP_NAME,
       tr("If you enable FAI, only RSSI and RxBt sensors will keep working.\nThis function cannot be disabled by the radio.\nAre you sure ?") ,
       QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::Yes) {
        generalSettings.fai = true;
      }
      else {
        ui->faimode_CB->setChecked(false);
      }
    }
    else {
      generalSettings.fai = false;
    }
    emit modified();
  }
}

void GeneralSetupPanel::populateRotEncModeCB()
{
  QComboBox * b = ui->rotEncMode_CB;
  QString strings[] = { tr("Normal"), tr("Inverted"), tr("Vertical Inverted, Horizontal Normal"), tr("Vertical Inverted, Horizontal Alternate"),  tr("Normal, Edit Inverted") };
  int itemCount = 5;

  if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
    itemCount = 2;
  }

  b->clear();
  for (int i = 0; i < itemCount; i++) {
    b->addItem(strings[i], 0);
  }
  b->setCurrentIndex(generalSettings.rotEncMode);
}

void GeneralSetupPanel::on_rotEncMode_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.rotEncMode = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_speakerPitchSB_editingFinished()
{
  if (!lock) {
    generalSettings.speakerPitch = ui->speakerPitchSB->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_hapticStrength_valueChanged()
{
  if (!lock) {
    generalSettings.hapticStrength = ui->hapticStrength->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_soundModeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.speakerMode = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_splashScreenDuration_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.splashMode = 3 - index;
    emit modified();
  }
}

void GeneralSetupPanel::on_pwrOnDelay_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.pwrOnSpeed = pwrDelayToYaml(index);
    emit modified();
  }
}

void GeneralSetupPanel::on_pwrOffDelay_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.pwrOffSpeed = pwrDelayToYaml(index);
    emit modified();
  }
}

void GeneralSetupPanel::on_pwrOnOffHaptic_CB_stateChanged(int)
{
  if (!lock) {
    generalSettings.disablePwrOnOffHaptic = ui->pwrOnOffHaptic_CB->isChecked() ? 0 : 1;
    emit modified();
  }
}

void GeneralSetupPanel::on_beepVolume_SL_valueChanged()
{
  if (!lock) {
    generalSettings.beepVolume = ui->beepVolume_SL->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_wavVolume_SL_valueChanged()
{
  if (!lock) {
    generalSettings.wavVolume = ui->wavVolume_SL->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_varioVolume_SL_valueChanged()
{
  if (!lock) {
    generalSettings.varioVolume = ui->varioVolume_SL->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_bgVolume_SL_valueChanged()
{
  if (!lock) {
    generalSettings.backgroundVolume = ui->bgVolume_SL->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_varioP0_SB_editingFinished()
{
  if (!lock) {
    generalSettings.varioPitch = (ui->varioP0_SB->value() - 700) / 10;
    updateVarioPitchRange();
    emit modified();
  }
}

void GeneralSetupPanel::on_varioPMax_SB_editingFinished()
{
  if (!lock) {
    generalSettings.varioRange = (ui->varioPMax_SB->value() - (700 + (generalSettings.varioPitch * 10)) - 1000) / 10;
    emit modified();
  }
}

void GeneralSetupPanel::on_varioR0_SB_editingFinished()
{
  if (!lock) {
    generalSettings.varioRepeat = (ui->varioR0_SB->value() - 500) / 10;
    emit modified();
  }
}

void GeneralSetupPanel::on_BLBright_SB_editingFinished()
{
  if (!lock) {
    if (ui->BLBright_SB->value() < ui->OFFBright_SB->value()) {
      ui->BLBright_SB->setValue(ui->OFFBright_SB->value());
    } else {
      ui->OFFBright_SB->setMaximum(ui->BLBright_SB->value());
      generalSettings.backlightBright = 100 - ui->BLBright_SB->value();
      emit modified();
    }
  }
}

void GeneralSetupPanel::on_OFFBright_SB_editingFinished()
{
  if (!lock) {
    if (ui->OFFBright_SB->value() > ui->BLBright_SB->value()) {
      ui->OFFBright_SB->setValue(ui->BLBright_SB->value());
    } else {
      ui->BLBright_SB->setMinimum(ui->OFFBright_SB->value());
      generalSettings.backlightOffBright = ui->OFFBright_SB->value();
      emit modified();
    }
  }
}

void GeneralSetupPanel::on_volume_SL_valueChanged()
{
  if (!lock) {
    generalSettings.speakerVolume = ui->volume_SL->value() - 12;
    emit modified();
  }
}

void GeneralSetupPanel::on_contrastSB_editingFinished()
{
  if (!lock) {
    generalSettings.contrast = ui->contrastSB->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_battwarningDSB_editingFinished()
{
  if (!lock) {
    generalSettings.vBatWarn = (int)(ui->battwarningDSB->value() * 10);
    emit modified();
  }
}

void GeneralSetupPanel::on_vBatMinDSB_editingFinished()
{
  if (!lock) {
    generalSettings.vBatMin = ui->vBatMinDSB->value() * 10 - 90;
    emit modified();
  }
}

void GeneralSetupPanel::on_vBatMaxDSB_editingFinished()
{
  if (!lock) {
    generalSettings.vBatMax = ui->vBatMaxDSB->value() * 10 - 120;
    emit modified();
  }
}

void GeneralSetupPanel::on_re_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.reNavigation = ui->re_CB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_countrycode_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.countryCode = ui->countrycode_CB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_units_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.imperial = ui->units_CB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_ppm_units_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.ppmunit = ui->ppm_units_CB->currentIndex();
    emit modified();
  }
}

void GeneralSetupPanel::on_beeperlenCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.beeperLength = index - 2;
    emit modified();
  }
}

void GeneralSetupPanel::on_hapticLengthCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.hapticLength = index - 2;
    emit modified();
  }
}

void GeneralSetupPanel::on_gpsFormatCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.gpsFormat = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_backlightautoSB_editingFinished()
{
  if (!lock) {
    int i = ui->backlightautoSB->value() / 5;
    if((i * 5) != ui->backlightautoSB->value())
      ui->backlightautoSB->setValue(i * 5);
    else
    {
      generalSettings.backlightDelay = i;
      emit modified();
    }
  }
}

void GeneralSetupPanel::on_switchesDelay_valueChanged(int)
{
  if (!lock) {
    generalSettings.switchesDelay = (ui->switchesDelay->value() / 10) - 15;
    emit modified();
  }
}

void GeneralSetupPanel::on_adjustRTC_stateChanged(int)
{
  if (!lock) {
    generalSettings.adjustRTC = ui->adjustRTC->isChecked();
    emit modified();
  }
}


void GeneralSetupPanel::on_inactimerSB_editingFinished()
{
  if (!lock) {
    generalSettings.inactivityTimer = ui->inactimerSB->value();
    emit modified();
  }
}

void GeneralSetupPanel::on_pwrOffIfInactiveSB_editingFinished()
{
  generalSettings.pwrOffIfInactive = ui->pwrOffIfInactiveSB->value();
  emit modified();
}

void GeneralSetupPanel::on_memwarnChkB_stateChanged(int)
{
  if (!lock) {
    generalSettings.disableMemoryWarning = !ui->memwarnChkB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_alarmwarnChkB_stateChanged(int)
{
  if (!lock) {
    generalSettings.disableAlarmWarning = !ui->alarmwarnChkB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_rssiPowerOffWarnChkB_stateChanged(int)
{
  if (!lock) {
    generalSettings.disableRssiPoweroffAlarm = !ui->rssiPowerOffWarnChkB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_trainerPowerOffWarnChkB_stateChanged(int)
{
  if (!lock) {
    generalSettings.disableTrainerPoweroffAlarm = !ui->trainerPowerOffWarnChkB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_beeperCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.beeperMode = (GeneralSettings::BeeperMode)(index - 2);
    emit modified();
  }
}

void GeneralSetupPanel::on_displayTypeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.optrexDisplay = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_hapticmodeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.hapticMode = (GeneralSettings::BeeperMode)(index - 2);
    emit modified();
  }
}


void GeneralSetupPanel::on_channelorderCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.templateSetup = ui->channelorderCB->currentData().toInt();
    emit modified();
  }
}

void GeneralSetupPanel::on_stickmodeCB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.stickMode = ui->stickmodeCB->currentData().toInt();
    emit modified();
  }
}

void GeneralSetupPanel::unlockSwitchEdited()
{
  if (!lock) {
    int i = 0;
    i |= (((uint16_t)ui->chkSA->value()));
    i |= (((uint16_t)ui->chkSB->value()) << 2);
    i |= (((uint16_t)ui->chkSC->value()) << 4);
    i |= (((uint16_t)ui->chkSD->value()) << 6);
    i |= (((uint16_t)ui->chkSE->value()) << 8);
    i |= (((uint16_t)ui->chkSF->value()) << 10);
    i |= (((uint16_t)ui->chkSG->value()) << 12);
    i |= (((uint16_t)ui->chkSH->value()) << 14);
    generalSettings.switchUnlockStates=i;
    emit modified();
  }
}

void GeneralSetupPanel::on_blAlarm_ChkB_stateChanged()
{
  if (!lock) {
    generalSettings.alarmsFlash = ui->blAlarm_ChkB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_registrationId_editingFinished()
{
  if (!lock) {
    strncpy(generalSettings.registrationId, ui->registrationId->text().toLatin1(), REGISTRATION_ID_LEN);
    emit modified();
  }
}

void GeneralSetupPanel::stickReverseEdited()
{
  if (!lock) {
    generalSettings.stickReverse = ((int)ui->stickReverse1->isChecked()) |
                                   ((int)ui->stickReverse2->isChecked() << 1) |
                                   ((int)ui->stickReverse3->isChecked() << 2) |
                                   ((int)ui->stickReverse4->isChecked() << 3);
    emit modified();
  }
}

void GeneralSetupPanel::on_modelQuickSelect_CB_stateChanged(int)
{
  if (!lock) {
    generalSettings.modelQuickSelect = ui->modelQuickSelect_CB->isChecked();
    emit modified();
  }
}

void GeneralSetupPanel::on_modelSelectLayout_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.modelSelectLayout = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_labelSingleSelect_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.labelSingleSelect = index;
    showLabelSelectOptions();
    emit modified();
  }
}

void GeneralSetupPanel::on_labelMultiMode_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.labelMultiMode = index;
    showLabelSelectOptions();
    emit modified();
  }
}

void GeneralSetupPanel::on_favMultiMode_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.favMultiMode = index;
    emit modified();
  }
}

void GeneralSetupPanel::on_startSoundCB_stateChanged(int)
{
  if (!lock) {
    generalSettings.dontPlayHello = !ui->startSoundCB->isChecked();
    emit modified();
  }
}
