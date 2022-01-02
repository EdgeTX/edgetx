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

#include "apppreferencesdialog.h"
#include "ui_apppreferencesdialog.h"
#include "mainwindow.h"
#include "appdata.h"
#include "helpers.h"
#include "storage.h"
#if defined(JOYSTICKS)
#include "joystick.h"
#include "joystickdialog.h"
#endif
#include "moduledata.h"
#include "compounditemmodels.h"

#include <QAbstractItemModel>

AppPreferencesDialog::AppPreferencesDialog(QWidget * parent) :
  QDialog(parent),
  ui(new Ui::AppPreferencesDialog),
  updateLock(false),
  mainWinHasDirtyChild(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("apppreferences.png"));
  ui->tabWidget->setCurrentIndex(0);

  initSettings();
  connect(ui->boardCB, SIGNAL(currentIndexChanged(int)), this, SLOT(onBaseFirmwareChanged()));
  connect(ui->opt_appDebugLog, &QCheckBox::toggled, this, &AppPreferencesDialog::toggleAppLogSettings);
  connect(ui->opt_fwTraceLog, &QCheckBox::toggled, this, &AppPreferencesDialog::toggleAppLogSettings);

#if !defined(JOYSTICKS)
  ui->joystickCB->hide();
  ui->joystickCB->setDisabled(true);
  ui->joystickcalButton->hide();
  ui->joystickChkB->hide();
  ui->label_11->hide();
#endif

  shrink();
}

AppPreferencesDialog::~AppPreferencesDialog()
{
  delete ui;
}

void AppPreferencesDialog::setMainWinHasDirtyChild(bool value)
{
  mainWinHasDirtyChild = value;
}

void AppPreferencesDialog::accept()
{
  Profile & profile = g.currentProfile();

  g.autoCheckApp(ui->autoCheckCompanion->isChecked());
  g.OpenTxBranch(AppData::DownloadBranchType(ui->OpenTxBranch->currentIndex()));
  g.autoCheckFw(ui->autoCheckFirmware->isChecked());
  g.showSplash(ui->showSplash->isChecked());
  g.promptProfile(ui->chkPromptProfile->isChecked());
  g.simuSW(ui->simuSW->isChecked());
  g.removeModelSlots(ui->opt_removeBlankSlots->isChecked());
  g.newModelAction(ui->opt_newMdl_useWizard->isChecked() ? AppData::MODEL_ACT_WIZARD : ui->opt_newMdl_useEditor->isChecked() ? AppData::MODEL_ACT_EDITOR : AppData::MODEL_ACT_NONE);
  g.historySize(ui->historySize->value());
  g.backLight(ui->backLightColor->currentIndex());
  profile.volumeGain(round(ui->volumeGain->value() * 10.0));
  g.libDir(ui->libraryPath->text());
  g.gePath(ui->ge_lineedit->text());
  g.embedSplashes(ui->splashincludeCB->currentIndex());
  g.enableBackup(ui->backupEnable->isChecked());

  g.appDebugLog(ui->opt_appDebugLog->isChecked());
  g.fwTraceLog(ui->opt_fwTraceLog->isChecked());
  g.appLogsDir(ui->appLogsDir->text());

  if (ui->joystickChkB ->isChecked() && ui->joystickCB->isEnabled()) {
    g.jsSupport(ui->joystickChkB ->isChecked());
    g.jsCtrl(ui->joystickCB ->currentIndex());
  }
  else {
    g.jsSupport(false);
    g.jsCtrl(0);
  }
  profile.defaultInternalModule(ui->defaultInternalModuleCB->currentData().toInt());
  profile.channelOrder(ui->channelorderCB->currentIndex());
  profile.defaultMode(ui->stickmodeCB->currentIndex());
  profile.renameFwFiles(ui->renameFirmware->isChecked());
  profile.burnFirmware(ui->burnFirmware->isChecked());
  profile.sdPath(ui->sdPath->text());
  profile.pBackupDir(ui->profilebackupPath->text());
  profile.penableBackup(ui->pbackupEnable->isChecked());
  profile.splashFile(ui->SplashFileName->text());

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
    profile.resetFwVariables();
    profile.fwType(newFw->getId());
    fwchange = true;
  }

  QDialog::accept();

  if (fwchange)
    emit firmwareProfileChanged();  // important to do this after the accepted() signal
}

void AppPreferencesDialog::on_snapshotPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this, tr("Select your snapshot folder"), g.snapshotDir());
  if (!fileName.isEmpty()) {
    g.snapshotDir(fileName);
    g.snapToClpbrd(false);
    ui->snapshotPath->setText(fileName);
  }
}

