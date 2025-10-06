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

#include "filteredwritedialog.h"
#include "ui_filteredwritedialog.h"
#include "radiodata.h"

FilteredWriteDialog::FilteredWriteDialog(QWidget * parent, RadioData & radioData,
                                         Params & params) :
  QDialog(parent),
  ui(new Ui::FilteredWriteDialog)
{
  ui->setupUi(this);

  for (unsigned int i = 0; i < radioData.models.size(); i++) {
    QListWidgetItem *item = new QListWidgetItem(radioData.models.at(i).name);
    item->setData(Qt::UserRole, i);
    ui->lstModels->addItem(item);
  }

  ui->chkRadio->setChecked(true);
  ui->chkCalibration->setChecked(true);
  ui->chkAllModels->setChecked(true);
  ui->lstModels->setSelectionMode(QAbstractItemView::NoSelection);

  connect(ui->chkRadio, &QCheckBox::checkStateChanged, [=](bool checked) {
    ui->chkCalibration->setChecked(checked);
    ui->chkCalibration->setEnabled(checked);
  });

  connect(ui->chkAllModels, &QCheckBox::checkStateChanged, [=](bool checked) {
    if (checked) {
      ui->lstModels->clearSelection();
      ui->lstModels->setSelectionMode(QAbstractItemView::NoSelection);
    } else {
      ui->lstModels->setSelectionMode(QAbstractItemView::MultiSelection);
    }
  });

  connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
    QDialog::reject();
  });

  connect(ui->buttonBox, &QDialogButtonBox::clicked, [&](QAbstractButton *button) {
    if (button == (QAbstractButton *)ui->buttonBox->button(QDialogButtonBox::Apply)) {
      params.calib = ui->chkCalibration->isChecked();
      params.genSettings = ui->chkRadio->isChecked();
      params.allModels = ui->chkAllModels->isChecked();
      params.modelList.clear();

      if (!ui->chkAllModels->isChecked()) {
        auto list = ui->lstModels->selectedItems();
        for (qsizetype i = 0; i < list.size(); ++i) {
          params.modelList.append(list.at(i)->data(Qt::UserRole).toInt());
        }
      }

      QDialog::accept();
    }
  });
}

FilteredWriteDialog::~FilteredWriteDialog()
{
  delete ui;
}
