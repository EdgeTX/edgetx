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
#include "appdata.h"
#include "updatefactories.h"
#include "updateoptionsdialog.h"

PrefsUpdatePanel::PrefsUpdatePanel(QWidget * parent, UpdateFactories * factories):
  PrefsPanel(parent),
  ui(new Ui::PrefsUpdate),
  factories(factories)
{
  ui->setupUi(this);
  lock = true;

  ui->cboUpdateCheckFreq->addItems(AppData::updateCheckFreqsList());

  connect(ui->btnResetUpdatesToDefaults, &QPushButton::clicked, [=]() {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Reset all update settings to defaults. Are you sure?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      g.resetUpdatesPrefs();
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

  connect(ui->chkDelDecompress, &QCheckBox::checkStateChanged, [=](const int checked) {
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
    chkCheckForUpdate[i]->setStyleSheet("spacing: 10px"); // workaround Qt 6.9.0 Qt::AlignHCenter causes text to overlap checkbox rhs
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

  lock = false;
}

PrefsUpdatePanel::~PrefsUpdatePanel()
{
  delete ui;
}

bool PrefsUpdatePanel::save()
{
  QStringList msgs;

  if (ui->leDownloadDir->text().isEmpty())
    msgs.append(tr("Download folder path missing!"));

  if (ui->leDecompressDir->text().isEmpty())
    msgs.append(tr("Decompress folder path missing!"));

  if (ui->leUpdateDir->text().isEmpty())
    msgs.append(tr("Update folder path missing!"));

  if (!ui->chkDecompressDirUseDwnld->isChecked() &&
      ui->leDecompressDir->text().trimmed() == ui->leDownloadDir->text().trimmed())
    msgs.append(tr("Decompress and download folders have the same path!"));

  if (msgs.count() > 0) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update Prefs:\n%1").arg(msgs.join("\n|")));
    return false;
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

  return true;
}

void PrefsUpdatePanel::update()
{
  // nothing to do
}
