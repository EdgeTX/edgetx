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
  ui(new Ui::FilteredWriteDialog),
  btnApply(nullptr)
{
  ui->setupUi(this);

  QList<QAbstractButton *> list = ui->buttonBox->buttons();

  // get pointer to Apply button
  for (auto i = list.cbegin(), end = list.cend(); i != end; ++i) {
    if (*i == (QAbstractButton *)ui->buttonBox->button(QDialogButtonBox::Apply))
      btnApply = *i;
  }

  for (unsigned int i = 0; i < radioData.models.size(); i++) {
    QListWidgetItem *item = new QListWidgetItem(radioData.models.at(i).name);
    item->setData(Qt::UserRole, i);
    ui->lstModels->addItem(item);
  }

  connect(ui->chkSettings, &QCheckBox::checkStateChanged, [this] () { setApplyState(); });
  connect(ui->chkCalib, &QCheckBox::checkStateChanged, [this] () { setApplyState(); });
  connect(ui->lstModels, &QListWidget::clicked, [this] () { setApplyState(); });

  connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
    QDialog::reject();
  });

  connect(ui->buttonBox, &QDialogButtonBox::clicked, [&](QAbstractButton *button) {
    if (button == (QAbstractButton *)ui->buttonBox->button(QDialogButtonBox::Apply)) {
      params.settings = ui->chkSettings->isChecked();
      params.calib = ui->chkCalib->isChecked();
      params.replace = ui->rbtnReplace->isChecked();
      params.models.clear();

      auto list = ui->lstModels->selectedItems();
      for (qsizetype i = 0; i < list.size(); ++i) {
        params.models.append(list.at(i)->data(Qt::UserRole).toInt());
      }

      QDialog::accept();
    }
  });

  ui->lstModels->selectAll();
  setApplyState();
}

FilteredWriteDialog::~FilteredWriteDialog()
{
  delete ui;
}

void FilteredWriteDialog::setApplyState()
{
  if (ui->chkSettings->isChecked() ||
      ui->chkCalib->isChecked() ||
      ui->lstModels->selectedItems().size() > 0)
    btnApply->setEnabled(true);
  else
    btnApply->setEnabled(false);
}
