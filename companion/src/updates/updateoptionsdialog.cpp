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

UpdateOptionsDialog::UpdateOptionsDialog(QWidget * parent, UpdateFactories * factories, const int idx) :
  QDialog(parent),
  ui(new Ui::UpdateOptionsDialog),
  factories(factories),
  idx(idx),
  name(g.component[idx].name()),
  dfltParams(factories->getDefaultParams(name)),
  runParams(factories->getRunParams(name))
{
  ui->setupUi(this);

  setWindowTitle(tr("%1 %2").arg(name).arg(tr("Options")));

  ui->leCurrent->setText(dfltParams->data.currentRelease);

  connect(ui->btnForget, &QPushButton::clicked, [=]() {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear current release information.\nWarning: There is no undo! Are you sure?"),
                             QMessageBox::Yes |QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      factories->clearRelease(name);
      ui->leCurrent->setText(dfltParams->data.currentRelease);
      emit changed(idx);
    }
  });

  ui->cboReleases->addItems(factories->releases(name));

  int i = -1;

  QVBoxLayout *grpAssetsLayout = new QVBoxLayout();

  foreach (const UpdateParameters::AssetParams &ap, dfltParams->data.assets) {
    int flags = ap.flags;
    const bool locked = flags & UpdateInterface::UPDFLG_Locked;
    i++;

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

    QSpacerItem *hsp1 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem(hsp1);

    grpAssetsLayout->addLayout(layout1);

    QHBoxLayout *layout2 = new QHBoxLayout();

    QCheckBox *chkDownload = new QCheckBox(tr("Download"));
    chkDownloads << chkDownload;
    layout2->addWidget(chkDownload);

    QCheckBox *chkDecompress = new QCheckBox(tr("Decompress"));
    chkDecompress->setEnabled(flags & UpdateInterface::UPDFLG_Decompress);
    chkDecompresses << chkDecompress;
    layout2->addWidget(chkDecompress);

    QCheckBox *chkInstall = new QCheckBox(tr("Install"));
    chkInstall->setEnabled(flags & UpdateInterface::UPDFLG_AsyncInstall);
    chkInstalls << chkInstall;
    layout2->addWidget(chkInstall);

    QSpacerItem *hsp2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem(hsp2);

    grpAssetsLayout->addLayout(layout2);

    QGridLayout *layout3 = new QGridLayout();

    QCheckBox *chkCopy = new QCheckBox(tr("Copy"));
    chkCopy->setEnabled(flags & UpdateInterface::UPDFLG_CopyDest);
    chkCopies << chkCopy;
    layout3->addWidget(chkCopy, 0, 0);

    QLabel *lblCopyFilter = new QLabel(tr("Filter"));
    lblCopyFilter->setEnabled(flags & UpdateInterface::UPDFLG_CopyDest);
    lblCopyFilters << lblCopyFilter;
    layout3->addWidget(lblCopyFilter, 0, 1);

    QComboBox *cboCopyFilterType = new QComboBox();
    cboCopyFilterType->setEnabled(!locked);
    cboCopyFilterType->addItems(UpdateParameters::updateFilterTypeList());
    cboCopyFilterTypes << cboCopyFilterType;
    layout3->addWidget(cboCopyFilterType, 0, 2);

    QLineEdit *leCopyFilter = new QLineEdit();
    leCopyFilter->setEnabled(flags & UpdateInterface::UPDFLG_CopyDest && (!locked));
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
    lblSubFolder->setEnabled(flags & UpdateInterface::UPDFLG_CopyDest);
    lblSubFolders << lblSubFolder;
    layout3->addWidget(lblSubFolder, 1, 1);

    QLineEdit *leSubFolder = new QLineEdit();
    leSubFolder->setEnabled(flags & UpdateInterface::UPDFLG_CopyDest && (!locked));
    leSubFolders << leSubFolder;
    layout3->addWidget(leSubFolder, 1, 2, 1, 2);

    grpAssetsLayout->addLayout(layout3);
  }

  ui->grpAssets->setLayout(grpAssetsLayout);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
    QDialog::accept();
  });

  connect(ui->buttonBox, &QDialogButtonBox::clicked, [=](QAbstractButton * button) {
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults) {
      factories->resetRunEnvironment(name);
      update();
    }
    else if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Discard) {
      QDialog::reject();
    }
    else if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Save) {
      runParams->data.updateRelease = ui->cboReleases->currentText();

      for (int i = 0; i < runParams->data.assets.size(); i++) {
        UpdateParameters::AssetParams &ap = runParams->data.assets[i];

        if (!(ap.flags & UpdateInterface::UPDFLG_Locked)) {
          ap.filterType = (UpdateParameters::UpdateFilterType)cboAssetFilterTypes.at(i)->currentIndex();
          ap.filter = leAssetFilters.at(i)->text();
          ap.copyFilterType = (UpdateParameters::UpdateFilterType)cboCopyFilterTypes.at(i)->currentIndex();
          ap.copyFilter = leCopyFilters.at(i)->text();
          ap.destSubDir = leSubFolders.at(i)->text();
        }

        chkDownloads.at(i)->isChecked() ? ap.flags |= UpdateInterface::UPDFLG_Download : ap.flags &= ~UpdateInterface::UPDFLG_Download;
        chkDecompresses.at(i)->isChecked() ? ap.flags |= UpdateInterface::UPDFLG_Decompress :ap.flags &= ~UpdateInterface::UPDFLG_Decompress;
        chkInstalls.at(i)->isChecked() ? ap.flags |= UpdateInterface::UPDFLG_AsyncInstall : ap.flags &= ~UpdateInterface::UPDFLG_AsyncInstall;
        chkCopies.at(i)->isChecked() ? ap.flags |= UpdateInterface::UPDFLG_CopyDest : ap.flags &= ~UpdateInterface::UPDFLG_CopyDest;
      }

      emit changed(idx);
    }
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
  ui->cboReleases->setCurrentText(runParams->data.updateRelease);

  int i = 0;

  foreach (const UpdateParameters::AssetParams &ap, runParams->data.assets) {
    cboAssetFilterTypes.at(i)->setCurrentIndex((int)ap.filterType);
    leAssetFilters.at(i)->setText(ap.filter);

    chkDownloads.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_Download);
    chkDecompresses.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_Decompress);
    chkInstalls.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_AsyncInstall);

    chkCopies.at(i)->setChecked(ap.flags & UpdateInterface::UPDFLG_CopyDest);
    cboCopyFilterTypes.at(i)->setCurrentIndex((int)ap.copyFilterType);
    leCopyFilters.at(i)->setText(ap.copyFilter);
    leSubFolders.at(i)->setText(ap.destSubDir);

    i++;
  }
}
