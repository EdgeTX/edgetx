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
#include "radiointerface.h"
#include "firmwareinterface.h"
#include "progressdialog.h"
#include "progresswidget.h"
#include "splashlibrarydialog.h"
#include "storage.h"

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
  } else {
    imageSource = FIRMWARE;
    imageFile = "";
    ui->useProfileSplash->setDisabled(true);
  }

  ui->checkHardwareCompatibility->setChecked(g.checkHardwareCompatibility());
  ui->checkBackup->setChecked(g.backupOnFlash());
  QString backupPath = g.profile[g.id()].pBackupDir();
  if (backupPath.isEmpty())
    backupPath = g.backupDir();

  if (backupPath.isEmpty() || !QDir(backupPath).exists())
    ui->checkBackup->setEnabled(false);

  detectClicked(true);
  updateUI();

  connect(ui->detectButton, &QPushButton::clicked, this, &FlashFirmwareDialog::detectClicked);
  connect(ui->firmwareLoad, &QPushButton::clicked, this, &FlashFirmwareDialog::firmwareLoadClicked);
  connect(ui->useProfileSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useProfileSplashClicked);
  connect(ui->useFirmwareSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useFirmwareSplashClicked);
  connect(ui->useLibrarySplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useLibrarySplashClicked);
  connect(ui->useExternalSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useExternalSplashClicked);
  connect(ui->writeButton, &QPushButton::clicked, this, &FlashFirmwareDialog::writeButtonClicked);
  connect(ui->cancelButton, &QPushButton::clicked, [=]() { close(); } );

}

FlashFirmwareDialog::~FlashFirmwareDialog()
{
  delete ui;
}

void FlashFirmwareDialog::updateUI()
{
  ui->firmwareFilename->setText(fwName);

  if (connectionMode) {
    ui->firmwareLoad->setEnabled(true);
    ui->writeButton->setEnabled(isFileConnectionCompatible() && QFile(fwName).exists());
  } else {
    ui->firmwareLoad->setEnabled(false);
    ui->writeButton->setEnabled(false);
  }

  FirmwareInterface firmware(fwName);
  if (firmware.isValid()) {
    ui->firmwareInfoFrame->show();
    ui->date->setText(firmware.getDate() + " " + firmware.getTime());
    ui->version->setText(firmware.getVersion());
    ui->variant->setText(firmware.getFlavour());
    ui->date->setEnabled(true);
    ui->version->setEnabled(true);
    ui->variant->setEnabled(true);

    if (firmware.hasSplash()) {
      ui->splashFrame->show();
      ui->splash->setFixedSize(firmware.getSplashWidth(), firmware.getSplashHeight());
    } else {
      ui->splashFrame->hide();
    }
  } else {
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

  if (!image.isNull())
    ui->splash->setPixmap(makePixMap(image));

  QTimer::singleShot(0, [=]() { adjustSize(); });
}

void FlashFirmwareDialog::firmwareLoadClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Firmware File"), g.flashDir(), getFirmwareFilesFilter());
  if (!fileName.isEmpty()) {
    fwName = fileName;

    if (!fwName.endsWith(".bin") && !fwName.endsWith(".uf2"))
      QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 may not be a valid firmware file").arg(fwName));

    FirmwareInterface fw(fwName);

    if (!fw.isValid())
      QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 may not be a valid firmware file").arg(fwName));

    if (connectionMode == CONNECTION_UF2) {
      const Uf2Info uf2(getUf2Info());
      if (fw.getFlavour() != uf2.board) {
        QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 \nRadio type mismatch - Firmware: %2 Radio: %3")
                                                        .arg(fwName)
                                                        .arg(fw.getFlavour())
                                                        .arg(uf2.board));
      }
    }

    if (fw.getFlavour() != getCurrentFirmware()->getFlavour()) {
      QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 \nRadio type mismatch - Firmware: %2 Profile: %3")
                                                      .arg(fwName)
                                                      .arg(fw.getFlavour())
                                                      .arg(getCurrentFirmware()->getFlavour()));
    }

    updateUI();
  }
}

void FlashFirmwareDialog::useFirmwareSplashClicked()
{
  FirmwareInterface firmware(fwName);

  if (!firmware.isValid()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR, tr( "The firmware file is not valid." ));
  } else if (!firmware.hasSplash()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR, tr( "There is no start screen image in the firmware file." ));
  } else {
    imageSource = FIRMWARE;
  }

  updateUI();
}

void FlashFirmwareDialog::useProfileSplashClicked()
{
  QString fileName = g.profile[g.id()].splashFile();

  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Profile image %1 is invalid.").arg(fileName));
    } else {
      imageSource = PROFILE;
    }
  }

  updateUI();
}

void FlashFirmwareDialog::useExternalSplashClicked()
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
    } else {
      imageSource = EXTERNAL;
      imageFile = fileName;
    }
  }

  updateUI();
}

void FlashFirmwareDialog::useLibrarySplashClicked()
{
  QString fileName;
  SplashLibraryDialog(this, &fileName).exec();

  if (!fileName.isEmpty()) {
    QImage image(fileName);

    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("The library image could not be loaded"));
    } else {
      imageSource = LIBRARY;
      imageFile = fileName;
    }
  }

  updateUI();
}

void FlashFirmwareDialog::writeButtonClicked()
{
  g.flashDir(QFileInfo(fwName).dir().absolutePath());
  g.profile[g.id()].fwName(fwName);
  g.checkHardwareCompatibility(ui->checkHardwareCompatibility->isChecked());
  g.backupOnFlash(ui->checkBackup->isChecked());

  qDebug() << "flashing:" << fwName;

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
    tempFile = generateProcessUniqueTempFileName("flash.bin");
    qDebug() << "patching" << fwName << "with custom splash screen and saving to" << tempFile;
    FirmwareInterface firmware(fwName);
    firmware.setSplash(image);

    if (firmware.save(tempFile) <= 0) {
      QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Cannot save customised firmware"));
      return;
    }

    startFlash(tempFile);
  } else {
    startFlash(fwName);
  }
}

