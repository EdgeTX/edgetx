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
#include "helpers.h"

constexpr char FIM_HATSMODE[]       {"Hats Mode"};
constexpr char FIM_STICKMODE[]      {"Stick Mode"};
constexpr char FIM_TEMPLATESETUP[]  {"Template Setup"};
constexpr char FIM_BACKLIGHTMODE[]  {"Backlight Mode"};
constexpr char FIM_CONTROLSRC[]     {"Control Source"};

GeneralSetupPanel::GeneralSetupPanel(QWidget * parent, GeneralSettings & generalSettings,
  Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
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
  panelFilteredModels->registerItemModel(new FilteredItemModel(GeneralSettings::backlightModeItemModel()), FIM_BACKLIGHTMODE);
  panelFilteredModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_ControlSource)),
                                         FIM_CONTROLSRC);

  lock = true;

  ui->volumeCtrl_CB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  ui->volumeCtrl_CB->setModel(panelFilteredModels->getItemModel(FIM_CONTROLSRC));
  ui->volumeCtrl_CB->setCurrentIndex(ui->volumeCtrl_CB->findData(generalSettings.volumeSrc.toValue()));
  if (ui->volumeCtrl_CB->currentIndex() < 0 && generalSettings.volumeSrc.toValue() == 0)
    ui->volumeCtrl_CB->setCurrentIndex(Helpers::getFirstPosValueIndex(ui->volumeCtrl_CB));

  ui->brightCtrl_CB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  ui->brightCtrl_CB->setModel(panelFilteredModels->getItemModel(FIM_CONTROLSRC));
  ui->brightCtrl_CB->setCurrentIndex(ui->brightCtrl_CB->findData(generalSettings.backlightSrc.toValue()));
  if (ui->brightCtrl_CB->currentIndex() < 0 && generalSettings.backlightSrc.toValue() == 0)
    ui->brightCtrl_CB->setCurrentIndex(Helpers::getFirstPosValueIndex(ui->brightCtrl_CB));

  ui->backlightswCB->setModel(panelFilteredModels->getItemModel(FIM_BACKLIGHTMODE));
  ui->backlightswCB->setCurrentIndex(ui->backlightswCB->findData(generalSettings.backlightMode));

  populateVoiceLangCB(ui->voiceLang_CB, generalSettings.ttsLanguage);
  populateTextLangCB(ui->textLang_CB, generalSettings.uiLanguage, Boards::getCapability(board, Board::HasColorLcd));

  if (!firmware->getCapability(MavlinkTelemetry)) {
    ui->mavbaud_CB->hide();
    ui->mavbaud_label->hide();
  }
  else {
    ui->mavbaud_CB->setCurrentIndex(generalSettings.mavbaud);
  }

  ui->beepVolume_SL->setValue(generalSettings.beepVolume);
  ui->varioVolume_SL->setValue(generalSettings.varioVolume);
  ui->bgVolume_SL->setValue(generalSettings.backgroundVolume);
  ui->wavVolume_SL->setValue(generalSettings.wavVolume);
  ui->varioP0_SB->setValue(700 + (generalSettings.varioPitch * 10));
  updateVarioPitchRange();
  ui->varioPMax_SB->setValue(700 + (generalSettings.varioPitch * 10) + 1000 + (generalSettings.varioRange * 10));
  ui->varioR0_SB->setValue(500 + (generalSettings.varioRepeat * 10));
  ui->faimode_CB->setChecked(generalSettings.fai);
  ui->countrycode_CB->setCurrentIndex(generalSettings.countryCode);
  ui->units_CB->setCurrentIndex(generalSettings.imperial);
  ui->ppm_units_CB->setCurrentIndex(generalSettings.ppmunit);
  ui->gpsFormatCB->setCurrentIndex(generalSettings.gpsFormat);
  ui->timezoneLE->setTime((generalSettings.timezone * 3600) + (generalSettings.timezoneMinutes/*quarter hours*/ * 15 * 60));
  ui->adjustRTC->setChecked(generalSettings.adjustRTC);

  if (IS_STM32(board)) {
    ui->usbModeCB->setCurrentIndex(generalSettings.usbMode);
  }
  else {
    ui->usbModeLabel->hide();
    ui->usbModeCB->hide();
  }

  if (IS_FLYSKY_EL18(board) || IS_FLYSKY_NV14(board) || IS_FAMILY_PL18(board)) {
    ui->hatsModeCB->setModel(panelFilteredModels->getItemModel(FIM_HATSMODE));
    ui->hatsModeCB->setField(generalSettings.hatsMode, this);
  }
  else {
    ui->hatsModeLabel->hide();
    ui->hatsModeCB->hide();
  }

  if (Boards::getCapability(board, Board::HasSwitchableJack)) {
    ui->jackModeCB->setCurrentIndex(generalSettings.jackMode);
  }
  else {
    ui->jackModeLabel->hide();
    ui->jackModeCB->hide();
  }

  ui->volume_SL->setMaximum(Boards::getCapability(board, Board::MaxVolume));

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

  if (!firmware->getCapability(Haptic)) {
    ui->hapticStrength->setDisabled(true);
    ui->hapticmodeCB->setDisabled(true);
  }

  if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
    ui->backlightautoSB->setMinimum(5);
  }

  ui->contrastSB->setMinimum(Boards::getCapability(board, Board::MinContrast));
  ui->contrastSB->setMaximum(Boards::getCapability(board, Board::MaxContrast));
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

  if (!Boards::getCapability(board, Board::PwrButtonPress)) {
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

  if (Boards::getCapability(board, Board::HasBacklightColor)) {
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

// Copied from tts.h
enum RadioLanguage {
  LANG_CN,
  LANG_CZ,
  LANG_DA,
  LANG_DE,
  LANG_EN,
  LANG_ES,
  LANG_FI,
  LANG_FR,
  LANG_HE,
  LANG_HU,
  LANG_IT,
  LANG_JP,
  LANG_KO,
  LANG_NL,
  LANG_PL,
  LANG_PT,
  LANG_RU,
  LANG_SE,
  LANG_SK,
  LANG_TW,
  LANG_UA,
  LANG_COUNT
};

// Order must match RadioLanguage
// Note: these align with the radio NOT computer locales - TODO harmonise with ISO and one list!!!
static const char* langStrings[][2] = {
  { "Chinese", "cn" },
  { "Czech", "cz" },
  { "Danish", "da" },
  { "German", "de" },
  { "English", "en" },
  { "Spanish", "es" },
  { "Finnish", "fi" },
  { "French", "fr" },
  { "Hebrew", "he" },
  { "Hungarian", "hu" },
  { "Italian", "it" },
  { "Japanese", "jp" },
  { "Korean", "ko" },
  { "Dutch", "nl" },
  { "Polish", "pl" },
  { "Portuguese", "pt" },
  { "Russian", "ru" },
  { "Swedish", "se" },
  { "Slovak", "sk" },
  { "Taiwanese", "tw" },
  { "Ukrainian", "ua" },
};

void GeneralSetupPanel::populateVoiceLangCB(QComboBox* b, const char* currLang)
{
  b->clear();
  for (int i = 0; i < LANG_COUNT; i++) {
    b->addItem(tr(langStrings[i][0]), langStrings[i][1]);
    if (strncmp(currLang, langStrings[i][1], 2) == 0) {
      b->setCurrentIndex(b->count() - 1);
    }
  }
}

void GeneralSetupPanel::populateTextLangCB(QComboBox* b, const char* currLang, bool isColor)
{
  // Available text languages
  // B&W
  static RadioLanguage bwLangs[] = {
    // LANG_CN,   // no fonts
    LANG_CZ,
    LANG_DA,
    LANG_DE,
    LANG_EN,
    LANG_ES,
    LANG_FI,
    LANG_FR,
    // LANG_HE,   // no fonts
    // LANG_HU,   // no translation file
    LANG_IT,
    // LANG_JP,   // no fonts
    // LANG_KO,   // no fonts
    LANG_NL,
    LANG_PL,
    LANG_PT,
    LANG_RU,
    LANG_SE,
    // LANG_SK,   // no translation file
    // LANG_TW,   // no fonts
    LANG_UA,
    LANG_COUNT
  };
  // Color
  static RadioLanguage colorLangs[] = {
    LANG_CN,
    LANG_CZ,
    LANG_DA,
    LANG_DE,
    LANG_EN,
    LANG_ES,
    LANG_FI,
    LANG_FR,
    LANG_HE,
    // LANG_HU,   // no translation file
    LANG_IT,
    LANG_JP,
    LANG_KO,
    LANG_NL,
    LANG_PL,
    LANG_PT,
    LANG_RU,
    LANG_SE,
    // LANG_SK,   // no translation file
    LANG_TW,
    LANG_UA,
    LANG_COUNT
  };

  RadioLanguage* langs = isColor ? colorLangs : bwLangs;

  b->clear();
  for (int i = 0; langs[i] < LANG_COUNT; i++) {
    int n = langs[i];
    b->addItem(tr(langStrings[n][0]),langStrings[n][1]);
    if (strncmp(currLang, langStrings[n][1], 2) == 0) {
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

void GeneralSetupPanel::on_textLang_CB_currentIndexChanged(int index)
{
  if (!lock) {
    QString code = ui->textLang_CB->itemData(index).toString();
    for (int i = 0; i < 2; i++) {
      generalSettings.uiLanguage[i] = code.at(i).toLatin1();
    }
    generalSettings.uiLanguage[2] = '\0';
    emit modified();
  }
}

void GeneralSetupPanel::updateVarioPitchRange()
{
  ui->varioPMax_SB->setMaximum(700 + (generalSettings.varioPitch * 10) + 1000 + 800);
  ui->varioPMax_SB->setMinimum(700 + (generalSettings.varioPitch * 10) + 1000 - 800);
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
  ui->OFFBright_SB->setMinimum(Boards::getCapability(board, Board::BacklightLevelMin));
  if (generalSettings.backlightOffBright > 100 - generalSettings.backlightBright)
    generalSettings.backlightOffBright = 100 - generalSettings.backlightBright;
  ui->BLBright_SB->setValue(100 - generalSettings.backlightBright);
  ui->OFFBright_SB->setValue(generalSettings.backlightOffBright);
  ui->BLBright_SB->setMinimum(ui->OFFBright_SB->value());
  ui->OFFBright_SB->setMaximum(ui->BLBright_SB->value());
  ui->volume_SL->setValue(generalSettings.speakerVolume + 12);
  ui->beeperlenCB->setCurrentIndex(generalSettings.beeperLength + 2);
  ui->speakerPitchSB->setValue(generalSettings.speakerPitch);
  ui->hapticStrength->setValue(generalSettings.hapticStrength);
  ui->hapticmodeCB->setCurrentIndex(generalSettings.hapticMode + 2);
  ui->vBatMinDSB->setValue((double)(generalSettings.vBatMin + 90) / 10);
  ui->vBatMaxDSB->setValue((double)(generalSettings.vBatMax + 120) / 10);
  ui->pwrOnDelay->setCurrentIndex(pwrDelayFromYaml(generalSettings.pwrOnSpeed));
  ui->pwrOffDelay->setCurrentIndex(pwrDelayFromYaml(generalSettings.pwrOffSpeed));
  ui->pwrOffIfInactiveSB->setValue(generalSettings.pwrOffIfInactive);
  ui->registrationId->setText(generalSettings.registrationId);
  ui->startSoundCB->setChecked(!generalSettings.dontPlayHello);
  ui->modelQuickSelect_CB->setChecked(generalSettings.modelQuickSelect);

  if (Boards::getCapability(board, Board::HasColorLcd)) {
    ui->modelSelectLayout_CB->setCurrentIndex(generalSettings.modelSelectLayout);
    ui->labelSingleSelect_CB->setCurrentIndex(generalSettings.labelSingleSelect);
    ui->labelMultiMode_CB->setCurrentIndex(generalSettings.labelMultiMode);
    ui->favMultiMode_CB->setCurrentIndex(generalSettings.favMultiMode);
    showLabelSelectOptions();
  } else {
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

void GeneralSetupPanel::on_volumeCtrl_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.volumeSrc = RawSource(ui->volumeCtrl_CB->itemData(ui->volumeCtrl_CB->currentIndex()).toInt());
    emit modified();
  }
}

void GeneralSetupPanel::on_brightCtrl_CB_currentIndexChanged(int index)
{
  if (!lock) {
    generalSettings.backlightSrc = RawSource(ui->brightCtrl_CB->itemData(ui->brightCtrl_CB->currentIndex()).toInt());
    emit modified();
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
