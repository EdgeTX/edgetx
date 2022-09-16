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
  name(g.component[idx].name())
{
  ui->setupUi(this);

  setWindowTitle(tr("%1 %2").arg(name).arg(tr("Options")));

  ui->txtCurrentRelease->setText(factories->currentRelease(name));

  connect(ui->btnClearRelease, &QPushButton::clicked, [=]() {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear current release information. Are you sure?"),
                             QMessageBox::Yes |QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
      factories->clearRelease(name);
      ui->txtCurrentRelease->setText(factories->currentRelease(name));
    }
  });

  QVBoxLayout *grpAssetsLayout = new QVBoxLayout();

  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[idx].asset[i].existsOnDisk(); i++) {
    int processes = g.component[idx].asset[i].processes();
    int flags = g.component[idx].asset[i].flags();
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
    for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[idx].asset[i].existsOnDisk(); i++) {
      ComponentAssetData &cad = g.component[idx].asset[i];

      int flags = cad.flags();

      if (!(flags & UpdateInterface::UPDFLG_Locked)) {
        cad.filterType(cboAssetFilterTypes.at(i)->currentIndex());
        cad.filter(leAssetFilters.at(i)->text());
        cad.maxExpected(sbMaxExpects.at(i)->value());
        cad.copyFilterType(cboCopyFilterTypes.at(i)->currentIndex());
        cad.copyFilter(leCopyFilters.at(i)->text());
        cad.destSubDir(leSubFolders.at(i)->text());
      }

      chkDownloads.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_Download : flags &= ~UpdateInterface::UPDFLG_Download;
      chkDecompresses.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_Decompress : flags &= ~UpdateInterface::UPDFLG_Decompress;
      chkInstalls.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_AsyncInstall : flags &= ~UpdateInterface::UPDFLG_AsyncInstall;
      chkCopies.at(i)->isChecked() ? flags |= UpdateInterface::UPDFLG_CopyDest : flags &= ~UpdateInterface::UPDFLG_CopyDest;
      cad.flags(flags);
    }
    QDialog::accept();
  });

  connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
    QDialog::reject();
  });

  connect(ui->buttonBox, &QDialogButtonBox::clicked, [=](QAbstractButton * button) {
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults) {
      factories->initAssetSettings(name);
      update();
    }
    //else if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
    //}
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
  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[idx].asset[i].existsOnDisk(); i++) {
    const ComponentAssetData &cad = g.component[idx].asset[i];
    cboAssetFilterTypes.at(i)->setCurrentIndex(cad.filterType());
    leAssetFilters.at(i)->setText(cad.filter());

    sbMaxExpects.at(i)->setValue(cad.maxExpected());

    chkDownloads.at(i)->setChecked(cad.flags() & UpdateInterface::UPDFLG_Download);

    chkDecompresses.at(i)->setChecked(cad.flags() & UpdateInterface::UPDFLG_Decompress);

    chkCopies.at(i)->setChecked(!chkCopies.at(i)->isChecked());
    chkCopies.at(i)->setChecked(cad.flags() & UpdateInterface::UPDFLG_CopyDest);
    cboCopyFilterTypes.at(i)->setCurrentIndex(-1);
    cboCopyFilterTypes.at(i)->setCurrentIndex(cad.copyFilterType());
    leCopyFilters.at(i)->setText(cad.copyFilter());
    leSubFolders.at(i)->setText(cad.destSubDir());

    chkInstalls.at(i)->setChecked(cad.flags() & UpdateInterface::UPDFLG_AsyncInstall);
  }
}
