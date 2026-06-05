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

#include "prefs_update.h"
#include "ui_prefs_update.h"
#include "updates/updatefactories.h"
#include "updates/updateoptionsdialog.h"

#include <QMessageBox>

PrefsUpdatePanel::PrefsUpdatePanel(QWidget * parent, Firmware * fw, Board::Type & bd, Profile & prof, UpdateFactories * factories):
  PrefsPanel(parent, fw, bd, prof),
  ui(new Ui::PrefsUpdate),
  factories(factories)
{
  ui->setupUi(this);
  lock = true;
  // need to track latest value in editor so
  // save before changed in profile preferences ui
  // updated by onSDPathChanged;
  profileSDPath = profile.sdPath().trimmed();

  ui->cboCheckFreq->addItems(AppData::updateCheckFreqsList());
  ui->cboCheckFreq->setPanel(this);
  ui->cboCheckFreq->setBindSave([this] {
    g.updateCheckFreq((AppData::UpdateCheckFreq)this->ui->cboCheckFreq->currentData().toInt());
  });

  ui->btnResetToDefaults->setBindClicked([this] {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Reset ALL update settings to defaults. Are you sure?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      g.resetUpdatesSettings();
      this->setValuesFromSettings();
      emit this->modified();
      QMessageBox::warning(this, CPN_STR_APP_NAME,
        tr("Please save changed preferences.\nThen close and restart Companion to avoid unexpected behaviour!"));
    }
  });

  sectionFolders();
  sectionComponents();
  sectionOptions();
  sectionPostUpdate();

  setValuesFromSettings();
  shrink();
  lock = false;
}

PrefsUpdatePanel::~PrefsUpdatePanel()
{
  delete ui;
}

void PrefsUpdatePanel::onSDPathChanged(QString path)
{
  profileSDPath = path.trimmed();

  if (profileSDPath.isEmpty())
    chkUpdateDirUseSD->setValue(false);
  else if (chkUpdateDirUseSD->isChecked())
    leUpdateDir->setValue(path);

  update();
}

void PrefsUpdatePanel::save()
{
  QStringList msgs;

  if (leDownloadDir->text().trimmed().isEmpty())
    msgs.append(tr("Download folder path missing!"));

  if (leDecompressDir->text().trimmed().isEmpty())
    msgs.append(tr("Decompress folder path missing!"));

  if (leUpdateDir->text().trimmed().isEmpty())
    msgs.append(tr("Update folder path missing!"));

  if (!chkDecompressDirUseDwnld->isChecked() &&
      leDecompressDir->text().trimmed() == leDownloadDir->text().trimmed())
    msgs.append(tr("Decompress and download folders have the same path!"));

  if (msgs.count() > 0) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Preferences:\n%1").arg(msgs.join("\n")));
  }

  AbstractPanel::save();
}

void PrefsUpdatePanel::sectionComponents()
{
  QGridLayout *layComponents = ui->csectComponents->start(tr("Components"));
  row = 0; col = 1;  //  leave col 0 blank

  QLabel *lblCheck = new QLabel(tr("Check"));
  layComponents->addWidget(lblCheck, row, col++);

  QLabel *lblReleaseChannel = new QLabel(tr("Release channel"));
  layComponents->addWidget(lblReleaseChannel, row, col++);

  col++;  // options button

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layComponents->addItem(spacer, row, col++);

  QMapIterator<QString, int> it(factories->sortedComponentsList());

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    row++; col = 0;

    lblName[i] = new AutoLabel();
    layComponents->addWidget(lblName[i], row, col++);

    chkCheckForUpdate[i] = new AutoCheckBox(this);
    // workaround Qt 6.9.0 Qt::AlignHCenter causes text to overlap checkbox rhs
    chkCheckForUpdate[i]->setStyleSheet("spacing: 10px");
    chkCheckForUpdate[i]->setBindSave([this, i] {
      g.component[i].checkForUpdate(this->chkCheckForUpdate[i]->isChecked());
    });
    layComponents->addWidget(chkCheckForUpdate[i], row, col++);
    layComponents->setAlignment(chkCheckForUpdate[i], Qt::AlignHCenter);

    cboReleaseChannel[i] = new AutoComboBox(this);
    cboReleaseChannel[i]->addItems(ComponentData::releaseChannelsList());
    cboReleaseChannel[i]->setBindSave([this, i] {
      g.component[i].releaseChannel((ComponentData::ReleaseChannel)this->cboReleaseChannel[i]->currentData().toInt());
    });
    layComponents->addWidget(cboReleaseChannel[i], row, col++);

    btnComponentOptions[i] = new AutoPushButton(this, tr("Options"));
    connect(btnComponentOptions[i], &QPushButton::clicked, [=]() {
      UpdateOptionsDialog *dlg = new UpdateOptionsDialog(this, factories->instance(i), i, false);
      dlg->exec();
      dlg->deleteLater();
    });
    layComponents->addWidget(btnComponentOptions[i], row, col++);
  }

  ui->csectComponents->finish(row, col, [this] { this->shrink(); });

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    g.component[i].checkForUpdate(chkCheckForUpdate[i]->isChecked());
    g.component[i].releaseChannel((ComponentData::ReleaseChannel)cboReleaseChannel[i]->currentIndex());
  }
}