void AppPreferencesDialog::initSettings()
{
  const Profile & profile = g.currentProfile();

  // Disable these for now, until online elements are implemented
  ui->autoCheckFirmware->setDisabled(true);
  ui->autoCheckCompanion->setDisabled(true);
  ui->OpenTxBranch->setDisabled(true);

  ui->snapshotClipboardCKB->setChecked(g.snapToClpbrd());
  ui->burnFirmware->setChecked(profile.burnFirmware());
  ui->snapshotPath->setText(g.snapshotDir());
  ui->snapshotPath->setReadOnly(true);
  if (ui->snapshotClipboardCKB->isChecked()) {
    ui->snapshotPath->setDisabled(true);
    ui->snapshotPathButton->setDisabled(true);
  }

#if !defined(ALLOW_NIGHTLY_BUILDS)
  // remove nightly version option entirely unless it's the current one (because user might have nightly version also installed)
  if (g.OpenTxBranch() != AppData::BRANCH_NIGHTLY_UNSTABLE)
    ui->OpenTxBranch->removeItem(2);
  // if it's already selected, add a warning message
  else
    ui->updatesLayout->addWidget(new QLabel("<font color='red'>" % tr("Note: Nightly builds are not available in this version, Release/RC update channel will be used.") % "</font>", this));
#endif
  ui->OpenTxBranch->setCurrentIndex(g.OpenTxBranch());

  ui->autoCheckCompanion->setChecked(g.autoCheckApp());
  ui->autoCheckFirmware->setChecked(g.autoCheckFw());
  ui->showSplash->setChecked(g.showSplash());
  ui->chkPromptProfile->setChecked(g.promptProfile());
  ui->historySize->setValue(g.historySize());
  ui->backLightColor->setCurrentIndex(g.backLight());
  ui->volumeGain->setValue(profile.volumeGain() / 10.0);

  if (IS_HORUS_OR_TARANIS(getCurrentBoard())) {
    ui->backLightColor->setEnabled(false);
  }

  ui->simuSW->setChecked(g.simuSW());
  ui->opt_removeBlankSlots->setChecked(g.removeModelSlots());
  ui->opt_newMdl_useNone->setChecked(g.newModelAction() == AppData::MODEL_ACT_NONE);
  ui->opt_newMdl_useWizard->setChecked(g.newModelAction() == AppData::MODEL_ACT_WIZARD);
  ui->opt_newMdl_useEditor->setChecked(g.newModelAction() == AppData::MODEL_ACT_EDITOR);
  ui->libraryPath->setText(g.libDir());
  ui->ge_lineedit->setText(g.gePath());

  if (!g.backupDir().isEmpty()) {
    if (QDir(g.backupDir()).exists()) {
      ui->backupPath->setText(g.backupDir());
      ui->backupEnable->setEnabled(true);
      ui->backupEnable->setChecked(g.enableBackup());
    }
    else {
      ui->backupEnable->setDisabled(true);
    }
  }
  else {
    ui->backupEnable->setDisabled(true);
  }
  ui->splashincludeCB->setCurrentIndex(g.embedSplashes());

  ui->opt_appDebugLog->setChecked(g.appDebugLog());
  ui->opt_fwTraceLog->setChecked(g.fwTraceLog());
  ui->appLogsDir->setText(g.appLogsDir());
  toggleAppLogSettings();

#if defined(JOYSTICKS)
  ui->joystickChkB->setChecked(g.jsSupport());
  if (ui->joystickChkB->isChecked()) {
    QStringList joystickNames;
    joystickNames << tr("No joysticks found");
    joystick = new Joystick(0,false,0,0);
    ui->joystickcalButton->setDisabled(true);
    ui->joystickCB->setDisabled(true);

    if ( joystick ) {
      if ( joystick->joystickNames.count() > 0 ) {
        joystickNames = joystick->joystickNames;
        ui->joystickCB->setEnabled(true);
        ui->joystickcalButton->setEnabled(true);
      }
      joystick->close();
    }
    ui->joystickCB->clear();
    ui->joystickCB->insertItems(0, joystickNames);
    ui->joystickCB->setCurrentIndex(g.jsCtrl());
  }
  else {
    ui->joystickCB->clear();
    ui->joystickCB->setDisabled(true);
    ui->joystickcalButton->setDisabled(true);
  }
#endif
  //  Profile Tab Inits
  ui->defaultInternalModuleCB->setModel(ModuleData::internalModuleItemModel());
  ui->defaultInternalModuleCB->setCurrentIndex(ui->defaultInternalModuleCB->findData(profile.defaultInternalModule()));
  ui->channelorderCB->setCurrentIndex(profile.channelOrder());
  ui->stickmodeCB->setCurrentIndex(profile.defaultMode());
  ui->renameFirmware->setChecked(profile.renameFwFiles());
  ui->sdPath->setText(profile.sdPath());
  if (!profile.pBackupDir().isEmpty()) {
    if (QDir(profile.pBackupDir()).exists()) {
      ui->profilebackupPath->setText(profile.pBackupDir());
      ui->pbackupEnable->setEnabled(true);
      ui->pbackupEnable->setChecked(profile.penableBackup());
    } else {
      ui->pbackupEnable->setDisabled(true);
    }
  }
  else {
      ui->pbackupEnable->setDisabled(true);
  }

  ui->profileNameLE->setText(profile.name());

  QString hwSettings;
  if (profile.stickPotCalib() == "" ) {
    hwSettings = tr("EMPTY: No radio settings stored in profile");
  }
  else  {
    QString str = profile.timeStamp();
    if (str.isEmpty())
      hwSettings = tr("AVAILABLE: Radio settings of unknown age");
    else
      hwSettings = tr("AVAILABLE: Radio settings stored %1").arg(str);
  }
  ui->lblGeneralSettings->setText(hwSettings);

  QString currType = QStringList(profile.fwType().split('-').mid(0, 2)).join('-');
  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    ui->boardCB->addItem(firmware->getName(), firmware->getId());
    if (currType == firmware->getId()) {
      ui->boardCB->setCurrentIndex(ui->boardCB->count() - 1);
    }
  }

  onBaseFirmwareChanged();
}

