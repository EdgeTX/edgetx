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

#include "flashfirmwaredialog.h"
#include "ui_flashfirmwaredialog.h"
#include "appdata.h"
#include "helpers.h"
#include "progressdialog.h"
#include "radiointerface.h"
#include "progresswidget.h"
#include "splashlibrarydialog.h"
#include "storage.h"

#if defined _MSC_VER || !defined __GNUC__
  #include <windows.h>
  #define sleep(x) Sleep(x*1000)
#else
  #include <unistd.h>
#endif

FlashFirmwareDialog::FlashFirmwareDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlashFirmwareDialog),
  fwName(g.profile[g.id()].fwName())
{
  ui->setupUi(this);

  if (!g.profile[g.id()].splashFile().isEmpty()){
    imageSource = PROFILE;
    imageFile = g.profile[g.id()].splashFile();
    ui->useProfileSplash->setChecked(true);
  }
  else {
    imageSource = FIRMWARE;
    imageFile = "";
    ui->useProfileSplash->setDisabled(true);
  }

  if (IS_STM32(getCurrentBoard())) {
    // No backup on Taranis ... could be done if in massstorage
    ui->backupEEprom->hide();
    ui->backupEEprom->setCheckState(Qt::Unchecked);
  }
  else {
    ui->backupEEprom->setCheckState(g.backupOnFlash() ? Qt::Checked : Qt::Unchecked);
  }

  QString backupPath = g.profile[g.id()].pBackupDir();
  if (backupPath.isEmpty()) {
    backupPath=g.backupDir();
  }
  if (backupPath.isEmpty() || !QDir(backupPath).exists()) {
    ui->backupEEprom->setEnabled(false);
  }

  ui->checkHardwareCompatibility->setChecked(g.checkHardwareCompatibility());

  updateUI();

  resize(0, 0); // TODO needed?
}

FlashFirmwareDialog::~FlashFirmwareDialog()
{
  delete ui;
}

void FlashFirmwareDialog::updateUI()
{
  ui->firmwareFilename->setText(fwName);
  ui->burnButton->setEnabled(QFile(fwName).exists());

  FirmwareInterface firmware(fwName);
  if (firmware.isValid()) {
    ui->firmwareInfoFrame->show();
    ui->date->setText(firmware.getDate() + " " + firmware.getTime());
    ui->version->setText(firmware.getVersion());
    ui->variant->setText(firmware.getEEpromId());
    ui->date->setEnabled(true);
    ui->version->setEnabled(true);
    ui->variant->setEnabled(true);
    if (firmware.hasSplash()) {
      ui->splashFrame->show();
      ui->splash->setFixedSize(firmware.getSplashWidth(), firmware.getSplashHeight());
    }
    else {
      ui->splashFrame->hide();
    }
  }
  else {
    imageSource = FIRMWARE;
    ui->firmwareInfoFrame->hide();
    ui->splashFrame->hide();
  }

  QImage image;
  switch (imageSource) {
    case FIRMWARE:
      ui->useFirmwareSplash->setChecked(true);
      image = firmware.getSplash();
      break;
    case PROFILE:
      ui->useProfileSplash->setChecked(true);
      image.load(g.profile[g.id()].splashFile());
      break;
    case LIBRARY:
      ui->useLibrarySplash->setChecked(true);
      image.load(imageFile);
      break;
    case EXTERNAL:
      ui->useExternalSplash->setChecked(true);
      image.load(imageFile);
      break;
  }

  if (!image.isNull()) {
    ui->splash->setPixmap(makePixMap(image));
  }
}

void FlashFirmwareDialog::on_firmwareLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Firmware File"), g.flashDir(), FLASH_FILES_FILTER);
  if (!fileName.isEmpty()) {
    fwName = fileName;
    if (!fwName.isEmpty() && !fwName.endsWith(".dfu") && !FirmwareInterface(fwName).isValid()) {
      QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 may not be a valid firmware file").arg(fwName));
    }
    updateUI();
  }
}

void FlashFirmwareDialog::on_useFirmwareSplash_clicked()
{
  FirmwareInterface firmware(fwName);
  if (!firmware.isValid()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR, tr( "The firmware file is not valid." ));
  }
  else if (!firmware.hasSplash()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR, tr( "There is no start screen image in the firmware file." ));
  }
  else {
    imageSource = FIRMWARE;
  }
  updateUI();
}

void FlashFirmwareDialog::on_useProfileSplash_clicked()
{
  QString fileName = g.profile[g.id()].splashFile();
  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Profile image %1 is invalid.").arg(fileName));
    }
    else {
      imageSource = PROFILE;
    }
  }
  updateUI();
}

void FlashFirmwareDialog::on_useExternalSplash_clicked()
{
  QString supportedImageFormats;
  for (int formatIndex = 0; formatIndex < QImageReader::supportedImageFormats().count(); formatIndex++) {
    supportedImageFormats += QLatin1String(" *.") + QImageReader::supportedImageFormats()[formatIndex];
  }
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open image file to use as radio start screen"), g.imagesDir(), tr("Images (%1)").arg(supportedImageFormats));
  if (!fileName.isEmpty()){
    g.imagesDir( QFileInfo(fileName).dir().absolutePath() );
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Image could not be loaded from %1").arg(fileName));
    }
    else{
      imageSource = EXTERNAL;
      imageFile = fileName;
    }
  }
  updateUI();
}

