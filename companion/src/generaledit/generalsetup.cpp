/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

GeneralSetupPanel::GeneralSetupPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
GeneralPanel(parent, generalSettings, firmware),
ui(new Ui::GeneralSetup)
{
  ui->setupUi(this);

  QLabel *pmsl[] = {ui->ro_label, ui->ro1_label, ui->ro2_label, ui->ro3_label, ui->ro4_label, ui->ro5_label, ui->ro6_label, ui->ro7_label, ui->ro8_label, NULL};
  QSlider *tpmsld[] = {ui->chkSA, ui->chkSB, ui->chkSC, ui->chkSD, ui->chkSE, ui->chkSF, ui->chkSG, ui->chkSH, NULL};

  if (IS_TARANIS(firmware->getBoard())) {
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

  ui->gpsFormatCB->setCurrentIndex(generalSettings.gpsFormat);
  ui->timezoneSB->setValue(generalSettings.timezone);

  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    ui->adjustRTC->setChecked(generalSettings.adjustRTC);
  }
  else {
    ui->adjustRTC->hide();
  }

  if (IS_STM32(firmware->getBoard())) {
    ui->usbModeCB->setCurrentIndex(generalSettings.usbMode);
  }
  else {
    ui->usbModeLabel->hide();
    ui->usbModeCB->hide();
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
    ui->volume_SB->hide();
    ui->volume_SB->setDisabled(true);
    ui->label_volume->hide();
  }
  else {
    ui->volume_SB->setMaximum(firmware->getCapability(MaxVolume));
  }

  if (!firmware->getCapability(HasBrightness)) {
    ui->BLBright_SB->hide();
    ui->BLBright_SB->setDisabled(true);
    ui->label_BLBright->hide();
  }

  if (!IS_FAMILY_HORUS_OR_T16(firmware->getBoard())) {
    ui->OFFBright_SB->hide();
    ui->OFFBright_SB->setDisabled(true);
    ui->label_OFFBright->hide();
  }

  if (!IS_JUMPER_T18(firmware->getBoard())) {
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

  ui->contrastSB->setMinimum(firmware->getCapability(MinContrast));
  ui->contrastSB->setMaximum(firmware->getCapability(MaxContrast));
  ui->contrastSB->setValue(generalSettings.contrast);

  ui->battwarningDSB->setValue((double)generalSettings.vBatWarn/10);
  ui->backlightautoSB->setValue(generalSettings.backlightDelay*5);
  ui->inactimerSB->setValue(generalSettings.inactivityTimer);

  ui->memwarnChkB->setChecked(!generalSettings.disableMemoryWarning); // Default is zero=checked
  ui->alarmwarnChkB->setChecked(!generalSettings.disableAlarmWarning); // Default is zero=checked

  ui->rssiPowerOffWarnChkB->setChecked(!generalSettings.disableRssiPoweroffAlarm); // Default is zero=checked

  if (IS_FAMILY_HORUS_OR_T16(firmware->getBoard())) {
    ui->splashScreenChkB->hide();
    ui->splashScreenDuration->hide();
    ui->splashScreenLabel->hide();
  }
  if (IS_TARANIS(firmware->getBoard())) {
    ui->splashScreenChkB->hide();
    ui->splashScreenDuration->setCurrentIndex(3-generalSettings.splashDuration);
  }
  else {
    ui->splashScreenDuration->hide();
    ui->splashScreenChkB->setChecked(!generalSettings.splashMode);
  }

  if (!firmware->getCapability(PwrButtonPress)) {
    ui->pwrOnDelayLabel->hide();
    ui->pwrOnDelay->hide();
    ui->pwrOffDelayLabel->hide();
    ui->pwrOffDelay->hide();
  }
  else if (!IS_TARANIS(firmware->getBoard())) {
    ui->pwrOnDelayLabel->hide();
    ui->pwrOnDelay->hide();
  }

  setValues();

  lock = false;

  for (int i=0; tpmsld[i]; i++) {
    connect(tpmsld[i], SIGNAL(valueChanged(int)),this,SLOT(unlockSwitchEdited()));
  }

  if (!IS_HORUS_OR_TARANIS(firmware->getBoard())) {
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

  if (IS_TARANIS_PLUS(firmware->getBoard())) {
    ui->backlightColor_SL->setValue(generalSettings.backlightColor);
  }
  else {
    ui->backlightColor_label->hide();
    ui->backlightColor_SL->hide();
    ui->backlightColor1_label->hide();
    ui->backlightColor2_label->hide();
  }

  ui->switchesDelay->setValue(10*(generalSettings.switchesDelay+15));
  ui->blAlarm_ChkB->setChecked(generalSettings.flashBeep);

  if (!firmware->getCapability(HasBatMeterRange)) {
    ui->batMeterRangeLabel->hide();
    ui->HasBatMeterMinRangeLabel->hide();
    ui->HasBatMeterMaxRangeLabel->hide();
    ui->vBatMinDSB->hide();
    ui->vBatMaxDSB->hide();
  }

  disableMouseScrolling();
}

GeneralSetupPanel::~GeneralSetupPanel()
{
  delete ui;
}

void GeneralSetupPanel::populateBacklightCB()
{
  QComboBox * b = ui->backlightswCB;
  QString strings[] = { tr("OFF"), tr("Keys"), tr("Sticks"), tr("Keys + Sticks"), tr("ON"), NULL };

  b->clear();
  int startValue = (firmware->getCapability(LcdDepth)>=8)?1:0;
  for (int i=startValue; !strings[i].isNull(); i++) {
    b->addItem(strings[i], 0);
    if (generalSettings.backlightMode == i) {
      b->setCurrentIndex(b->count()-1);
    }
  }
}

void GeneralSetupPanel::populateVoiceLangCB()
{
  QComboBox * b = ui->voiceLang_CB;
  QString strings[] = { tr("English"), tr("Danish"), tr("Dutch"), tr("French"), tr("Italian"), tr("German"), tr("Czech"), tr("Slovak"), tr("Spanish"), tr("Polish"), tr("Portuguese"), tr("Russian"), tr("Swedish"), tr("Hungarian"), NULL};
  QString langcode[] = { "en", "da", "nl","fr", "it", "de", "cz", "sk", "es", "pl", "pt", "ru", "se", "hu", NULL};

  b->clear();
  for (int i=0; strings[i]!=NULL; i++) {
    b->addItem(strings[i],langcode[i]);
    if (generalSettings.ttsLanguage == langcode[i]) {
      b->setCurrentIndex(b->count()-1);
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
    for (int i=0; i<2; i++) {
      generalSettings.ttsLanguage[i] = code.at(i).toLatin1();
    }
    generalSettings.ttsLanguage[2] = '\0';
    emit modified();
  }
}

void GeneralSetupPanel::updateVarioPitchRange()
{
  ui->varioPMax_SB->setMaximum(700+(generalSettings.varioPitch*10)+1000+800);
  ui->varioPMax_SB->setMinimum(700+(generalSettings.varioPitch*10)+1000-800);
}

void GeneralSetupPanel::populateRotEncCB(int reCount)
{
  QString strings[] = { tr("No"), tr("RotEnc A"), tr("Rot Enc B"), tr("Rot Enc C"), tr("Rot Enc D"), tr("Rot Enc E")};
  QComboBox * b = ui->re_CB;

  b->clear();
  for (int i=0; i<=reCount; i++) {
    b->addItem(strings[i]);
  }
  b->setCurrentIndex(generalSettings.reNavigation);
}

void GeneralSetupPanel::setValues()
{
  ui->beeperCB->setCurrentIndex(generalSettings.beeperMode+2);
  ui->channelorderCB->setCurrentIndex(generalSettings.templateSetup);
  ui->stickmodeCB->setCurrentIndex(generalSettings.stickMode);
  if (firmware->getCapability(Haptic)) {
    ui->hapticLengthCB->setCurrentIndex(generalSettings.hapticLength+2);
  }
  else {
    ui->label_HL->hide();
    ui->hapticLengthCB->hide();
  }
  ui->BLBright_SB->setValue(100-generalSettings.backlightBright);
  ui->OFFBright_SB->setValue(generalSettings.backlightOffBright);
  ui->soundModeCB->setCurrentIndex(generalSettings.speakerMode);
  ui->volume_SB->setValue(generalSettings.speakerVolume);
  ui->beeperlenCB->setCurrentIndex(generalSettings.beeperLength+2);
  ui->speakerPitchSB->setValue(generalSettings.speakerPitch);
  ui->hapticStrength->setValue(generalSettings.hapticStrength);
  ui->hapticmodeCB->setCurrentIndex(generalSettings.hapticMode+2);

  if (firmware->getCapability(HasBatMeterRange)) {
    ui->vBatMinDSB->setValue((double)(generalSettings.vBatMin + 90) / 10);
    ui->vBatMaxDSB->setValue((double)(generalSettings.vBatMax + 120) / 10);
  }

  ui->pwrOnDelay->setValue(2 - generalSettings.pwrOnSpeed);
  ui->pwrOffDelay->setValue(2 - generalSettings.pwrOffSpeed);

    // TODO: only if ACCESS available??
  ui->registrationId->setText(generalSettings.registrationId);
}

void GeneralSetupPanel::on_faimode_CB_stateChanged(int)
{
  if (ui->faimode_CB->isChecked()) {
    int ret = QMessageBox::question(this, CPN_STR_APP_NAME,
     tr("If you enable FAI, only RSSI and RxBt sensors will keep working.\nThis function cannot be disabled by the radio.\nAre you sure ?") ,
     QMessageBox::Yes | QMessageBox::No);
    if (ret==QMessageBox::Yes) {
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

void GeneralSetupPanel::populateRotEncModeCB()
{
  QComboBox * b = ui->rotEncMode_CB;
  QString strings[] = { tr("Normal"), tr("Inverted"), tr("Vertical Inverted, Horizontal Normal"), tr("Vertical Inverted, Horizontal Alternate") };
  int itemCount = 4;

  if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
    itemCount = 2;
  }

  b->clear();
  for (uint8_t i=0; i < itemCount; i++) {
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
  generalSettings.speakerPitch = ui->speakerPitchSB->value();
  emit modified();
}

void GeneralSetupPanel::on_hapticStrength_valueChanged()
{
  generalSettings.hapticStrength = ui->hapticStrength->value();
  emit modified();
}

void GeneralSetupPanel::on_soundModeCB_currentIndexChanged(int index)
{
  generalSettings.speakerMode = index;
  emit modified();
}


void GeneralSetupPanel::on_splashScreenChkB_stateChanged(int )
{
  generalSettings.splashMode = ui->splashScreenChkB->isChecked() ? 0 : 1;
  emit modified();
}

void GeneralSetupPanel::on_splashScreenDuration_currentIndexChanged(int index)
{
  generalSettings.splashDuration = 3-index;
  emit modified();
}

void GeneralSetupPanel::on_pwrOnDelay_valueChanged(int)
{
  generalSettings.pwrOnSpeed = 2 - ui->pwrOnDelay->value();
  emit modified();
}

void GeneralSetupPanel::on_pwrOffDelay_valueChanged(int)
{
  generalSettings.pwrOffSpeed = 2 - ui->pwrOffDelay->value();
  emit modified();
}

void GeneralSetupPanel::on_beepVolume_SL_valueChanged()
{
  generalSettings.beepVolume=ui->beepVolume_SL->value();
  emit modified();
}

void GeneralSetupPanel::on_wavVolume_SL_valueChanged()
{
  generalSettings.wavVolume=ui->wavVolume_SL->value();
  emit modified();
}

void GeneralSetupPanel::on_varioVolume_SL_valueChanged()
{
  generalSettings.varioVolume=ui->varioVolume_SL->value();
  emit modified();
}

void GeneralSetupPanel::on_bgVolume_SL_valueChanged()
{
  generalSettings.backgroundVolume=ui->bgVolume_SL->value();
  emit modified();
}

void GeneralSetupPanel::on_varioP0_SB_editingFinished()
{
  generalSettings.varioPitch = (ui->varioP0_SB->value()-700)/10;
  updateVarioPitchRange();
  emit modified();
}

void GeneralSetupPanel::on_varioPMax_SB_editingFinished()
{
  generalSettings.varioRange = (ui->varioPMax_SB->value()-(700+(generalSettings.varioPitch*10))-1000)/10;
  emit modified();
}

void GeneralSetupPanel::on_varioR0_SB_editingFinished()
{
  generalSettings.varioRepeat = (ui->varioR0_SB->value()-500)/10;
  emit modified();
}

void GeneralSetupPanel::on_BLBright_SB_editingFinished()
{
  generalSettings.backlightBright = 100 - ui->BLBright_SB->value();
  emit modified();
}

void GeneralSetupPanel::on_OFFBright_SB_editingFinished()
{
  generalSettings.backlightOffBright = ui->OFFBright_SB->value();
  emit modified();
}

void GeneralSetupPanel::on_volume_SB_editingFinished()
{
  generalSettings.speakerVolume = ui->volume_SB->value();
  emit modified();
}

void GeneralSetupPanel::on_contrastSB_editingFinished()
{
  generalSettings.contrast = ui->contrastSB->value();
  emit modified();
}

void GeneralSetupPanel::on_battwarningDSB_editingFinished()
{
  generalSettings.vBatWarn = (int)(ui->battwarningDSB->value()*10);
  emit modified();
}

void GeneralSetupPanel::on_vBatMinDSB_editingFinished()
{
  generalSettings.vBatMin = ui->vBatMinDSB->value() * 10 - 90;
  emit modified();
}

void GeneralSetupPanel::on_vBatMaxDSB_editingFinished()
{
  generalSettings.vBatMax = ui->vBatMaxDSB->value() * 10 - 120;
  emit modified();
}

void GeneralSetupPanel::on_re_CB_currentIndexChanged(int index)
{
  generalSettings.reNavigation = ui->re_CB->currentIndex();
  emit modified();
}

void GeneralSetupPanel::on_countrycode_CB_currentIndexChanged(int index)
{
  generalSettings.countryCode = ui->countrycode_CB->currentIndex();
  emit modified();
}

void GeneralSetupPanel::on_units_CB_currentIndexChanged(int index)
{
  generalSettings.imperial = ui->units_CB->currentIndex();
  emit modified();
}

void GeneralSetupPanel::on_beeperlenCB_currentIndexChanged(int index)
{
  generalSettings.beeperLength = index-2;
  emit modified();
}

void GeneralSetupPanel::on_hapticLengthCB_currentIndexChanged(int index)
{
  generalSettings.hapticLength = index-2;
  emit modified();
}

void GeneralSetupPanel::on_gpsFormatCB_currentIndexChanged(int index)
{
  generalSettings.gpsFormat = index;
  emit modified();
}

void GeneralSetupPanel::on_backlightautoSB_editingFinished()
{
  int i = ui->backlightautoSB->value()/5;
  if((i*5)!=ui->backlightautoSB->value())
    ui->backlightautoSB->setValue(i*5);
  else
  {
    generalSettings.backlightDelay = i;
    emit modified();
  }
}

void GeneralSetupPanel::on_switchesDelay_valueChanged(int)
{
  generalSettings.switchesDelay = (ui->switchesDelay->value() / 10) - 15;
  emit modified();
}

void GeneralSetupPanel::on_timezoneSB_editingFinished()
{
  generalSettings.timezone = ui->timezoneSB->value();
  emit modified();
}

void GeneralSetupPanel::on_adjustRTC_stateChanged(int)
{
  generalSettings.adjustRTC = ui->adjustRTC->isChecked();
  emit modified();
}


void GeneralSetupPanel::on_inactimerSB_editingFinished()
{
  generalSettings.inactivityTimer = ui->inactimerSB->value();
  emit modified();
}

void GeneralSetupPanel::on_memwarnChkB_stateChanged(int)
{
  generalSettings.disableMemoryWarning = ui->memwarnChkB->isChecked() ? 0 : 1;
  emit modified();
}

void GeneralSetupPanel::on_alarmwarnChkB_stateChanged(int)
{
  generalSettings.disableAlarmWarning = ui->alarmwarnChkB->isChecked() ? 0 : 1;
  emit modified();
}

void GeneralSetupPanel::on_rssiPowerOffWarnChkB_stateChanged(int)
{
  generalSettings.disableRssiPoweroffAlarm = ui->rssiPowerOffWarnChkB->isChecked() ? 0 : 1;
  emit modified();
}

void GeneralSetupPanel::on_beeperCB_currentIndexChanged(int index)
{
  generalSettings.beeperMode = (GeneralSettings::BeeperMode)(index-2);
  emit modified();
}

void GeneralSetupPanel::on_displayTypeCB_currentIndexChanged(int index)
{
  generalSettings.optrexDisplay = index;
  emit modified();
}

void GeneralSetupPanel::on_hapticmodeCB_currentIndexChanged(int index)
{
  generalSettings.hapticMode = (GeneralSettings::BeeperMode)(index-2);
  emit modified();
}


void GeneralSetupPanel::on_channelorderCB_currentIndexChanged(int index)
{
  generalSettings.templateSetup = index;
  emit modified();
}

void GeneralSetupPanel::on_stickmodeCB_currentIndexChanged(int index)
{
  generalSettings.stickMode = index;
  emit modified();
}

void GeneralSetupPanel::unlockSwitchEdited()
{
  int i=0;
  i|=(((uint16_t)ui->chkSA->value()));
  i|=(((uint16_t)ui->chkSB->value())<<2);
  i|=(((uint16_t)ui->chkSC->value())<<4);
  i|=(((uint16_t)ui->chkSD->value())<<6);
  i|=(((uint16_t)ui->chkSE->value())<<8);
  i|=(((uint16_t)ui->chkSF->value())<<10);
  i|=(((uint16_t)ui->chkSG->value())<<12);
  i|=(((uint16_t)ui->chkSH->value())<<14);
  generalSettings.switchUnlockStates=i;
  emit modified();
}

void GeneralSetupPanel::on_blAlarm_ChkB_stateChanged()
{
  generalSettings.flashBeep = ui->blAlarm_ChkB->isChecked();
  emit modified();
}

void GeneralSetupPanel::on_registrationId_editingFinished()
{
  //copy ownerID back to generalSettings.registrationId
  QByteArray array = ui->registrationId->text().toLocal8Bit();
  strncpy(generalSettings.registrationId, array, 8);
  generalSettings.registrationId[8] = '\0';
  emit modified();
}

void GeneralSetupPanel::stickReverseEdited()
{
  generalSettings.stickReverse = ((int)ui->stickReverse1->isChecked()) | ((int)ui->stickReverse2->isChecked()<<1) | ((int)ui->stickReverse3->isChecked()<<2) | ((int)ui->stickReverse4->isChecked()<<3);
  emit modified();
}