void AppPreferencesDialog::on_libraryPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your library folder"), g.libDir());
  if (!fileName.isEmpty()) {
    g.libDir(fileName);
    ui->libraryPath->setText(fileName);
  }
}

void AppPreferencesDialog::on_snapshotClipboardCKB_clicked()
{
  if (ui->snapshotClipboardCKB->isChecked()) {
    ui->snapshotPath->setDisabled(true);
    ui->snapshotPathButton->setDisabled(true);
    g.snapToClpbrd(true);
  }
  else {
    ui->snapshotPath->setEnabled(true);
    ui->snapshotPath->setReadOnly(true);
    ui->snapshotPathButton->setEnabled(true);
    g.snapToClpbrd(false);
  }
}

void AppPreferencesDialog::on_backupPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your Models and Settings backup folder"), g.backupDir());
  if (!fileName.isEmpty()) {
    g.backupDir(fileName);
    ui->backupPath->setText(fileName);
    ui->backupEnable->setEnabled(true);
  }
}

void AppPreferencesDialog::on_ProfilebackupPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your Models and Settings backup folder"), g.backupDir());
  if (!fileName.isEmpty()) {
    ui->profilebackupPath->setText(fileName);
    ui->pbackupEnable->setEnabled(true);
  }
}


void AppPreferencesDialog::on_btn_appLogsDir_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this, tr("Select a folder for application logs"), ui->appLogsDir->text());
  if (!fileName.isEmpty()) {
    ui->appLogsDir->setText(fileName);
  }
}

void AppPreferencesDialog::on_ge_pathButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Select Google Earth executable"),ui->ge_lineedit->text());
  if (!fileName.isEmpty()) {
    ui->ge_lineedit->setText(fileName);
  }
}

void AppPreferencesDialog::on_btnClearPos_clicked()
{
  SimulatorOptions opts = g.profile[g.sessionId()].simulatorOptions();
  opts.controlsState.clear();
  g.profile[g.sessionId()].simulatorOptions(opts);
}

#if defined(JOYSTICKS)
void AppPreferencesDialog::on_joystickChkB_clicked() {
  if (ui->joystickChkB->isChecked()) {
    QStringList joystickNames;
    joystickNames << tr("No joysticks found");
    joystick = new Joystick(0,false,0,0);
    ui->joystickcalButton->setDisabled(true);
    ui->joystickCB->setDisabled(true);

    if ( joystick ) {
      if ( joystick->joystickNames.count() > 0 ) {
        joystickNames = joystick->joystickNames;
        ui->joystickCB->setEnabled(true);
        ui->joystickcalButton->setEnabled(true);
      }
      joystick->close();
    }
    ui->joystickCB->clear();
    ui->joystickCB->insertItems(0, joystickNames);
  }
  else {
    ui->joystickCB->clear();
    ui->joystickCB->setDisabled(true);
    ui->joystickcalButton->setDisabled(true);
  }
}