void FlashFirmwareDialog::on_useLibrarySplash_clicked()
{
  QString fileName;
  auto ld = new SplashLibraryDialog(this, &fileName);
  ld->exec();
  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("The library image could not be loaded"));
    }
    else {
      imageSource = LIBRARY;
      imageFile = fileName;
    }
  }
  updateUI();
}

void FlashFirmwareDialog::on_burnButton_clicked()
{
  g.flashDir(QFileInfo(fwName).dir().absolutePath());
  g.profile[g.id()].fwName(fwName);
  g.checkHardwareCompatibility(ui->checkHardwareCompatibility->isChecked());
  g.backupOnFlash(ui->backupEEprom->isChecked());

  qDebug() << "FlashFirmwareDialog: flashing" << fwName;

  if (imageSource != FIRMWARE) {
    // load the splash image
    const QPixmap pixmap = ui->splash->pixmap(Qt::ReturnByValue);
    QImage image;
    image = pixmap.toImage().scaled(ui->splash->width(), ui->splash->height());
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Splash image not found"));
      return;
    }
    // write the customized firmware
    QString tempFile;
    if (getStorageType(fwName) == STORAGE_TYPE_HEX)
      tempFile = generateProcessUniqueTempFileName("flash.hex");
    else
      tempFile = generateProcessUniqueTempFileName("flash.bin");
    qDebug() << "FlashFirmwareDialog: patching" << fwName << "with custom splash screen and saving to" << tempFile;
    FirmwareInterface firmware(fwName);
    firmware.setSplash(image);
    if (firmware.save(tempFile) <= 0) {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Cannot save customized firmware"));
      return;
    }
    startFlash(tempFile);
  }
  else {
    startFlash(fwName);
  }
}

void FlashFirmwareDialog::on_cancelButton_clicked()
{
  close();
}

void FlashFirmwareDialog::shrink()
{
  resize(0, 0);
}

void FlashFirmwareDialog::startFlash(const QString &filename)
{
//  bool backup = g.backupOnFlash();

  close();

  ProgressDialog progressDialog(this, tr("Write Firmware to Radio"), CompanionIcon("write_flash.png"));

  // check hardware compatibility if requested
  bool checkPassed = true;
  if (g.checkHardwareCompatibility()) {
    QString tempFirmware = generateProcessUniqueTempFileName("flash-check.bin");
    if (!readFirmware(tempFirmware, progressDialog.progress())) {
      QMessageBox::warning(this, tr("Firmware check failed"), tr("Could not check firmware from radio"));
      checkPassed = false;
    }
    FirmwareInterface previousFirmware(tempFirmware);
    qunlink(tempFirmware);
    FirmwareInterface newFirmware(filename);
    qDebug() << "startFlash: checking firmware compatibility between " << tempFirmware << "and" << filename;
    if (!newFirmware.isHardwareCompatible(previousFirmware)) {
      QMessageBox::warning(this, tr("Firmware check failed"), tr("New firmware is not compatible with the one currently installed!"));
      if (isTempFileName(filename)) {
        qDebug() << "startFlash: removing temporary file" << filename;
        qunlink(filename);
      }
      checkPassed = false;
    }
  }

  if (checkPassed) {
    // backup if requested
//    bool result = true;
//    QString backupFilename;
//    QString backupPath;
//    if (backup) {
//      backupPath = g.profile[g.id()].pBackupDir();
//      if (backupPath.isEmpty()) {
//        backupPath=g.backupDir();
//      }
//      backupFilename = backupPath + "/backup-" + QDateTime().currentDateTime().toString("yyyy-MM-dd-HHmmss") + ".bin";
//      result = readSettings(backupFilename, progressDialog.progress());
//      sleep(2);
//    }

    // flash
    //result = (result && writeFirmware(filename, progressDialog.progress()));
    writeFirmware(filename, progressDialog.progress());

    // restore if backup requested
//    if (backup && result) {
//      sleep(2);
//      QString restoreFilename = generateProcessUniqueTempFileName("restore.bin");
//      if (!convertEEprom(backupFilename, restoreFilename, filename)) {
//        QMessageBox::warning(this, tr("Conversion failed"), tr("Cannot convert Models and Settings for use with this firmware, original data will be used"));
//        restoreFilename = backupFilename;
//      }
//      if (!writeSettings(restoreFilename, progressDialog.progress())) {
//        QMessageBox::warning(this, tr("Restore failed"), tr("Could not restore Models and Settings to Radio. The models and settings data file can be found at: %1").arg(backupFilename));
//      }
//    }

    progressDialog.progress()->setInfo(tr("Flashing done"));
  }

  progressDialog.exec();

  if (isTempFileName(filename)) {
    qDebug() << "startFlash: removing temporary file" << filename;
    qunlink(filename);
  }
}
