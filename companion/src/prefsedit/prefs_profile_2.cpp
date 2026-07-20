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

#include "prefs_profile.h"
#include "ui_prefs_profile.h"
#include "appdata.h"
#include "moduledata.h"
#include "filtereditemmodels.h"

#include <QPalette>
#include <QColorDialog>

constexpr char FIM_TEMPLATESETUP[]    {"Template Setup"};

PrefsProfilePanel::PrefsProfilePanel(QWidget * parent):
  PrefsPanel(parent),
  ui(new Ui::PrefsProfile)
{
  ui->setupUi(this);
  lock = true;

  panelItemModels = new FilteredItemModelFactory();
  panelItemModels->registerItemModel(new FilteredItemModel(GeneralSettings::templateSetupItemModel()), FIM_TEMPLATESETUP);

  connect(ui->profileBackupPath, &QLineEdit::editingFinished, this, &PrefsProfilePanel::onProfileBackupPathEditingFinished);
  connect(ui->profileBackupPathButton, &QPushButton::clicked, this, &PrefsProfilePanel::onProfileBackupPathButtonClicked);

  update();
  lock = false;
}

PrefsProfilePanel::~PrefsProfilePanel()
{
  delete ui;
}


bool PrefsProfilePanel::save()
{
  Profile & profile = g.currentProfile();
  profile.defaultInternalModule(ui->defaultInternalModuleCB->currentData().toInt());
  profile.externalModuleSize(ui->externalModuleSizeCB->currentData().toInt());
  profile.channelOrder(ui->channelorderCB->currentData().toInt());
  profile.defaultMode(ui->stickmodeCB->currentData().toInt());
  profile.burnFirmware(ui->burnFirmware->isChecked());
  profile.sdPath(ui->sdPath->text());
  profile.pBackupDir(ui->profileBackupPath->text());
  profile.penableBackup(ui->profileBackupEnable->isChecked());
  profile.splashFile(ui->SplashFileName->text());
  profile.runSDSync(ui->chkPromptSDSync->isChecked());
  profile.radioSimCaseColor(ui->lblRadioColorSample->palette().button().color());
  profile.simBtnClickedUseOSTheme(ui->chkSimBtnClickedUseOSTheme->isChecked());
  profile.simBtnClickedColor(ui->lblSimBtnClickedColorSample->palette().button().color());
  profile.useSavedSettings(ui->chkUseSavedSettingsProfile->isChecked());

  // The profile name may NEVER be empty
  if (ui->profileNameLE->text().isEmpty())
    profile.name(tr("My Radio"));
  else
    profile.name(ui->profileNameLE->text());

  bool fwchange = false;
  Firmware * newFw = getFirmwareVariant();  // always !null
  // If a new fw type has been choosen, several things need to reset
  if (Firmware::getCurrentVariant()->getId() != newFw->getId()) {
    // check if we're going to be converting to a new radio type and there are unsaved files in the main window
    if (mainWinHasDirtyChild && !Boards::isBoardCompatible(Firmware::getCurrentVariant()->getBoard(), newFw->getBoard())) {
      QString q = tr("<p><b>You cannot switch Radio Type or change Build Options while there are unsaved file changes. What do you wish to do?</b></p> <ul>" \
                     "<li><i>Save All</i> - Save any open file(s) before saving Settings.<li>" \
                     "<li><i>Reset</i> - Revert to the previous Radio Type and Build Options before saving Settings.</li>" \
                     "<li><i>Cancel</i> - Return to the Settings editor dialog.</li></ul>");
      int resp = QMessageBox::question(this, windowTitle(), q, (QMessageBox::SaveAll | QMessageBox::Reset | QMessageBox::Cancel), QMessageBox::Cancel);
      if (resp == QMessageBox::SaveAll) {
        // signal main window to save files, need to do this before the fw actually changes
        emit firmwareProfileAboutToChange();
      }
      else if (resp == QMessageBox::Reset) {
        // bail out early before saving the radio type & firmware options
        QDialog::accept();
        return;
      }
      else {
        // we do not accept the dialog close
        return;
      }
    }
    Firmware::setCurrentVariant(newFw);
    profile.fwName("");
    profile.fwType(newFw->getId());
    fwchange = true;
  }

  // This needs handling in settingsedit
  QDialog::accept();

  if (fwchange)
    emit firmwareProfileChanged();  // important to do this after the accepted() signal

}

