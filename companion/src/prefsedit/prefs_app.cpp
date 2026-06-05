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

#include "prefs_app.h"
#include "ui_prefs_app.h"
#include "appdata.h"

PrefsAppPanel::PrefsAppPanel(QWidget * parent, Firmware * fw, Board::Type & bd, Profile & prof) :
  PrefsPanel(parent, fw, bd, prof),
  ui(new Ui::PrefsApp)
{
  ui->setupUi(this);
  lock = true;

  sectionGeneral();
  sectionRadioProfiles();
  sectionFolders();
  sectionLogging();
  sectionGoogleEarth();
  sectionSplashScreens();

  update();
  shrink();
  lock = false;
}

PrefsAppPanel::~PrefsAppPanel()
{
  delete ui;
}

void PrefsAppPanel::save()
{
  AbstractPanel::save();
}

void PrefsAppPanel::sectionFolders()
{
  QGridLayout *layFolders = ui->csectFolders->start(tr("Folders"));
  row = col = 0;

  AutoLabel *lblBackupsPath = new AutoLabel(this, tr("Backups"));
  layFolders->addWidget(lblBackupsPath, row, col++);

  leBackupsPath = newPathWidget(this);
  leBackupsPath->setValue(g.backupDir());
  leBackupsPath->setBindSave([this] {
    g.backupDir(this->leBackupsPath->text());
  });
  layFolders->addWidget(leBackupsPath, row, col++);

  AutoDirectorySelectButton *btnBackupsPath = new AutoDirectorySelectButton(this);
  btnBackupsPath->setup(tr("Select backups folder"), g.backupDir(), leBackupsPath);;
  layFolders->addWidget(btnBackupsPath, row, col++);

  ++row; col = 0;
  AutoLabel *lblLogsPath = new AutoLabel(this, tr("Logs"));
  layFolders->addWidget(lblLogsPath, row, col++);

  leLogsPath = newPathWidget(this);
  leLogsPath->setValue(g.logDir());
  leLogsPath->setBindSave([this] {
    g.logDir(this->leLogsPath->text());
  });
  layFolders->addWidget(leLogsPath, row, col++);

  AutoDirectorySelectButton *btnLogsPath = new AutoDirectorySelectButton(this);
  btnLogsPath->setup(tr("Select logs folder"), g.logDir(), leLogsPath);;
  layFolders->addWidget(btnLogsPath, row, col++);

  /*  TODO implement
  ++row; col = 0;
  AutoLabel *lblSDPath = new AutoLabel(this, tr("Models"));
  layFolders->addWidget(lblSDPath, row, col++);

  leModelsPath = newPathWidget(this);
  leModelsPath->setValue(g.modelsDir());
  leModelsPath->setBindSave([this] {
    g.modelsDir(this->leModelsPath->text());
  });
  layFolders->addWidget(leModelsPath, row, col++);

  AutoDirectorySelectButton *btnModelsPath = new AutoDirectorySelectButton(this);
  btnModelsPath->setup(tr("Select models folder"), g.modelsDir(), leModelsPath);;
  layFolders->addWidget(btnModelsPath, row, col++);
 */

  ui->csectFolders->finish(-1, -1, [this] { this->shrink(); });
}

