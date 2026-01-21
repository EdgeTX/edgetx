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
#include "autocollapsiblesection.h"

#include <QTimer>

FlashFirmwareDialog::FlashFirmwareDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlashFirmwareDialog),
  fwName(g.profile[g.id()].fwName()),
  imageSource(FIRMWARE),
  imageFile(""),
  connectionMode(CONNECTION_NONE)
{
  ui->setupUi(this);
  ui->checkBackup->setChecked(g.backupOnFlash());

  ui->advancedWidget->setTitle(tr("Advanced"));
  ui->advancedWidget->setAnimationDuration(300);
  QVBoxLayout *advLayout = new QVBoxLayout();
  chkHWComp = new QCheckBox(tr("check hardware compatibility (recommended)"), ui->advancedWidget);
  chkHWComp->setChecked(true);
  advLayout->addWidget(chkHWComp);
  chkProfComp = new QCheckBox(tr("check profile compatibility"), ui->advancedWidget);
  chkProfComp->setChecked(true);
  advLayout->addWidget(chkProfComp);
  ui->advancedWidget->setContentLayout(*advLayout);

  const QString splashFile(g.currentProfile().splashFile());

  if (!splashFile.isEmpty() && QFile(splashFile).exists()){
    imageSource = PROFILE;
    imageFile = splashFile;
    ui->useProfileSplash->setChecked(true);
  } else {
    ui->useProfileSplash->setDisabled(true);
  }

  detectClicked(true);
  updateUI();

  connect(ui->detectButton, &QPushButton::clicked, this, &FlashFirmwareDialog::detectClicked);
  connect(ui->loadButton, &QPushButton::clicked, this, &FlashFirmwareDialog::loadClicked);
  connect(ui->useProfileSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useProfileSplashClicked);
  connect(ui->useFirmwareSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useFirmwareSplashClicked);
  connect(ui->useLibrarySplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useLibrarySplashClicked);
  connect(ui->useExternalSplash, &QRadioButton::clicked, this, &FlashFirmwareDialog::useExternalSplashClicked);
  connect(ui->writeButton, &QPushButton::clicked, this, &FlashFirmwareDialog::writeButtonClicked);
  connect(ui->cancelButton, &QPushButton::clicked, [this] () { close(); } );
  connect(ui->advancedWidget, &AutoCollapsibleSection::resized, this, &FlashFirmwareDialog::shrink);
}

FlashFirmwareDialog::~FlashFirmwareDialog()
{
  delete ui;
}

void FlashFirmwareDialog::updateUI()
{
  if (connectionMode)
    ui->loadButton->setEnabled(true);
  else
    ui->loadButton->setEnabled(false);

  if (connectionMode && isFileConnectionCompatible() && QFile(fwName).exists()) {
    ui->firmwareFilename->setText(fwName);
    ui->writeButton->setEnabled(true);

    FirmwareInterface firmware(fwName);

    if (firmware.isValid()) {
      ui->firmwareInfoFrame->show();
      ui->date->setText(firmware.getDate() + " " + firmware.getTime());
      ui->version->setText(firmware.getVersion());
      ui->variant->setText(firmware.getFlavour());

      if (firmware.hasSplash()) {
        ui->splashFrame->show();
        ui->splash->setFixedSize(firmware.getSplashWidth(), firmware.getSplashHeight());

        QImage image;
        switch (imageSource) {
          case FIRMWARE:
            ui->useFirmwareSplash->setChecked(true);
            image = firmware.getSplash();
            break;
          case PROFILE:
            ui->useProfileSplash->setChecked(true);
            image.load(g.currentProfile().splashFile());
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
        else
          ui->splash->setPixmap(QPixmap());

      } else {
        ui->splashFrame->hide();
      }
    } else {
      ui->firmwareInfoFrame->hide();
      ui->splashFrame->hide();
    }
  } else {
    ui->firmwareFilename->setText("");
    ui->writeButton->setEnabled(false);
    ui->firmwareInfoFrame->hide();
    ui->splashFrame->hide();
  }

  // add resize event to queue after changes above
  QTimer::singleShot(0, [=]() { shrink(); });
}

void FlashFirmwareDialog::loadClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Firmware File"),
                                                  g.flashDir(),
                                                  getFirmwareFilesFilter());

  if (!fileName.isEmpty()) {
    if (!firmwareFileExtensions().contains(QFileInfo(fileName).suffix().toLower())) {
      QMessageBox::critical(this, tr("Open Firmware File"),
                            tr("%1 has an unsupported file extension")
                            .arg(fileName));
      return;
    }

    FirmwareInterface newfw(fileName);

    if (chkHWComp->isChecked()) {
      if (!newfw.isValid()) {
        QMessageBox::critical(this, tr("Open Firmware File"),
                              tr("Error - %1 is not a valid firmware file")
                              .arg(fileName));
        return;
      }

      if (connectionMode == CONNECTION_UF2) {
        const Uf2Info uf2(getUf2Info());
        if (newfw.getFlavour() != uf2.board) {
          QMessageBox::critical(this, tr("Open Firmware File"),
                                tr("%1 \nIncompatability - File: '%2' Connected radio: '%3'")
                                .arg(fileName)
                                .arg(newfw.getFlavour())
                                .arg(uf2.board));
          return;
        }
      }
    }

    if (chkProfComp->isChecked() &&
        newfw.getFlavour() != getCurrentFirmware()->getFlavour()) {
      QMessageBox::critical(this, tr("Open Firmware File"),
                            tr("%1 \nIncompatability - File: '%2' Profile: '%3'")
                            .arg(fileName)
                            .arg(newfw.getFlavour())
                            .arg(getCurrentFirmware()->getFlavour()));
      return;
    }

    fwName = fileName;
    updateUI();
  }
}

