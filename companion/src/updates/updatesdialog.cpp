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

#include "updatesdialog.h"
#include "ui_updatesdialog.h"
#include "updateoptionsdialog.h"

#include <QStandardItemModel>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>

UpdatesDialog::UpdatesDialog(QWidget * parent, UpdateFactories * factories) :
  QDialog(parent),
  ui(new Ui::UpdatesDialog),
  factories(factories),
  sortedCompList(factories->sortedComponentsList(true))
{
  //  downloading the release meta data takes a few seconds for each component so display progress
  QDialog *status = new QDialog(parent);
  status->setWindowTitle(tr("Downloading Release Metadata"));
  QVBoxLayout *layout = new QVBoxLayout(status);
  QLabel *msg = new QLabel(status);
  msg->setFixedWidth(400);
  msg->setContentsMargins(50, 50, 50, 50);
  layout->addWidget(msg);
  status->show();

  ui->setupUi(this);

  ui->chkDelDownloads->setChecked(g.updDelDownloads());
  ui->chkDelDecompress->setChecked(g.updDelDecompress());
  ui->leDownloadDir->setText(g.downloadDir());

  connect(ui->chkDecompressDirUseDwnld, &QCheckBox::stateChanged, [=](const int checked) {
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

  //  trigger toggled signal by changing design value and then setting to saved value
  ui->chkDecompressDirUseDwnld->setChecked(!ui->chkDecompressDirUseDwnld->isChecked());
  ui->chkDecompressDirUseDwnld->setChecked(g.decompressDirUseDwnld());

  connect(ui->chkUpdateDirUseSD, &QCheckBox::stateChanged, [=](const int checked) {
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

  if (g.currentProfile().sdPath().trimmed().isEmpty())
    ui->chkUpdateDirUseSD->setEnabled(false);
  else
    ui->chkUpdateDirUseSD->setEnabled(true);

  if (g.updateDirUseSD()) {
    //  trigger toggled signal by changing design value and then setting to saved value
    ui->chkUpdateDirUseSD->setChecked(!ui->chkUpdateDirUseSD->isChecked());
    ui->chkUpdateDirUseSD->setChecked(g.updateDirUseSD());
  }
  else
    ui->chkUpdateDirUseSD->setChecked(false);

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

  QCheckBox *h1 = new QCheckBox();
  grid->addWidget(h1, row, col++);

  QLabel *h2 = new QLabel(tr("Name"));
  grid->addWidget(h2, row, col++);

  QLabel *h3 = new QLabel(tr("Channel"));
  grid->addWidget(h3, row, col++);

  QLabel *h4 = new QLabel(tr("Current"));
  grid->addWidget(h4, row, col++);

  QLabel *h5 = new QLabel(tr("Update to"));
  grid->addWidget(h5, row, col++);

  col++;  // options

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  grid->addItem(spacer, row, col++);

  QMapIterator<QString, int> it(sortedCompList);

  while (it.hasNext()) {
    it.next();
    const int i = it.value();
    const QString name = it.key();

    UpdateInterface *iface = factories->instance(i);

    row++;
    col = 0;

    msg->setText(tr("Retrieving latest release information for %1").arg(name));
    chkUpdate[i] = new QCheckBox();
    chkUpdate[i]->setProperty("index", i);
    grid->addWidget(chkUpdate[i], row, col++);
    grid->setAlignment(chkUpdate[i], Qt::AlignHCenter);

    QLabel *lblName = new QLabel(name);
    grid->addWidget(lblName, row, col++);

    cboRelChannel[i] = new QComboBox();
    cboRelChannel[i]->addItems(ComponentData::releaseChannelsList());
    cboRelChannel[i]->setCurrentIndex(g.component[i].releaseChannel());
    grid->addWidget(cboRelChannel[i], row, col++);

    lblCurrentRel[i] = new QLabel(iface->releaseCurrent());
    grid->addWidget(lblCurrentRel[i], row, col++);

    cboUpdateRel[i] = new QComboBox();
    cboUpdateRel[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboUpdateRel[i]->addItems(iface->releaseList());
    grid->addWidget(cboUpdateRel[i], row, col++);

    connect(cboRelChannel[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (const int index) {
      iface->setReleaseChannel(index);
      cboUpdateRel[i]->clear();
      cboUpdateRel[i]->addItems(iface->releaseList());
      chkUpdate[i]->setChecked(cboUpdateRel[i]->currentIndex() != -1 && !iface->isReleaseLatest());
    });

    connect(cboUpdateRel[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (const int index) {
      chkUpdate[i]->setChecked(cboUpdateRel[i]->currentIndex() != -1 && cboUpdateRel[i]->currentText() != lblCurrentRel[i]->text());
    });

    btnOptions[i] = new QPushButton(tr("Options"));
    connect(btnOptions[i], &QPushButton::clicked, [=]() {
      UpdateOptionsDialog *dlg = new UpdateOptionsDialog(this, iface, i, true);
      connect(dlg, &UpdateOptionsDialog::changed, [=](const int i) {
        lblCurrentRel[i]->setText(iface->releaseCurrent());
        cboUpdateRel[i]->setCurrentText(iface->releaseUpdate());
        chkUpdate[i]->setChecked(cboUpdateRel[i]->currentIndex() != -1 && !iface->isReleaseLatest());
      });
      dlg->exec();
      dlg->deleteLater();
    });

    chkUpdate[i]->setChecked(cboUpdateRel[i]->currentIndex() != -1 && !iface->isReleaseLatest());

    grid->addWidget(btnOptions[i], row, col++);
  }

  ui->grpComponents->setLayout(grid);

  connect(h1, &QCheckBox::clicked, [=] (bool checked) {
    QMapIterator<QString, int> it(sortedCompList);
    while (it.hasNext()) {
      it.next();
      chkUpdate[it.value()]->setChecked(checked);
    }
  });

  QPushButton *btnSaveAsDefaults = new QPushButton(tr("Save as Defaults"));
  ui->buttonBox->addButton(btnSaveAsDefaults, QDialogButtonBox::ActionRole);
  connect(btnSaveAsDefaults, &QPushButton::clicked, [=]() {
    saveAsDefaults();
  });

  connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
    QDialog::reject();
  });

  ui->leDownloadDir->home(false);

  status->hide();
  status->deleteLater();

  QTimer::singleShot(50, this, [=]() {
    adjustSize();
  });
}

UpdatesDialog::~UpdatesDialog()
{
  delete ui;
}

void UpdatesDialog::accept()
{
  if (ui->leDownloadDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Download folder path missing!"));
    return;
  }

  if (ui->leDecompressDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Decompress folder path missing!"));
    return;
  }

  if (ui->leUpdateDir->text().isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Update folder path missing!"));
    return;
  }

  if (!ui->chkDecompressDirUseDwnld->isChecked() &&
      ui->leDecompressDir->text().trimmed() == ui->leDownloadDir->text().trimmed()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Decompress and download folders have the same path!"));
    return;
  }

  int cnt = 0;

  QMapIterator<QString, int> it(sortedCompList);

  while (it.hasNext()) {
    it.next();
    const int i = it.value();

    if (chkUpdate[i]->isChecked()) {
      cnt++;
      UpdateParameters *params = factories->instance(i)->params();
      params->releaseUpdate = cboUpdateRel[i]->currentText();
      params->flags |= UpdateInterface::UPDFLG_Update;
      params->downloadDir = ui->leDownloadDir->text();
      params->decompressDirUseDwnld = ui->chkDecompressDirUseDwnld->isChecked();
      params->decompressDir = ui->leDecompressDir->text();
      params->updateDirUseSD = ui->chkUpdateDirUseSD->isChecked();
      params->updateDir = ui->leUpdateDir->text();
      if (ui->chkDelDownloads->isChecked())
        params->flags |= UpdateInterface::UPDFLG_DelDownloads;
      else
        params->flags &= ~UpdateInterface::UPDFLG_DelDownloads;
      if (ui->chkDelDecompress->isChecked())
        params->flags |= UpdateInterface::UPDFLG_DelDecompress;
      else
        params->flags &= ~UpdateInterface::UPDFLG_DelDecompress;
    }
  }

  if (cnt < 1) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("No components selected for update!"));
    return;
  }

  QDialog::accept();
}

void UpdatesDialog::saveAsDefaults()
{
  g.downloadDir(ui->leDownloadDir->text());
  g.decompressDirUseDwnld(ui->chkDecompressDirUseDwnld->isChecked());
  g.decompressDir(ui->leDecompressDir->text());
  g.updateDirUseSD(ui->chkUpdateDirUseSD->isChecked());
  g.updateDir(ui->leUpdateDir->text());
  g.updDelDownloads(ui->chkDelDownloads->isChecked());
  g.updDelDecompress(ui->chkDelDecompress->isChecked());

  QMapIterator<QString, int> it(sortedCompList);

  while (it.hasNext()) {
    it.next();
    const int i = it.value();
    g.component[i].releaseChannel((ComponentData::ReleaseChannel)cboRelChannel[i]->currentIndex());
    factories->instance(i)->assetSettingsSave();
  }
}