void PrefsProfilePanel::update()
{
  Profile & profile = g.currentProfile();

  ui->burnFirmware->setChecked(profile.burnFirmware());
  ui->defaultInternalModuleCB->setModel(ModuleData::internalModuleItemModel());
  ui->defaultInternalModuleCB->setCurrentIndex(ui->defaultInternalModuleCB->findData(profile.defaultInternalModule()));
  ui->externalModuleSizeCB->setModel(Boards::externalModuleSizeItemModel());
  ui->externalModuleSizeCB->setCurrentIndex(ui->externalModuleSizeCB->findData(profile.externalModuleSize()));
  panelItemModels->getItemModel(FIM_TEMPLATESETUP)->setFilterFlags(Boards::isAir() ? GeneralSettings::RadioTypeContextAir :
                                                                                     GeneralSettings::RadioTypeContextSurface);
  ui->sdPath->setText(profile.sdPath());

  ui->profileBackupPath->setText(profile.pBackupDir());
  if (!ui->profileBackupPath->text().isEmpty()) {
    if (QDir(ui->profileBackupPath->text()).exists()) {
      ui->profileBackupEnable->setEnabled(true);
      ui->profileBackupEnable->setChecked(profile.penableBackup());
    } else {
      ui->profileBackupEnable->setChecked(false);
      ui->profileBackupEnable->setEnabled(false);
    }
  } else if (!ui->backupPath->text().isEmpty() && QDir(ui->backupPath->text()).exists()) {
    ui->profileBackupEnable->setEnabled(true);
    ui->profileBackupEnable->setChecked(profile.penableBackup());
  } else {
    ui->profileBackupEnable->setChecked(false);
    ui->profileBackupEnable->setEnabled(false);
  }

  ui->profileNameLE->setText(profile.name());

  QString hwSettings;

  if (profile.generalSettings().isEmpty()) {
    hwSettings = tr("No backup available for this profile");
  }
  else  {
    QString str = profile.timeStamp();
    if (str.isEmpty())
      hwSettings = tr("Backup available of unknown age");
    else
      hwSettings = tr("Backup available dated %1").arg(str);
  }

  ui->stickmodeCB->setModel(GeneralSettings::stickModeItemModel());
  ui->stickmodeCB->setCurrentIndex(ui->stickmodeCB->findData(profile.defaultMode()));

  if (Boards::isSurface()) {
    ui->stickmodeLabel->hide();
    ui->stickmodeCB->hide();
  }

  ui->channelorderCB->setModel(panelItemModels->getItemModel(FIM_TEMPLATESETUP));
  ui->channelorderCB->setCurrentIndex(ui->channelorderCB->findData(profile.channelOrder()));

  ui->chkUseSavedSettingsProfile->setChecked(profile.useSavedSettings());
  ui->lblGeneralSettings->setText(hwSettings);
  on_chkUseSavedSettingsProfile_stateChanged();

  ui->chkPromptSDSync->setChecked(profile.runSDSync());
  ui->lblRadioColorSample->setPalette(QPalette(profile.radioSimCaseColor()));
  ui->chkSimBtnClickedUseOSTheme->setChecked(profile.simBtnClickedUseOSTheme());
  ui->lblSimBtnClickedColorSample->setPalette(QPalette(profile.simBtnClickedColor()));

  if (ui->chkSimBtnClickedUseOSTheme->isChecked()) {
    ui->lblSimBtnClickedColorSample->setVisible(false);
    ui->btnSimBtnClickedColor->setEnabled(false);
  } else {
    ui->lblSimBtnClickedColorSample->setVisible(true);
    ui->btnSimBtnClickedColor->setEnabled(true);
  }

  QString currType = QStringList(profile.fwType().split('-').mid(0, 2)).join('-');
  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    ui->boardCB->addItem(firmware->getName(), firmware->getId());
    if (currType == firmware->getId()) {
      ui->boardCB->setCurrentIndex(ui->boardCB->count() - 1);
    }
  }

  populateFirmwareOptions(getBaseFirmware());


}