void FlashFirmwareDialog::startFlash(const QString &filename)
{
  close();

  ProgressDialog progressDialog(this, tr("Write Firmware to Radio"),
                                CompanionIcon("write_flash.png"));

  FirmwareInterface fw(filename);
  auto progress = progressDialog.progress();

  bool backup = g.backupOnFlash();
  QString backupPath;

  if (backup) {
    QString backupDir = g.currentProfile().pBackupDir();
    if (backupDir.isEmpty())
      backupDir = g.backupDir();

    if (!QFileInfo::exists(backupDir)) {
      QMessageBox::critical(this, tr("Backup Radio Firmware"),
                            tr("Directory %1 does not exist!")
                            .arg(backupDir));
      return;
    }

    // include time in file name as there could be multiple backups in a day and
    // we do not want to replace earlier copies
    backupPath = QString("%1/fw-%2-%3-%4.%5")
                        .arg(backupDir)
                        .arg(getCurrentFirmware()->getFlavour())
                        .arg(QDate(QDate::currentDate()).toString("yyyyMMdd"))
                        .arg(QTime(QTime::currentTime()).toString("HHmm"))
                        .arg(fw.typeFileExtn());
  }

  if (g.checkHardwareCompatibility() || backup) {
    bool checkHw = g.checkHardwareCompatibility();
    readFirmware(
        [this, &fw, progress, checkHw, backup, &backupPath](const QByteArray &_data) {
          qDebug() << "Read old firmware, size = " << _data.size();
          bool backupFail = false;
          bool checkHwFail = false;

          if (backup) {
            qDebug() << "Writing backup to" << backupPath;
            progress->addMessage(tr("Writing backup to: %1").arg(backupPath));
            QFile backupFile(backupPath);
            if (!backupFile.open(QIODevice::ReadWrite)) {
              QString errMsg(tr("Unable to open backup file: %1 (reason: %2)")
                                .arg(backupFile.fileName())
                                .arg(backupFile.errorString()));
              progress->addMessage(errMsg, QtFatalMsg);
              QMessageBox::critical(this, tr("Backup Radio Firmware"), errMsg);
            }

            if (backupFile.write(_data) <= 0) {
              qDebug() << "Backup failed";
              backupFail = true;
              QString errMsg(tr("Error writing to file: %1 (reason: %2)")
                                .arg(backupFile.fileName())
                                .arg(backupFile.errorString()));
              progress->addMessage(errMsg, QtFatalMsg);
              QMessageBox::critical(this, tr("Backup Radio Firmware"), errMsg);
            } else {
              qDebug() << "Backup written";
              progress->addMessage(tr("Backup finished"));
            }

            backupFile.close();
          }

          if (!backupFail && checkHw && !fw.isHardwareCompatible(FirmwareInterface(_data))) {
            qDebug() << "Firmware not compatible";
            checkHwFail = true;
            QString errMsg(tr("New firmware not compatible"));
            progress->addMessage(errMsg, QtFatalMsg);
            QMessageBox::critical(this, tr("Firmware Compatibility Check"), errMsg);
          }

          if (!backupFail && !checkHwFail) {
            qDebug() << "Start flashing firmware (if requested, backup and checks done)";
            if (writeFirmware(fw.getFlash(), progress)) {
              qDebug() << "Flashing firmware complete";
            } else {
              qDebug() << "Flashing firmware error";
            }
          } else {
            qDebug() << "Abort flashing firmware as backup or checks failed";
          }
        },
        [this](const QString &err) {
          QMessageBox::critical(this, tr("Flashing Firmware"),
                                tr("Could not read current firmware: %1").arg(err));
        },
        progress);
  } else {
    qDebug() << "Start flashing firmware (no backup or checks done)";
    if (writeFirmware(fw.getFlash(), progress)) {
      qDebug() << "Flashing firmware complete";
    } else {
      qDebug() << "Flashing firmware error";
    }
  }

  progressDialog.exec();

  if (isTempFileName(filename)) {
    qDebug() << "Removing temporary file" << filename;
    qunlink(filename);
  }
}

void FlashFirmwareDialog::detectClicked(bool atLoad)
{
  QString connectionMsg;

  if (isUf2DeviceFound()) {
    connectionMode = CONNECTION_UF2;
    connectionMsg = tr("Radio connection mode: UF2");
  } else if (isDfuDeviceFound()) {
    connectionMode = CONNECTION_DFU;
    connectionMsg = tr("Radio connection mode: DFU");
  } else {
    connectionMode = CONNECTION_NONE;
    connectionMsg = tr("ALERT: No radio connected");
  }

  ui->connectionMode->setText(connectionMsg);

  if (!connectionMode) {
    QMessageBox::critical((atLoad ? (QWidget *)parent(): this), tr("Detect Radio"),
      tr("Radio could not be detected by DFU or UF2 modes") % ".\n" %
      tr("Check cable is securely connected and radio lights are illuminated") % ".\n" %
      tr("Note: USB mode is not suitable for flashing firmware."));
  }

  if (!atLoad)
    updateUI();
}

bool FlashFirmwareDialog::isFileConnectionCompatible()
{
  if (connectionMode == CONNECTION_DFU ||
     (connectionMode == CONNECTION_UF2 && !fwName.isEmpty() && QFileInfo(fwName).suffix().toLower() == "uf2"))
    return true;

  return false;
}
