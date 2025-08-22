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
  }
  else {
    imageSource = FIRMWARE;
    imageFile = "";
    ui->useProfileSplash->setDisabled(true);
  }

  ui->checkHardwareCompatibility->setChecked(g.checkHardwareCompatibility());

  updateUI();

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
  ui->writeButton->setEnabled(QFile(fwName).exists());

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

  QTimer::singleShot(0, [=]() { adjustSize(); });
}

void FlashFirmwareDialog::firmwareLoadClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Firmware File"), g.flashDir(), getFlashFilesFilter());
  if (!fileName.isEmpty()) {
    fwName = fileName;
    if (!fwName.isEmpty() && !fwName.endsWith(".bin") && !fwName.endsWith(".uf2") && !FirmwareInterface(fwName).isValid()) {
      QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("%1 may not be a valid firmware file").arg(fwName));
    }
    updateUI();
  }
}

void FlashFirmwareDialog::useFirmwareSplashClicked()
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

void FlashFirmwareDialog::useProfileSplashClicked()
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
    }
    else{
      imageSource = EXTERNAL;
      imageFile = fileName;
    }
  }
  updateUI();
}

void FlashFirmwareDialog::useLibrarySplashClicked()
{
  QString fileName;
  auto ld = new SplashLibraryDialog(this, &fileName);
  ld->exec();
  delete ld;
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

void FlashFirmwareDialog::writeButtonClicked()
{
  g.flashDir(QFileInfo(fwName).dir().absolutePath());
  g.profile[g.id()].fwName(fwName);
  g.checkHardwareCompatibility(ui->checkHardwareCompatibility->isChecked());

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

void FlashFirmwareDialog::startFlash(const QString &filename)
{
  close();

  ProgressDialog progressDialog(this, tr("Write Firmware to Radio"),
                                CompanionIcon("write_flash.png"));

  FirmwareInterface fw(filename);
  auto progress = progressDialog.progress();

  if (g.checkHardwareCompatibility()) {
    readFirmware(
        [this, &fw, progress](const QByteArray &_data) {
          qDebug() << "Read old fw, size = " << _data.size();
          if (!fw.isHardwareCompatible(FirmwareInterface(_data))) {
            QMessageBox::warning(this, tr("Firmware check failed"),
                tr("New firmware is not compatible with the one currently installed!"));
          } else {
            qDebug() << "Start writing firmware (compatibility checked)";
            writeFirmware(fw.getFlash(), progress);
          }
        },
        [this](const QString &err) {
          QMessageBox::critical(this, tr("Firmware check failed"),
              tr("Could not read current firmware: %1").arg(err));
        });
  } else {
    qDebug() << "Start writing firmware (no checks done)";
    writeFirmware(fw.getFlash(), progress);
  }

  progressDialog.exec();

  if (isTempFileName(filename)) {
    qDebug() << "startFlash: removing temporary file" << filename;
    qunlink(filename);
  }
}
