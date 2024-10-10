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

#include "apppreferencesdialog.h"
#include "ui_apppreferencesdialog.h"
#include "mainwindow.h"
#include "helpers.h"
#include "storage.h"
#if defined(JOYSTICKS)
#include "joystick.h"
#include "joystickdialog.h"
#endif
#include "moduledata.h"
#include "filtereditemmodels.h"
#include "updates/updatefactories.h"
#include "updates/updateoptionsdialog.h"

constexpr char FIM_TEMPLATESETUP[]    {"Template Setup"};

AppPreferencesDialog::AppPreferencesDialog(QWidget * parent, UpdateFactories * factories) :
  QDialog(parent),
  ui(new Ui::AppPreferencesDialog),
  updateLock(false),
  mainWinHasDirtyChild(false),
  factories(factories),
  panelItemModels(nullptr)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("apppreferences.png"));
  ui->tabWidget->setCurrentIndex(0);

  panelItemModels = new FilteredItemModelFactory();
  panelItemModels->registerItemModel(new FilteredItemModel(GeneralSettings::templateSetupItemModel()), FIM_TEMPLATESETUP);

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
  delete panelItemModels;
}

void AppPreferencesDialog::setMainWinHasDirtyChild(bool value)
{
  mainWinHasDirtyChild = value;
}

void AppPreferencesDialog::accept()
{
  Profile & profile = g.currentProfile();

  g.showSplash(ui->showSplash->isChecked());
  g.sortProfiles(ui->sortProfiles->isChecked());
  g.promptProfile(ui->chkPromptProfile->isChecked());
  g.simuSW(ui->simuSW->isChecked());
  g.disableJoystickWarning(ui->joystickWarningCB->isChecked());
  g.removeModelSlots(ui->opt_removeBlankSlots->isChecked());
  g.newModelAction((AppData::NewModelAction)ui->cboNewModelAction->currentIndex());
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
  g.runAppInstaller(ui->chkPromptInstall->isChecked());

  if (ui->joystickChkB ->isChecked()) {
    g.jsSupport(ui->joystickChkB ->isChecked());
    // Don't overwrite selected joystick if not connected. Avoid surprising the user.
    if (ui->joystickCB->isEnabled()) {
      profile.jsName(ui->joystickCB->currentText());
      g.loadNamedJS();
    }
  }
  else {
    g.jsSupport(false);
  }

  //  Updates tab

  if (ui->leDownloadDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Settings: Download folder path missing!"));
    return;
  }

  if (ui->leDecompressDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Settings: Decompress folder path missing!"));
    return;
  }

  if (ui->leUpdateDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Settings: Update folder path missing!"));
    return;
  }

  if (!ui->chkDecompressDirUseDwnld->isChecked() &&
      ui->leDecompressDir->text().trimmed() == ui->leDownloadDir->text().trimmed()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Settings: Decompress and download folders have the same path!"));
    return;
  }

  g.updateCheckFreq(AppData::UpdateCheckFreq(ui->cboUpdateCheckFreq->currentIndex()));
  g.downloadDir(ui->leDownloadDir->text());

  g.decompressDirUseDwnld(ui->chkDecompressDirUseDwnld->isChecked());
  g.decompressDir(ui->leDecompressDir->text());

  g.updateDirUseSD(ui->chkUpdateDirUseSD->isChecked());
  g.updateDir(ui->leUpdateDir->text());

  g.updDelDownloads(ui->chkDelDownloads->isChecked());
  g.updDelDecompress(ui->chkDelDecompress->isChecked());
  g.updLogLevel(ui->cboLogLevel->currentIndex());

  QMapIterator<QString, int> it(factories->sortedComponentsList());

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    g.component[i].checkForUpdate(chkCheckForUpdate[i]->isChecked());
    g.component[i].releaseChannel((ComponentData::ReleaseChannel)cboReleaseChannel[i]->currentIndex());
  }

  profile.defaultInternalModule(ui->defaultInternalModuleCB->currentData().toInt());
  profile.externalModuleSize(ui->externalModuleSizeCB->currentData().toInt());
  profile.channelOrder(ui->channelorderCB->currentData().toInt());
  profile.defaultMode(ui->stickmodeCB->currentData().toInt());
  profile.burnFirmware(ui->burnFirmware->isChecked());
  profile.sdPath(ui->sdPath->text());
  profile.pBackupDir(ui->profilebackupPath->text());
  profile.penableBackup(ui->pbackupEnable->isChecked());
  profile.splashFile(ui->SplashFileName->text());
  profile.runSDSync(ui->chkPromptSDSync->isChecked());

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

  ui->snapshotClipboardCKB->setChecked(g.snapToClpbrd());
  ui->burnFirmware->setChecked(profile.burnFirmware());
  ui->snapshotPath->setText(g.snapshotDir());
  ui->snapshotPath->setReadOnly(true);
  if (ui->snapshotClipboardCKB->isChecked()) {
    ui->snapshotPath->setDisabled(true);
    ui->snapshotPathButton->setDisabled(true);
  }

  ui->showSplash->setChecked(g.showSplash());
  ui->sortProfiles->setChecked(g.sortProfiles());
  ui->chkPromptProfile->setChecked(g.promptProfile());
  ui->historySize->setValue(g.historySize());
  ui->backLightColor->setCurrentIndex(g.backLight());
  ui->volumeGain->setValue(profile.volumeGain() / 10.0);

  if (IS_HORUS_OR_TARANIS(getCurrentBoard())) {
    ui->backLightColor->setEnabled(false);
  }

  ui->simuSW->setChecked(g.simuSW());
  ui->joystickWarningCB->setChecked(g.disableJoystickWarning());
  ui->opt_removeBlankSlots->setChecked(g.removeModelSlots());
  ui->cboNewModelAction->addItems(AppData::newModelActionsList());
  ui->cboNewModelAction->setCurrentIndex(g.newModelAction());
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
  ui->chkPromptInstall->setChecked(g.runAppInstaller());