void AppPreferencesDialog::on_joystickcalButton_clicked() {
   joystickDialog * jd=new joystickDialog(this, ui->joystickCB->currentIndex());
   jd->exec();
}
#endif

// ******** Profile tab functions

void AppPreferencesDialog::on_sdPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select the folder replicating your SD structure"), g.currentProfile().sdPath());
  if (!fileName.isEmpty()) {
    ui->sdPath->setText(fileName);
  }
}

bool AppPreferencesDialog::displayImage(const QString & fileName)
{
  // Start by clearing the label
  ui->imageLabel->clear();

  if (fileName.isEmpty())
    return false;

  QImage image(fileName);
  if (image.isNull())
    return false;

  ui->imageLabel->setPixmap(makePixMap(image));
  ui->imageLabel->setFixedSize(getCurrentFirmware()->getCapability(LcdWidth), getCurrentFirmware()->getCapability(LcdHeight));
  return true;
}

void AppPreferencesDialog::on_SplashSelect_clicked()
{
  QString supportedImageFormats;
  for (int formatIndex = 0; formatIndex < QImageReader::supportedImageFormats().count(); formatIndex++) {
    supportedImageFormats += QLatin1String(" *.") + QImageReader::supportedImageFormats()[formatIndex];
  }

  QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image to load"), g.imagesDir(), tr("Images (%1)").arg(supportedImageFormats));

  if (!fileName.isEmpty()){
    g.imagesDir(QFileInfo(fileName).dir().absolutePath());

    displayImage(fileName);
    ui->SplashFileName->setText(fileName);
  }
}

void AppPreferencesDialog::on_clearImageButton_clicked()
{
  ui->imageLabel->clear();
  ui->SplashFileName->clear();
}

void AppPreferencesDialog::onBaseFirmwareChanged()
{
  populateFirmwareOptions(getBaseFirmware());

  Firmware *newfw = getFirmwareVariant();
  Profile & profile = g.currentProfile();
  profile.defaultInternalModule(Boards::getDefaultInternalModules(newfw->getBoard()));
  ui->defaultInternalModuleCB->setModel(ModuleData::internalModuleItemModel(newfw->getBoard()));
  ui->defaultInternalModuleCB->setCurrentIndex(ui->defaultInternalModuleCB->findData(profile.defaultInternalModule()));
}

Firmware *AppPreferencesDialog::getBaseFirmware() const
{
  return Firmware::getFirmwareForId(ui->boardCB->currentData().toString());
}

Firmware * AppPreferencesDialog::getFirmwareVariant() const
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

void AppPreferencesDialog::onFirmwareOptionChanged(bool state)
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

void AppPreferencesDialog::toggleAppLogSettings()
{
  bool vis = (ui->opt_appDebugLog->isChecked() || ui->opt_fwTraceLog->isChecked());
  ui->appLogsDir->setVisible(vis);
  ui->lbl_appLogsDir->setVisible(vis);
  ui->btn_appLogsDir->setVisible(vis);
}

void AppPreferencesDialog::populateFirmwareOptions(const Firmware * firmware)
{
  const Firmware * baseFw = firmware->getFirmwareBase();
  QStringList currVariant = Firmware::getCurrentVariant()->getId().split('-');
  const QString currLang = ui->langCombo->count() ? ui->langCombo->currentText() : currVariant.last();

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
        connect(cb, &QCheckBox::toggled, this, &AppPreferencesDialog::onFirmwareOptionChanged);
      optionsCheckBoxes.insert(opt.name, cb);
      prevFocus = cb;
      ++index;
    }
  }

  // TODO: Remove once splash replacement supported on Horus
  // NOTE: 480x272 image causes issues on screens <800px high, needs a solution like scrolling once reinstated
  if (IS_FAMILY_HORUS_OR_T16(baseFw->getBoard())) {
    ui->widget_splashImage->hide();
    ui->SplashFileName->setText("");
  }
  else {
    ui->widget_splashImage->show();
    ui->SplashFileName->setText(g.currentProfile().splashFile());
    displayImage(g.currentProfile().splashFile());
  }

  updateLock = false;
  QTimer::singleShot(50, this, &AppPreferencesDialog::shrink);
}

void AppPreferencesDialog::shrink()
{
  adjustSize();
}
