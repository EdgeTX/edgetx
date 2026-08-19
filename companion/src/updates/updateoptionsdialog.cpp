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

#include "updateoptionsdialog.h"
#include "ui_updateoptionsdialog.h"
#include "appdata.h"

#include <QAbstractButton>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

UpdateOptionsDialog::UpdateOptionsDialog(QWidget * parent, UpdateInterface * iface, const int idx, const bool isRun) :
  QDialog(parent),
  ui(new Ui::UpdateOptionsDialog),
  iface(iface),
  params(iface->params()),
  idx(idx),
  isRun(isRun)
{
  ui->setupUi(this);

  if (!isRun)
    iface->resetEnvironment();

  setWindowTitle(tr("%1 %2").arg(iface->name()).arg(tr("Options")));

  ui->txtreleaseCurrent->setText(iface->releaseCurrent());

  connect(ui->btnReleaseClear, &QPushButton::clicked, [=]() {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear current release information. Are you sure?"),
                             QMessageBox::Yes |QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      iface->releaseClear();
      ui->txtreleaseCurrent->setText(iface->releaseCurrent());
      emit changed(idx);

    }
  });

  QVBoxLayout *grpAssetsLayout = new QVBoxLayout();

  for (int i = 0; i < params->assets.size(); i++) {
    const UpdateParameters::AssetParams & ap = params->assets.at(i);

    int processes = ap.processes;
    int flags = ap.flags;
    const bool locked = flags & UpdateInterface::UPDFLG_Locked;

    QHBoxLayout *layout1 = new QHBoxLayout();

    if (i > 0) {
      QFrame *line = new QFrame(this);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setLineWidth(1);
      line->setMidLineWidth(0);
      grpAssetsLayout->addWidget(line);
    }

    QLabel *aflbl = new QLabel(tr("Filter"));
    layout1->addWidget(aflbl);

    QComboBox *cboAssetFilterType = new QComboBox();
    cboAssetFilterType->setEnabled(!locked);
    cboAssetFilterType->addItems(UpdateParameters::updateFilterTypeList());
    cboAssetFilterTypes << cboAssetFilterType;
    layout1->addWidget(cboAssetFilterType);

    QLineEdit *leAssetFilter = new QLineEdit();
    leAssetFilter->setEnabled(!locked);
    leAssetFilters << leAssetFilter;
    layout1->addWidget(leAssetFilter);

    connect(leAssetFilter, &QLineEdit::editingFinished, [=]() {
      QRegularExpression re(leAssetFilter->text());
      leAssetFilter->setStyleSheet((re.isValid() ? "QLineEdit { color : black; }" : "QLineEdit { color : red; }"));
    });

    connect(cboAssetFilterType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](const int index) {
      if (index == UpdateParameters::UFT_None) {
        leAssetFilter->setEnabled(false);
        leAssetFilter->setText("");
      }
      else
        leAssetFilter->setEnabled(!locked);
    });

    QSpacerItem *hsp1 = new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Minimum );
    layout1->addItem(hsp1);

    grpAssetsLayout->addLayout(layout1);

    QHBoxLayout *layout2 = new QHBoxLayout();

    layout2->addWidget(new QLabel(tr("Max.Expected")));

    QSpinBox *sbMaxExpected = new QSpinBox();
    sbMaxExpected->setEnabled(!locked);
    sbMaxExpected->setSpecialValueText(tr("No Limit"));
    sbMaxExpects << sbMaxExpected;
    layout2->addWidget(sbMaxExpected);

    QCheckBox *chkDownload = new QCheckBox(tr("Download"));
    chkDownloads << chkDownload;
    layout2->addWidget(chkDownload);

    QCheckBox *chkDecompress = new QCheckBox(tr("Decompress"));
    chkDecompress->setEnabled(processes & UpdateInterface::UPDFLG_Decompress);
    chkDecompresses << chkDecompress;
    layout2->addWidget(chkDecompress);

    QCheckBox *chkInstall = new QCheckBox(tr("Install"));
    chkInstall->setEnabled(processes & UpdateInterface::UPDFLG_AsyncInstall);
    chkInstalls << chkInstall;
    layout2->addWidget(chkInstall);

    QSpacerItem *hsp2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem(hsp2);

    grpAssetsLayout->addLayout(layout2);

    QGridLayout *layout3 = new QGridLayout();

    QCheckBox *chkCopy = new QCheckBox(tr("Copy"));
    chkCopy->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest);
    chkCopies << chkCopy;
    layout3->addWidget(chkCopy, 0, 0);

    QLabel *lblCopyFilter = new QLabel(tr("Filter"));
    lblCopyFilter->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest);
    lblCopyFilters << lblCopyFilter;
    layout3->addWidget(lblCopyFilter, 0, 1);

    QComboBox *cboCopyFilterType = new QComboBox();
    cboCopyFilterType->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest && (!locked));
    cboCopyFilterType->addItems(UpdateParameters::updateFilterTypeList());
    cboCopyFilterTypes << cboCopyFilterType;
    layout3->addWidget(cboCopyFilterType, 0, 2);

    QLineEdit *leCopyFilter = new QLineEdit();
    leCopyFilter->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest && (!locked));
    leCopyFilters << leCopyFilter;
    layout3->addWidget(leCopyFilter, 0, 3);

    connect(leCopyFilter, &QLineEdit::editingFinished, [=]() {
      QRegularExpression re(leCopyFilter->text());
      leCopyFilter->setStyleSheet((re.isValid() ? "QLineEdit { color : black; }" : "QLineEdit { color : red; }"));
    });

    connect(cboCopyFilterType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](const int index) {
      if (index == UpdateParameters::UFT_None) {
        leCopyFilter->setEnabled(false);
        leCopyFilter->setText("");
      }
      else
        leCopyFilter->setEnabled(!locked);
    });

    QLabel *lblSubFolder = new QLabel(tr("Sub folder"));
    lblSubFolder->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest);
    lblSubFolders << lblSubFolder;
    layout3->addWidget(lblSubFolder, 1, 1);

    QLineEdit *leSubFolder = new QLineEdit();
    leSubFolder->setEnabled(processes & UpdateInterface::UPDFLG_CopyDest && (!locked));
    leSubFolders << leSubFolder;
    layout3->addWidget(leSubFolder, 1, 2, 1, 2);

    connect(chkDownload, &QCheckBox::stateChanged, [=](const int checked) {
      if (!checked) {
        chkDecompress->setChecked(false);
        chkCopy->setChecked(false);
        chkInstall->setChecked(false);
      }
      else {
        chkDecompress->setChecked(processes & UpdateInterface::UPDFLG_Decompress);
        chkCopy->setChecked(processes & UpdateInterface::UPDFLG_CopyDest);
        chkInstall->setChecked(processes & UpdateInterface::UPDFLG_AsyncInstall);
      }
    });

    connect(chkDecompress, &QCheckBox::stateChanged, [=](const int checked) {
      if (!checked) {
        chkCopy->setChecked(false);
        chkInstall->setChecked(false);
      }
      else {
        chkCopy->setChecked(processes & UpdateInterface::UPDFLG_CopyDest);
        chkInstall->setChecked(processes & UpdateInterface::UPDFLG_AsyncInstall);
      }
    });

    connect(chkCopy, &QCheckBox::stateChanged, [=](const int checked) {
      cboCopyFilterType->setEnabled(checked ? (processes & UpdateInterface::UPDFLG_CopyDest) && (!locked) : checked);
      leCopyFilter->setEnabled(checked ? (processes & UpdateInterface::UPDFLG_CopyDest) && (!locked) : checked);
      leSubFolder->setEnabled(checked ? (processes & UpdateInterface::UPDFLG_CopyDest) && (!locked) : checked);
      if (!checked) {
        chkInstall->setChecked(false);
      }
      else {
        chkInstall->setChecked(processes & UpdateInterface::UPDFLG_AsyncInstall);
      }
    });

    grpAssetsLayout->addLayout(layout3);
  }

  ui->grpAssets->setLayout(grpAssetsLayout);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
    for (int i = 0; i < params->assets.size(); i++) {
      UpdateParameters::AssetParams & ap = params->assets[i];

      int flags = ap.flags;

      if (!(flags & UpdateInterface::UPDFLG_Locked)) {
        ap.filterType = (UpdateParameters::UpdateFilterType)cboAssetFilterTypes.at(i)->currentIndex();
        ap.filter = leAssetFilters.at(i)->text();
        ap.maxExpected = sbMaxExpects.at(i)->value();
        ap.copyFilterType = (UpdateParameters::UpdateFilterType)cboCopyFilterTypes.at(i)->currentIndex();
        ap.copyFilter = leCopyFilters.at(i)->text();
        ap.destSubDir = leSubFolders.at(i)->text();
      }

      chkDownloads.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_Download : flags &= ~UpdateInterface::UPDFLG_Download;
      chkDecompresses.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_Decompress : flags &= ~UpdateInterface::UPDFLG_Decompress;
      chkInstalls.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_AsyncInstall : flags &= ~UpdateInterface::UPDFLG_AsyncInstall;
      chkCopies.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_CopyDest : flags &= ~UpdateInterface::UPDFLG_CopyDest;
      ap.flags = flags;
    }

    if (!isRun)
      iface->assetSettingsSave();

    QDialog::accept();
  });

  connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
    QDialog::reject();
  });

  update();

  QTimer::singleShot(50, this, [=]() {
    adjustSize();
  });
}

UpdateOptionsDialog::~UpdateOptionsDialog()
{
  delete ui;
}

void UpdateOptionsDialog::update()
{
  for (int i = 0; i < params->assets.size(); i++) {
    const UpdateParameters::AssetParams & ap = params->assets.at(i);
    cboAssetFilterTypes.at(i)->setCurrentIndex(ap.filterType);
    leAssetFilters.at(i)->setText(ap.filter);

    sbMaxExpects.at(i)->setValue(ap.maxExpected);

    chkDownloads.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_Download);

    chkDecompresses.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_Decompress);

    chkCopies.at(i)->setChecked(!chkCopies.at(i)->isChecked());
    chkCopies.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_CopyDest);
    cboCopyFilterTypes.at(i)->setCurrentIndex(-1);
    cboCopyFilterTypes.at(i)->setCurrentIndex(ap.copyFilterType);
    leCopyFilters.at(i)->setText(ap.copyFilter);
    leSubFolders.at(i)->setText(ap.destSubDir);

    chkInstalls.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_AsyncInstall);
  }
}