#if defined(JOYSTICKS)
  ui->joystickChkB->setChecked(g.jsSupport());
  if (ui->joystickChkB->isChecked()) {
    QStringList joystickNames;
    joystickNames << tr("No joysticks found");
    joystick = new Joystick(0,0,false,0);
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
    int stick = joystick->findCurrent(g.currentProfile().jsName());
    ui->joystickCB->setCurrentIndex(stick);
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
  ui->externalModuleSizeCB->setModel(Boards::externalModuleSizeItemModel());
  ui->externalModuleSizeCB->setCurrentIndex(ui->externalModuleSizeCB->findData(profile.externalModuleSize()));
  panelItemModels->getItemModel(FIM_TEMPLATESETUP)->setFilterFlags(Boards::isAir() ? GeneralSettings::RadioTypeContextAir :
                                                                                     GeneralSettings::RadioTypeContextSurface);
  ui->channelorderCB->setModel(panelItemModels->getItemModel(FIM_TEMPLATESETUP));
  ui->channelorderCB->setCurrentIndex(ui->channelorderCB->findData(profile.channelOrder()));
  ui->stickmodeCB->setModel(GeneralSettings::stickModeItemModel());
  ui->stickmodeCB->setCurrentIndex(ui->stickmodeCB->findData(profile.defaultMode()));
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

  if (Boards::isSurface()) {
    ui->stickmodeLabel->hide();
    ui->stickmodeCB->hide();
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
  ui->chkPromptSDSync->setChecked(profile.runSDSync());

  QString currType = QStringList(profile.fwType().split('-').mid(0, 2)).join('-');
  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    ui->boardCB->addItem(firmware->getName(), firmware->getId());
    if (currType == firmware->getId()) {
      ui->boardCB->setCurrentIndex(ui->boardCB->count() - 1);
    }
  }

  populateFirmwareOptions(getBaseFirmware());

  //  Updates tab

  ui->cboUpdateCheckFreq->addItems(AppData::updateCheckFreqsList());

  connect(ui->btnResetUpdatesToDefaults, &QPushButton::clicked, [=]() {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Reset all update settings to defaults. Are you sure?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      g.resetUpdatesSettings();
      QMessageBox::warning(this, CPN_STR_APP_NAME,
                           tr("Update settings have been reset. Please close and restart Companion to avoid unexpected behaviour!"));
      loadUpdatesTab();
    }
  });

  connect(ui->chkDecompressDirUseDwnld, &QCheckBox::toggled, [=](const bool checked) {
    if (!checked) {
      ui->leDecompressDir->setText(g.decompressDir());
      ui->leDecompressDir->setEnabled(true);
      ui->btnDecompressSelect->setEnabled(true);
      ui->chkDelDownloads->setEnabled(true);
    }
    else {
      ui->leDecompressDir->setText(g.downloadDir());
      ui->leDecompressDir->setEnabled(false);
      ui->btnDecompressSelect->setEnabled(false);
      if (ui->chkDelDecompress->isChecked()) {
        ui->chkDelDownloads->setEnabled(true);
      }
      else {
        ui->chkDelDownloads->setEnabled(false);
        ui->chkDelDownloads->setChecked(false);
      }
    }
  });

  connect(ui->chkUpdateDirUseSD, &QCheckBox::toggled, [=](const bool checked) {
    if (!checked) {
      ui->leUpdateDir->setText(g.updateDir());
      ui->leUpdateDir->setEnabled(true);
      ui->btnUpdateSelect->setEnabled(true);
    }
    else {
      ui->leUpdateDir->setText(g.currentProfile().sdPath());
      ui->leUpdateDir->setEnabled(false);
      ui->btnUpdateSelect->setEnabled(false);
    }
  });

  connect(ui->btnDownloadSelect, &QPushButton::clicked, [=]() {
    QString dirPath = QFileDialog::getExistingDirectory(this,tr("Select your download folder"), g.downloadDir());
    if (!dirPath.isEmpty()) {
      ui->leDownloadDir->setText(dirPath);
    }
  });

  connect(ui->btnDecompressSelect, &QPushButton::clicked, [=]() {
    QString dirPath = QFileDialog::getExistingDirectory(this,tr("Select your decompress folder"), g.decompressDir());
    if (!dirPath.isEmpty()) {
      ui->leDecompressDir->setText(dirPath);
    }
  });

  connect(ui->btnUpdateSelect, &QPushButton::clicked, [=]() {
    QString dirPath = QFileDialog::getExistingDirectory(this,tr("Select your update destination folder"), g.updateDir());
    if (!dirPath.isEmpty()) {
      ui->leUpdateDir->setText(dirPath);
    }
  });

  connect(ui->chkDelDecompress, &QCheckBox::stateChanged, [=](const int checked) {
    if (!checked) {
      if (ui->chkDecompressDirUseDwnld->isChecked()) {
        ui->chkDelDownloads->setEnabled(false);
        ui->chkDelDownloads->setChecked(false);
      }
    }
    else {
      ui->chkDelDownloads->setEnabled(true);
    }
  });

  int row = 0;
  int col = 0;

  QGridLayout *grid = new QGridLayout();

  col++;  //  leave col 0 blank

  QLabel *h1 = new QLabel(tr("Check"));
  grid->addWidget(h1, row, col++);

  QLabel *h2 = new QLabel(tr("Release channel"));
  grid->addWidget(h2, row, col++);

  col++;  // options button

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  grid->addItem(spacer, row, col++);

  QMapIterator<QString, int> it(factories->sortedComponentsList());

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    row++;
    col = 0;

    lblName[i] = new QLabel();
    grid->addWidget(lblName[i], row, col++);

    chkCheckForUpdate[i] = new QCheckBox();
    grid->addWidget(chkCheckForUpdate[i], row, col++);
    grid->setAlignment(chkCheckForUpdate[i], Qt::AlignHCenter);

    cboReleaseChannel[i] = new QComboBox();
    cboReleaseChannel[i]->addItems(ComponentData::releaseChannelsList());
    grid->addWidget(cboReleaseChannel[i], row, col++);

    btnComponentOptions[i] = new QPushButton(tr("Options"));
    connect(btnComponentOptions[i], &QPushButton::clicked, [=]() {
      UpdateOptionsDialog *dlg = new UpdateOptionsDialog(this, factories->instance(i), i, false);
      dlg->exec();
      dlg->deleteLater();
    });
    grid->addWidget(btnComponentOptions[i], row, col++);
  }

  ui->grpComponents->setLayout(grid);

  ui->cboLogLevel->addItems(AppData::updateLogLevelsList());

  loadUpdatesTab();
}