void PrefsAppPanel::sectionGeneral()
{
  QGridLayout *layGeneral = ui->csectGeneral->start(tr("General"));
  row = col = 0;
  AutoLabel *lblShowSplash = new AutoLabel(this, tr("Show splash screen"));
  layGeneral->addWidget(lblShowSplash, row, col++);
  chkSplash = new AutoCheckBox(this);
  chkSplash->setValue(g.showSplash());
  chkSplash->setBindSave([this] {
    g.showSplash(this->chkSplash->isChecked());
  });
  layGeneral->addWidget(chkSplash, row, col++);

  ++row; col = 0;
  AutoLabel *lblProfPrmpt = new AutoLabel(this, tr("Prompt for radio profile on startup"));
  layGeneral->addWidget(lblProfPrmpt, row, col++);
  chkProfPrmpt = new AutoCheckBox(this);
  chkProfPrmpt->setValue(g.promptProfile());
  chkProfPrmpt->setBindSave([this] {
    g.promptProfile(this->chkProfPrmpt->isChecked());
  });
  layGeneral->addWidget(chkProfPrmpt, row, col++);

  ++row; col = 0;
  AutoLabel *lblRecentFiles = new AutoLabel(this, tr("Most recently used files"));
  layGeneral->addWidget(lblRecentFiles, row, col++);
  sbxRecentFiles = new AutoSpinBox(this);
  sbxRecentFiles->setRange(1, 50);
  sbxRecentFiles->setValue(g.historySize());
  sbxRecentFiles->setBindSave([this] {
    g.historySize(this->sbxRecentFiles->value());
  });
  layGeneral->addWidget(sbxRecentFiles, row, col++);

  ++row; col = 0;
  AutoLabel *lblModelNew = new AutoLabel(this, tr("New models action"));
  layGeneral->addWidget(lblModelNew, row, col++);
  cboModelNew = new AutoComboBox(this);
  cboModelNew->addItems(AppData::newModelActionsList());
  cboModelNew->setValue((int)g.newModelAction());
  cboModelNew->setBindSave([this] {
    g.newModelAction((AppData::NewModelAction)this->cboModelNew->currentData().toInt());
  });
  layGeneral->addWidget(cboModelNew, row, col++);

  //  TODO implement profile level setting as override
  ++row; col = 0;
  AutoLabel *lblModelDelete = new AutoLabel(this, tr("B&W remove empty slot on delete)"));
  layGeneral->addWidget(lblModelDelete, row, col++);
  chkModelDelete = new AutoCheckBox(this);
  chkModelDelete->setValue(g.removeModelSlots());
  chkModelDelete->setBindSave([this] {
    g.removeModelSlots(this->chkModelDelete->isChecked());
  });
  layGeneral->addWidget(chkModelDelete, row, col++);
  ui->csectGeneral->finish(row, col, [this] { this->shrink(); }, true);
}

void PrefsAppPanel::sectionGoogleEarth()
{
  QGridLayout *layGoogle = ui->csectGoogleEarth->start(tr("Google Earth"));
  row = col = 0;

  AutoLabel *lblGoogleExe = new AutoLabel(this, tr("Executable"));
  layGoogle->addWidget(lblGoogleExe, row, col++);

  leGoogleExe = newPathWidget(this);
  leGoogleExe->setValue(g.gePath());
  leGoogleExe->setBindSave([this] {
    g.gePath(this->leGoogleExe->text());
  });
  layGoogle->addWidget(leGoogleExe, row, col++);

  AutoFileSelectButton *btnGoogleExe = new AutoFileSelectButton(this);
  btnGoogleExe->setup(tr("Select Google Earth executable"), g.gePath(), "", leGoogleExe);;
  layGoogle->addWidget(btnGoogleExe, row, col++);

  ui->csectGoogleEarth->finish(-1, -1, [this] { this->shrink(); });
}

void PrefsAppPanel::sectionLogging()
{
  QGridLayout *layLogging = ui->csectLogging->start(tr("Logging"));
  row = col = 0;

  AutoLabel *lblLogApp = new AutoLabel(this, tr("Application (Companion/Simulator)"));
  layLogging->addWidget(lblLogApp, row, col++);

  chkLogApp = new AutoCheckBox(this);
  chkLogApp->setValue(g.appDebugLog());
  chkLogApp->setBindSave([this] {
    g.appDebugLog(this->chkLogApp->isChecked());
  });
  layLogging->addWidget(chkLogApp, row, col++);

  ++row; col = 0;
  AutoLabel *lblLogFW = new AutoLabel(this, tr("Radio Firmware (in Simulator)"));
  layLogging->addWidget(lblLogFW, row, col++);

  chkLogFW = new AutoCheckBox(this);
  chkLogFW->setValue(g.fwTraceLog());
  chkLogFW->setBindSave([this] {
    g.fwTraceLog(this->chkLogFW->isChecked());
  });
  layLogging->addWidget(chkLogFW, row, col++);

  ui->csectLogging->finish(row, col, [this] { this->shrink(); });
}