void PrefsProfilePanel::onProfileBackupPathButtonClicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your profile backup folder"), g.backupDir());
  if (!fileName.isEmpty()) {
    ui->profileBackupPath->setText(fileName);
    ui->profileBackupEnable->setEnabled(true);
  }
}

void PrefsProfilePanel::on_sdPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select the folder replicating your SD structure"), g.currentProfile().sdPath());
  if (!fileName.isEmpty()) {
    ui->sdPath->setText(fileName);
  }
}

void PrefsProfilePanel::onBaseFirmwareChanged()
{
  populateFirmwareOptions(getBaseFirmware());

  Firmware *newfw = getFirmwareVariant();
  Profile & profile = g.currentProfile();
  profile.defaultInternalModule(Boards::getDefaultInternalModules(newfw->getBoard()));
  ui->defaultInternalModuleCB->setModel(ModuleData::internalModuleItemModel(newfw->getBoard()));
  ui->defaultInternalModuleCB->setCurrentIndex(ui->defaultInternalModuleCB->findData(profile.defaultInternalModule()));

  profile.externalModuleSize(Boards::getDefaultExternalModuleSize(newfw->getBoard()));
  ui->externalModuleSizeCB->setModel(Boards::externalModuleSizeItemModel());
  ui->externalModuleSizeCB->setCurrentIndex(ui->externalModuleSizeCB->findData(profile.externalModuleSize()));

  if (Boards::isSurface()) {
    profile.defaultMode(1);
    ui->stickmodeLabel->hide();
    ui->stickmodeCB->hide();
    profile.channelOrder(0);
  }

  ui->stickmodeCB->setCurrentIndex(ui->stickmodeCB->findData(profile.defaultMode()));
  panelItemModels->getItemModel(FIM_TEMPLATESETUP)->setFilterFlags(Boards::isAir() ? GeneralSettings::RadioTypeContextAir :
                                                                                     GeneralSettings::RadioTypeContextSurface);
  ui->channelorderCB->setCurrentIndex(ui->channelorderCB->findData(profile.channelOrder()));

}

Firmware *PrefsProfilePanel::getBaseFirmware() const
{
  return Firmware::getFirmwareForId(ui->boardCB->currentData().toString());
}

Firmware * PrefsProfilePanel::getFirmwareVariant() const
{
  QString id = ui->boardCB->currentData().toString();

  foreach(QCheckBox *cb, optionsCheckBoxes.values()) {
    if (cb->isChecked())
      id += "-" + cb->text();
  }

  if (ui->langCombo->count())
    id += "-" + ui->langCombo->currentText();

  return Firmware::getFirmwareForId(id);
}

void PrefsProfilePanel::onFirmwareOptionChanged(bool state)
{
  QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
  if (!(cb && state))
    return;

  // This de-selects any mutually exlusive options (that is, members of the same QList<Option> list).
  const Firmware::OptionsList & fwOpts = getBaseFirmware()->optionGroups();
  for (const Firmware::OptionsGroup & optGrp : fwOpts) {
    for (const Firmware::Option & opt : optGrp) {
      if (cb->text() == opt.name) {
        QCheckBox *ocb = nullptr;
        foreach(const Firmware::Option & other, optGrp)
          if (other.name != opt.name && (ocb = optionsCheckBoxes.value(other.name, nullptr)))
            ocb->setChecked(false);
        return;
      }
    }
  }
}