void AppPreferencesDialog::loadUpdatesTab()
{
  ui->cboUpdateCheckFreq->setCurrentIndex(g.updateCheckFreq());
  ui->chkDelDownloads->setChecked(g.updDelDownloads());
  ui->chkDelDecompress->setChecked(g.updDelDecompress());
  ui->leDownloadDir->setText(g.downloadDir());
  //  trigger toggled signal by changing design value and then setting to saved value
  ui->chkDecompressDirUseDwnld->setChecked(!ui->chkDecompressDirUseDwnld->isChecked());
  ui->chkDecompressDirUseDwnld->setChecked(g.decompressDirUseDwnld());

  if (g.currentProfile().sdPath().trimmed().isEmpty())
    ui->chkUpdateDirUseSD->setEnabled(false);
  else
    ui->chkUpdateDirUseSD->setEnabled(true);

  if (g.updateDirUseSD() && g.currentProfile().sdPath().trimmed().isEmpty()) {
    g.updateDirUseSD(false);
    g.updateDirReset();
  }

  if (g.updateDirUseSD()) {
    //  trigger toggled signal by changing design value and then setting to saved value
    ui->chkUpdateDirUseSD->setChecked(!ui->chkUpdateDirUseSD->isChecked());
    ui->chkUpdateDirUseSD->setChecked(g.updateDirUseSD());
  }
  else
    ui->chkUpdateDirUseSD->setChecked(false);

  ui->chkDelDownloads->setChecked(g.updDelDownloads());
  ui->cboLogLevel->setCurrentIndex(g.updLogLevel());

  QMapIterator<QString, int> it(factories->sortedComponentsList());

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    lblName[i]->setText(it.key());
    chkCheckForUpdate[i]->setChecked(g.component[i].checkForUpdate());
    cboReleaseChannel[i]->setCurrentIndex(g.component[i].releaseChannel());
  }
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
    joystick = new Joystick(0,0,false,0);
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
  g.currentProfile().jsName(ui->joystickCB->currentText());
  joystickDialog * jd = new joystickDialog(this);
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
  ui->imageLabel->setFixedSize(Boards::getCapability(getCurrentBoard(), Board::LcdWidth),
                               Boards::getCapability(getCurrentBoard(), Board::LcdHeight));
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
  QTimer::singleShot(50, this, &AppPreferencesDialog::shrink);
}

void AppPreferencesDialog::shrink()
{
  adjustSize();
}