void PrefsAppPanel::sectionRadioProfiles()
{
  QGridLayout *layProfiles = ui->csectProfiles->start(tr("Radio Profiles"));
  row = col = 0;

  chkProfileTop = new AutoCheckBox(this, tr("Move selected profile to the top of the list"));
  chkProfileTop->setValue(g.sortProfiles());
  chkProfileTop->setBindSave([this] {
    g.sortProfiles(this->chkProfileTop->isChecked());
  });
  layProfiles->addWidget(chkProfileTop, row, col++);

  ++row; col = 0;
  chkBackupFirmware = new AutoCheckBox(this, tr("Prompt to backup current firmware before flashing new firmware"));
  chkBackupFirmware->setValue(g.enableBackup());
  chkBackupFirmware->setBindSave([this] {
    g.enableBackup(this->chkBackupFirmware->isChecked());
  });
  layProfiles->addWidget(chkBackupFirmware, row, col++);

  ++row; col = 0;
  chkUseSavedSettings = new AutoCheckBox(this, tr("Use radio settings backup for new models and settings files"));
  chkUseSavedSettings->setValue(g.useSavedSettings());
  chkUseSavedSettings->setBindSave([this] {
    g.useSavedSettings(this->chkUseSavedSettings->isChecked());
  });
  layProfiles->addWidget(chkUseSavedSettings, row, col++);

  ++row; col = 0;
  chkConfirmDeleteWrite = new AutoCheckBox(this, tr("Do not prompt to delete exisiting models on write to radio or SD path"));
  chkConfirmDeleteWrite->setValue(g.confirmWriteModelsAndSettings());
  chkConfirmDeleteWrite->setBindSave([this] {
    g.confirmWriteModelsAndSettings(this->chkConfirmDeleteWrite->isChecked());
  });
  layProfiles->addWidget(chkConfirmDeleteWrite, row, col++);

  ui->csectProfiles->finish(row, col, [this] { this->shrink(); });
}

void PrefsAppPanel::sectionSplashScreens()
{
  QGridLayout *laySplash = ui->csectSplash->start(tr("Splash Screens (B&&W radios)"));
  row = col = 0;

  AutoLabel *lblSplashLibraries = new AutoLabel(this, tr("Libraries"));
  laySplash->addWidget(lblSplashLibraries, row, col++);

  cboSplashLibraries = new AutoComboBox(this);
  cboSplashLibraries->addItems(AppData::splashLibsList());
  cboSplashLibraries->setValue(g.embedSplashes());
  cboSplashLibraries->setBindSave([this] {
    g.embedSplashes(this->cboSplashLibraries->currentData().toInt());
  });
  laySplash->addWidget(cboSplashLibraries, row, col++);

  ++row; col = 0;
  AutoLabel *lblSplashUserPath = new AutoLabel(this, tr("User folder"));
  laySplash->addWidget(lblSplashUserPath, row, col++);

  leSplashUserPath = newPathWidget(this);
  leSplashUserPath->setValue(g.libDir());
  leSplashUserPath->setBindSave([this] {
    g.libDir(this->leSplashUserPath->text());
  });
  laySplash->addWidget(leSplashUserPath, row, col++);

  AutoDirectorySelectButton *btnSplashUser = new AutoDirectorySelectButton(this);
  btnSplashUser->setup(tr("Select user splash folder"), g.libDir(), leSplashUserPath);;
  laySplash->addWidget(btnSplashUser, row, col++);

  ui->csectSplash->finish(-1, -1, [this] { this->shrink(); });
}

void PrefsAppPanel::update()
{
  AbstractPanel::update();
}