void PrefsUpdatePanel::sectionFolders()
{
  QGridLayout *layFolders = ui->csectFolders->start(tr("Folders"));
  row = col = 0;

  AutoLabel *lblDownloadDir = new AutoLabel(this, tr("Download"));
  layFolders->addWidget(lblDownloadDir, row, col++);

  leDownloadDir = newPathWidget(this);
  leDownloadDir->setBindSave([this] {
    g.downloadDir(this->leDownloadDir->text().trimmed());
  });
  leDownloadDir->setBindPostChanged([this] {
    if (this->chkDecompressDirUseDwnld->isChecked()) {
      this->leDecompressDir->setValue(this->leDownloadDir->text().trimmed());

      if (this->leDownloadDir->text().trimmed().isEmpty())
        this->chkDecompressDirUseDwnld->setValue(false);
    }

    this->update();
  });
  layFolders->addWidget(leDownloadDir, row, col++);

  AutoDirectorySelectButton *btnDownloadDir = new AutoDirectorySelectButton(this);
  btnDownloadDir->setup(tr("Select download folder"), g.downloadDir(), leDownloadDir);
  btnDownloadDir->addBuddyParentWidget(leDownloadDir);
  layFolders->addWidget(btnDownloadDir, row, col++);

  ++row; col = 0;
  AutoLabel *lblDecompressDir = new AutoLabel(this, tr("Decompress"));
  layFolders->addWidget(lblDecompressDir, row, col++);

  chkDecompressDirUseDwnld = new AutoCheckBox(this, tr("create sub-folders in Download folder"));
  chkDecompressDirUseDwnld->setBindSave([this] {
    g.decompressDirUseDwnld(this->chkDecompressDirUseDwnld->isChecked());
  });
  chkDecompressDirUseDwnld->setBindEnabled([this] {
    return !this->leDownloadDir->text().trimmed().isEmpty();
  });
  chkDecompressDirUseDwnld->setBindPostChanged([this] {
    if (chkDecompressDirUseDwnld->isChecked())
      this->leDecompressDir->setValue(this->leDownloadDir->text());
    this->update();
  });
  layFolders->addWidget(chkDecompressDirUseDwnld, row, col++);

  ++row; col = 1; // skip col 0
  leDecompressDir = newPathWidget(this);
  leDecompressDir->setBindEnabled([this] {
    return (!this->chkDecompressDirUseDwnld->isChecked() ||
            this->leDownloadDir->text().trimmed().isEmpty());
  });
  leDecompressDir->setBindSave([this] {
    g.decompressDir(this->leDecompressDir->text().trimmed());
  });
  layFolders->addWidget(leDecompressDir, row, col++);

  AutoDirectorySelectButton *btnDecompressDir = new AutoDirectorySelectButton(this);
  btnDecompressDir->setup(tr("Select decompression folder"), g.decompressDir(), leDecompressDir);;
  btnDecompressDir->addBuddyParentWidget(leDecompressDir);
  layFolders->addWidget(btnDecompressDir, row, col++);

  ++row; col = 0;
  AutoLabel *lblUpdateDir = new AutoLabel(this, tr("Update"));
  layFolders->addWidget(lblUpdateDir, row, col++);

  chkUpdateDirUseSD = new AutoCheckBox(this, tr("use Radio Profile SD path"));
  chkUpdateDirUseSD->setBindEnabled([this] {
    return !this->profileSDPath.isEmpty();
  });
  chkUpdateDirUseSD->setBindSave([this] {
    g.updateDirUseSD(this->chkUpdateDirUseSD->isChecked());
  });
  chkUpdateDirUseSD->setBindPostChanged([this] {
    if (this->chkUpdateDirUseSD->isChecked()) {
      this->leUpdateDir->setValue(profileSDPath);
    }
    this->update();
  });
  layFolders->addWidget(chkUpdateDirUseSD, row, col++);

  ++row; col = 1; // skip col 0
  leUpdateDir = newPathWidget(this);
  leUpdateDir->setBindEnabled([this] {
    return !this->chkUpdateDirUseSD->isChecked() ||
            this->profileSDPath.isEmpty();
  });
  leUpdateDir->setBindSave([this] {
    g.updateDir(this->leUpdateDir->text().trimmed());
  });
  layFolders->addWidget(leUpdateDir, row, col++);

  AutoDirectorySelectButton *btnUpdateDir = new AutoDirectorySelectButton(this);
  btnUpdateDir->setup(tr("Select update folder"), g.updateDir(), leUpdateDir);;
  btnUpdateDir->addBuddyParentWidget(leUpdateDir);
  layFolders->addWidget(btnUpdateDir, row, col++);

  ui->csectFolders->finish(-1, -1, [this] { this->shrink(); });
}

