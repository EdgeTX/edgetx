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
#include <QMap>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>

UpdatesDialog::UpdatesDialog(QWidget * parent, UpdateFactories * factories) :
  QDialog(parent),
  ui(new Ui::UpdatesDialog),
  factories(factories)
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

  ui->leDownloadDir->setText(g.downloadDir());

  connect(ui->chkDecompressDirUseDwnld, &QCheckBox::stateChanged, [=](const int checked) {
    if (!checked) {
      ui->leDecompressDir->setText(g.decompressDir());
      ui->leDecompressDir->setEnabled(true);
      ui->btnDecompressSelect->setEnabled(true);
    }
    else {
      ui->leDecompressDir->setText(g.downloadDir());
      ui->leDecompressDir->setEnabled(false);
      ui->btnDecompressSelect->setEnabled(false);
    }
  });

  ui->chkDecompressDirUseDwnld->setChecked(!g.decompressDirUseDwnld());
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

  int row = 0;
  int col = 0;

  QGridLayout *grid = new QGridLayout();

  QLabel *h1 = new QLabel();
  grid->addWidget(h1, row, col++);

  QLabel *h2 = new QLabel(tr("Name"));
  grid->addWidget(h2, row, col++);

  QLabel *h3 = new QLabel(tr("Channel"));
  grid->addWidget(h3, row, col++);

  QLabel *h4 = new QLabel(tr("Current Release"));
  grid->addWidget(h4, row, col++);

  QLabel *h5 = new QLabel(tr("Update Release"));
  grid->addWidget(h5, row, col++);

  col++;  // options button

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  grid->addItem(spacer, row, col++);

  QMapIterator<QString, int> it(factories->sortedComponentsList(true));

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    const QString name = it.key();

    row++;
    col = 0;

    msg->setText(tr("Retrieving latest release information for %1").arg(name));
    chkUpdate[i] = new QCheckBox();
    chkUpdate[i]->setProperty("index", i);
    chkUpdate[i]->setChecked(!factories->isLatestRelease(name));
    grid->addWidget(chkUpdate[i], row, col++);
    grid->setAlignment(chkUpdate[i], Qt::AlignHCenter);

    QLabel *lblName = new QLabel(name);
    grid->addWidget(lblName, row, col++);

    QLabel *lblRC = new QLabel(ComponentData::releaseChannelsList().at(g.component[i].releaseChannel()));
    grid->addWidget(lblRC, row, col++);

    lblCurrentRel[i] = new QLabel(factories->currentRelease(name));
    grid->addWidget(lblCurrentRel[i], row, col++);

    lblUpdateRel[i] = new QLabel(factories->updateRelease(name));
    grid->addWidget(lblUpdateRel[i], row, col++);

    btnOptions[i] = new QPushButton(tr("Options"));
    connect(btnOptions[i], &QPushButton::clicked, [=]() {
      UpdateOptionsDialog *dlg = new UpdateOptionsDialog(this, factories, i);
      connect(dlg, &UpdateOptionsDialog::changed, [=](const int i) {
        QString name = g.component[i].name();
        chkUpdate[i]->setChecked(!factories->isLatestRelease(name));
        lblCurrentRel[i]->setText(factories->currentRelease(name));
        lblUpdateRel[i]->setText(factories->updateRelease(name));
      });
      dlg->exec();
      dlg->deleteLater();
    });

    grid->addWidget(btnOptions[i], row, col++);
  }

  ui->grpComponents->setLayout(grid);

  ui->chkDelDownloads->setChecked(g.updDelDownloads());

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

  QMapIterator<QString, int> it(factories->sortedComponentsList());

  int cnt = 0;

  while (it.hasNext()) {
    it.next();
    int i = it.value();

    if (g.component[i].checkForUpdate()) {
      if (chkUpdate[i]->isChecked()) {
        cnt++;
        const QString name = it.key();
        UpdateParameters *runParams = factories->getRunParams(name);
        runParams->data.flags |= UpdateInterface::UPDFLG_Update;
        runParams->data.downloadDir = ui->leDownloadDir->text();
        runParams->data.decompressDirUseDwnld = ui->chkDecompressDirUseDwnld->isChecked();
        runParams->data.decompressDir = ui->leDecompressDir->text();
        runParams->data.updateDirUseSD = ui->chkUpdateDirUseSD->isChecked();
        runParams->data.updateDir = ui->leUpdateDir->text();
        if (ui->chkDelDownloads->isChecked())
          runParams->data.flags |= UpdateInterface::UPDFLG_DelDownloads;
        else
          runParams->data.flags &= ~UpdateInterface::UPDFLG_DelDownloads;
      }
    }
  }

  if (cnt < 1) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("No components selected for update!"));
    return;
  }

  QDialog::accept();
}