void FlashFirmwareDialog::useFirmwareSplashClicked()
{
  FirmwareInterface firmware(fwName);

  if (!firmware.isValid()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR, tr( "The firmware file is not valid." ));
  } else if (!firmware.hasSplash()) {
    QMessageBox::warning(this, CPN_STR_TTL_ERROR,
                         tr( "The firmware file does not cntain a start screen image." ));
  } else {
    imageSource = FIRMWARE;
  }

  updateUI();
}

void FlashFirmwareDialog::useProfileSplashClicked()
{
  QString fileName = g.currentProfile().splashFile();

  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR,
                            tr("Profile image %1 is invalid.").arg(fileName));
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

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open image file to use as radio start screen"),
                                                  g.imagesDir(),
                                                  tr("Images (%1)").arg(supportedImageFormats));

  if (!fileName.isEmpty()){
    g.imagesDir( QFileInfo(fileName).dir().absolutePath() );
    QImage image(fileName);

    if (image.isNull()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR,
                            tr("Image could not be loaded from %1")
                            .arg(fileName));
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
  g.currentProfile().fwName(fwName);
  g.backupOnFlash(ui->checkBackup->isChecked());

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

    startWrite(tempFile);
  } else {
    startWrite(fwName);
  }
}

void FlashFirmwareDialog::startWrite(const QString &filename)
{
  close();

  ProgressDialog progressDialog(this, tr("Flash Firmware to Radio"),
                                CompanionIcon("write_flash.png"));

  auto progress = progressDialog.progress();
  bool checkHw = chkHWComp->isChecked();
  bool checkProfile = chkProfComp->isChecked();
  bool backup = g.backupOnFlash();
  FirmwareInterface newfw(filename);

  if (checkHw || checkProfile || backup) {
    progress->updateInfoAndMessages(tr("Reading old firmware..."));
    readFirmware(
        [this, &newfw, progress, checkHw, checkProfile, backup](const QByteArray &_data) {
          FirmwareInterface currfw(_data);

          qDebug() << "profile:" << getCurrentFirmware()->getFlavour()
                    << "current:" << currfw.getFlavour()
                    << "new:" << newfw.getFlavour();

          if (checkHw) {
            if (!currfw.isValid()) {
              QString errMsg(tr("Firmware read from radio invalid"));
              progress->addMessage(errMsg, QtFatalMsg);
              progress->setInfo(errMsg);
              return;
            }

            progress->addMessage(tr("Performing hardware compatibity check"));
            if (!newfw.isHardwareCompatible(currfw)) {
              QString errMsg(tr("New firmware is not compatible with current firmware"));
              progress->updateInfoAndMessages(errMsg, QtFatalMsg);
              return;
            }
          }

          if (checkProfile) {
            progress->addMessage(tr("Performing profile compatibity check"));
            if (currfw.getFlavour() != getCurrentFirmware()->getFlavour()) {
              QString errMsg(tr("Current firmware is not compatible with profile"));
              progress->updateInfoAndMessages(errMsg, QtFatalMsg);
              return;
            } else if (newfw.getFlavour() != getCurrentFirmware()->getFlavour()) {
              QString errMsg(tr("New firmware is not compatible with profile"));
              progress->updateInfoAndMessages(errMsg, QtFatalMsg);
              return;
            }
          }

          if (backup) {
            progress->addMessage(tr("Backing up current firmware"));
            if (!writeFirmwareToFile(this, _data, progress)) {
              return;
            }
          }

          progress->addMessage(tr("Flashing new firmware"));
          writeFirmware(newfw.getFlash(), progress);
        },
        [progress](const QString &err) {
          progress->addMessage(tr("Could not read current firmware: %1").arg(err));
        },
        progress);
  } else {
    progress->setInfo(tr("Flashing new firmware..."));
    writeFirmware(newfw.getFlash(), progress);
  }

  progressDialog.exec();

  if (isTempFileName(filename)) {
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
    connectionMsg = tr("ALERT: No radio detected");
  }

  ui->connectionMode->setText(connectionMsg);

  if (!connectionMode) {
    QMessageBox::warning((atLoad ? (QWidget *)parent(): this), tr("Detect Radio"),
      tr("Radio could not be detected by DFU or UF2 modes") % ".\n" %
      tr("Check cable is securely connected and radio lights are illuminated") % ".\n" %
      tr("Note: USB mode is not suitable for flashing firmware."));
  }

  if (!atLoad)
    updateUI();
}

bool FlashFirmwareDialog::isFileConnectionCompatible()
{
  if (!fwName.isEmpty()) {
    const QString extn(QFileInfo(fwName).suffix().toLower());
    if ((connectionMode == CONNECTION_DFU && dfuFileExtensions().contains(extn)) ||
        (connectionMode == CONNECTION_UF2 && uf2FileExtensions().contains(extn))) {
      return true;
    }
  }

  return false;
}

void FlashFirmwareDialog::shrink()
{
  this->adjustSize();
  this->resize(0, 0);
}