void PrefsUpdatePanel::sectionOptions()
{
  QGridLayout *layOptions = ui->csectOptions->start(tr("Options"));
  row = col = 0;

  AutoLabel *lblDelDownloads = new AutoLabel(this, tr("Delete downloads"));
  layOptions->addWidget(lblDelDownloads, row, col++);

  chkDelDownloads = new AutoCheckBox(this);
  chkDelDownloads->setBindSave([this] {
    g.updDelDownloads(this->chkDelDownloads->isChecked());
  });
  chkDelDownloads->setBindEnabled([this] {
    return !this->chkDecompressDirUseDwnld->isChecked() ||
            this->chkDelDecompress->isChecked();
  });
  layOptions->addWidget(chkDelDownloads, row, col++);

  row++; col = 0;
  AutoLabel *lblDelDecompress = new AutoLabel(this, tr("Delete decompressions"));
  layOptions->addWidget(lblDelDecompress, row, col++);

  chkDelDecompress = new AutoCheckBox(this);
  chkDelDecompress->setBindSave([this] {
    g.updDelDecompress(this->chkDelDecompress->isChecked());
  });
  chkDelDecompress->setBindPostChanged([this] {
    if (!this->chkDelDecompress->isChecked()) {
      if (this->chkDecompressDirUseDwnld->isChecked()) {
        this->chkDelDownloads->setValue(false);
      }
    } else {
      this->chkDelDownloads->setEnabled(true);
    }

    update();
  });
  layOptions->addWidget(chkDelDecompress, row, col++);

  row++; col = 0;
  QLabel *lblLogLevel = new QLabel(tr("Log level"), this);
  layOptions->addWidget(lblLogLevel, row, col++);

  cboLogLevel = new AutoComboBox(this);
  cboLogLevel->addItems(AppData::updateLogLevelsList());
  cboLogLevel->setBindSave([this] {
    g.updLogLevel(this->cboLogLevel->currentData().toInt());
  });
  layOptions->addWidget(cboLogLevel, row, col++);

  ui->csectOptions->finish(row, col, [this] { this->shrink(); });
}

void PrefsUpdatePanel::sectionPostUpdate()
{
  QGridLayout *layPostUpdate = ui->csectPostUpdate->start(tr("Post Update"));
  row = col = 0;

  chkPrmptFlash = new AutoCheckBox(this, tr("Prompt to flash firmware"));
  chkPrmptFlash->setBindSave([this] {
    profile.burnFirmware(this->chkPrmptFlash->isChecked());
  });
  layPostUpdate->addWidget(chkPrmptFlash, row, col++);

  row++; col = 0;
  chkPrmptSDSync = new AutoCheckBox(this, tr("Prompt to run SD Sync"));
  chkPrmptSDSync->setBindSave([this] {
    profile.runSDSync(this->chkPrmptSDSync->isChecked());
  });
  layPostUpdate->addWidget(chkPrmptSDSync, row, col++);

  row++; col = 0;
  chkPrmptCpnInstall = new AutoCheckBox(this, tr("Prompt to run Companion installer"));
  chkPrmptCpnInstall->setBindSave([this] {
    g.runAppInstaller(this->chkPrmptCpnInstall->isChecked());
  });
  layPostUpdate->addWidget(chkPrmptCpnInstall, row, col++);

  ui->csectPostUpdate->finish(row, col, [this] { this->shrink(); });
}

void PrefsUpdatePanel::setValuesFromSettings()
{
  lock = true;
  ui->cboCheckFreq->setValue(g.updateCheckFreq());

  // folders
  leDownloadDir->setValue(g.downloadDir());
  chkDecompressDirUseDwnld->setValue(g.decompressDirUseDwnld());
  leDecompressDir->setValue(g.decompressDir());
  chkUpdateDirUseSD->setValue(g.updateDirUseSD());
  leUpdateDir->setValue(g.updateDir());

  //  components
  QMapIterator<QString, int> it(factories->sortedComponentsList());

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    lblName[i]->setText(it.key());
    chkCheckForUpdate[i]->setValue(g.component[i].checkForUpdate());
    cboReleaseChannel[i]->setValue(g.component[i].releaseChannel());
  }

  // options
  chkDelDownloads->setValue(g.updDelDownloads());
  chkDelDecompress->setValue(g.updDelDecompress());
  cboLogLevel->setValue(g.updLogLevel());

  // post update
  chkPrmptFlash->setValue(profile.burnFirmware());
  chkPrmptSDSync->setValue(profile.runSDSync());
  chkPrmptCpnInstall->setValue(g.runAppInstaller());

  update();
  lock = false;
}

void PrefsUpdatePanel::update()
{
  AbstractPanel::update();
}
