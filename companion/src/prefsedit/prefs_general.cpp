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

#include "prefs_general.h"
#include "ui_prefs_general.h"
#include "appdata.h"

PrefsGeneralPanel::PrefsGeneralPanel(QWidget * parent):
  PrefsPanel(parent),
  ui(new Ui::General)
{
  ui->setupUi(this);
  lock = true;

  connect(ui->opt_appDebugLog, &QCheckBox::toggled, this, &PrefsGeneralPanel::toggleAppLogPrefs);
  connect(ui->opt_fwTraceLog, &QCheckBox::toggled, this, &PrefsGeneralPanel::toggleAppLogPrefs);
  connect(ui->backupPath, &QLineEdit::editingFinished, this, &PrefsGeneralPanel::onBackupPathEditingFinished);
  connect(ui->backupPathButton, &QPushButton::clicked, this, &PrefsGeneralPanel::onBackupPathButtonClicked);

  update();
  lock = false;
}

PrefsGeneralPanel::~PrefsGeneralPanel()
{
  delete ui;
}


bool PrefsGeneralPanel::save()
{
  g.showSplash(ui->showSplash->isChecked());
  g.sortProfiles(ui->sortProfiles->isChecked());
  g.promptProfile(ui->chkPromptProfile->isChecked());
  g.removeModelSlots(ui->opt_removeBlankSlots->isChecked());
  g.newModelAction((AppData::NewModelAction)ui->cboNewModelAction->currentIndex());
  g.historySize(ui->historySize->value());
  profile.volumeGain(round(ui->volumeGain->value() * 10.0));
  g.libDir(ui->libraryPath->text());
  g.gePath(ui->ge_lineedit->text());
  g.embedSplashes(ui->splashincludeCB->currentIndex());
  g.enableBackup(ui->backupEnable->isChecked());

  g.appDebugLog(ui->opt_appDebugLog->isChecked());
  g.fwTraceLog(ui->opt_fwTraceLog->isChecked());
  g.appLogsDir(ui->appLogsDir->text());
  g.runAppInstaller(ui->chkPromptInstall->isChecked());
  g.useSavedSettings(ui->chkUseSavedSettingsApp->isChecked());

}

void PrefsGeneralPanel::update()
{
  ui->showSplash->setChecked(g.showSplash());
  ui->sortProfiles->setChecked(g.sortProfiles());
  ui->chkPromptProfile->setChecked(g.promptProfile());
  ui->historySize->setValue(g.historySize());
  ui->opt_removeBlankSlots->setChecked(g.removeModelSlots());
  ui->cboNewModelAction->addItems(AppData::newModelActionsList());
  ui->cboNewModelAction->setCurrentIndex(g.newModelAction());
  ui->libraryPath->setText(g.libDir());
  ui->ge_lineedit->setText(g.gePath());

  ui->backupPath->setText(g.backupDir());
  if (!ui->backupPath->text().isEmpty() && QDir(ui->backupPath->text()).exists()) {
    ui->backupEnable->setEnabled(true);
    ui->backupEnable->setChecked(g.enableBackup());
  } else {
    ui->backupEnable->setChecked(false);
    ui->backupEnable->setEnabled(false);
  }

  ui->splashincludeCB->setCurrentIndex(g.embedSplashes());

  ui->opt_appDebugLog->setChecked(g.appDebugLog());
  ui->opt_fwTraceLog->setChecked(g.fwTraceLog());
  ui->appLogsDir->setText(g.appLogsDir());
  toggleAppLogPrefs();
  ui->chkPromptInstall->setChecked(g.runAppInstaller());

}

void PrefsGeneralPanel::on_libraryPathButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your library folder"), g.libDir());
  if (!fileName.isEmpty()) {
    g.libDir(fileName);
    ui->libraryPath->setText(fileName);
  }
}

void PrefsGeneralPanel::onBackupPathButtonClicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this,tr("Select your global backup folder"), g.backupDir());
  if (!fileName.isEmpty()) {
    g.backupDir(fileName);
    ui->backupPath->setText(fileName);
    ui->backupEnable->setEnabled(true);
    ui->profileBackupEnable->setEnabled(true);
  } else {
    ui->backupEnable->setEnabled(false);
    if (!g.currentProfile().pBackupDir().isEmpty() && QFileInfo(g.currentProfile().pBackupDir()).exists()) {
      ui->profileBackupEnable->setEnabled(true);
    } else {
      ui->profileBackupEnable->setEnabled(false);
    }
  }
}

void PrefsGeneralPanel::on_btn_appLogsDir_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this, tr("Select a folder for application logs"), ui->appLogsDir->text());
  if (!fileName.isEmpty()) {
    ui->appLogsDir->setText(fileName);
  }
}

void PrefsGeneralPanel::on_ge_pathButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Select Google Earth executable"),ui->ge_lineedit->text());
  if (!fileName.isEmpty()) {
    ui->ge_lineedit->setText(fileName);
  }
}

bool PrefsGeneralPanel::displayImage(const QString & fileName)
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

void PrefsGeneralPanel::on_SplashSelect_clicked()
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

void PrefsGeneralPanel::on_clearImageButton_clicked()
{
  ui->imageLabel->clear();
  ui->SplashFileName->clear();
}

void PrefsGeneralPanel::toggleAppLogPrefs()
{
  bool vis = (ui->opt_appDebugLog->isChecked() || ui->opt_fwTraceLog->isChecked());
  ui->appLogsDir->setVisible(vis);
  ui->lbl_appLogsDir->setVisible(vis);
  ui->btn_appLogsDir->setVisible(vis);
}

void PrefsGeneralPanel::onBackupPathEditingFinished()
{
  if(!ui->backupPath->text().isEmpty() && QFileInfo(ui->backupPath->text()).exists()) {
    ui->backupEnable->setEnabled(true);
  } else {
    ui->backupEnable->setChecked(false);
    ui->backupEnable->setEnabled(false);
  }

  onProfileBackupPathEditingFinished();
}