void PrefsProfilePanel::populateFirmwareOptions(const Firmware * firmware)
{
  const Firmware * baseFw = firmware->getFirmwareBase();
  QStringList currVariant = Firmware::getCurrentVariant()->getId().split('-');
  QString fwLang = Firmware::getCurrentVariant()->getLanguage();

  if (fwLang.isEmpty()) // try to detect os language
    fwLang = QLocale::languageToString(QLocale().language()).split("_").first();

  const QString currLang = ui->langCombo->count() ? ui->langCombo->currentText() : fwLang;
  updateLock = true;

  ui->langCombo->clear();
  for (const char *lang : baseFw->languageList()) {
    ui->langCombo->addItem(lang);
    if (currLang == lang) {
      ui->langCombo->setCurrentIndex(ui->langCombo->count() - 1);
    }
  }

  if (optionsCheckBoxes.size()) {
    currVariant.clear();
    QMutableMapIterator<QString, QCheckBox *> it(optionsCheckBoxes);
    while (it.hasNext()) {
      it.next();
      QCheckBox * cb = it.value();
      if (cb->isChecked())
        currVariant.append(it.key());    // keep previous selections
      ui->optionsLayout->removeWidget(cb);
      cb->deleteLater();
      it.remove();
    }
  }

  int index = 0;
  QWidget * prevFocus = ui->langCombo;
  for (const Firmware::OptionsGroup &optGrp : baseFw->optionGroups()) {
    for (const Firmware::Option &opt : optGrp) {
      QCheckBox * cb = new QCheckBox(ui->profileTab);
      cb->setText(opt.name);
      cb->setToolTip(opt.tooltip);
      cb->setChecked(currVariant.contains(opt.name));
      ui->optionsLayout->addWidget(cb, index / 4, index % 4);
      QWidget::setTabOrder(prevFocus, cb);
      // connect to duplicates check handler if this option is part of a group
      if (optGrp.size() > 1)
        connect(cb, &QCheckBox::toggled, this, &PrefsProfilePanel::onFirmwareOptionChanged);
      optionsCheckBoxes.insert(opt.name, cb);
      prevFocus = cb;
      ++index;
    }
  }

  if (Boards::getCapability(baseFw->getBoard(), Board::HasColorLcd)) {
    ui->widget_splashImage->hide();
    ui->SplashFileName->setText("");
  }
  else {
    ui->widget_splashImage->show();
    ui->SplashFileName->setText(g.currentProfile().splashFile());
    displayImage(g.currentProfile().splashFile());
  }

  updateLock = false;
  QTimer::singleShot(50, this, &PrefsProfilePanel::shrink);
}

void PrefsProfilePanel::onProfileBackupPathEditingFinished()
{
  if (!ui->profileBackupPath->text().isEmpty()) {
    if (QDir(ui->profileBackupPath->text()).exists()) {
      ui->profileBackupEnable->setEnabled(true);
    } else {
      ui->profileBackupEnable->setChecked(false);
      ui->profileBackupEnable->setEnabled(false);
    }
  } else if (!ui->backupPath->text().isEmpty() && QDir(ui->backupPath->text()).exists()) {
    ui->profileBackupEnable->setEnabled(true);
  } else {
    ui->profileBackupEnable->setChecked(false);
    ui->profileBackupEnable->setEnabled(false);
  }
}

void PrefsProfilePanel::on_chkUseSavedPrefsProfile_stateChanged()
{
  if (ui->chkUseSavedSettingsProfile->isChecked() && !g.currentProfile().generalSettings().isEmpty()) {
    ui->stickmodeLabel->setEnabled(false);
    ui->stickmodeCB->setEnabled(false);
    ui->channelorderLabel->setEnabled(false);
    ui->channelorderCB->setEnabled(false);
  } else {
    ui->stickmodeLabel->setEnabled(true);
    ui->stickmodeCB->setEnabled(true);
    ui->channelorderLabel->setEnabled(true);
    ui->channelorderCB->setEnabled(true);
  }
}
