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

#include "profilechooser.h"
#include "ui_profilechooser.h"
#include "appdata.h"

ProfileChooserDialog::ProfileChooserDialog(QWidget * parent):
  QDialog(parent),
  ui(new Ui::ProfileChooserDialog)
{
  ui->setupUi(this);
  setWindowIcon(QIcon(":/icon.png"));
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  QComboBox *prof = ui->cboProfiles;
  prof->clear();

  QMap<int, QString> active;
  active = g.getActiveProfiles();
  QMapIterator<int, QString> i(active);
  while (i.hasNext()) {
      i.next();
      prof->addItem(i.value(), i.key());
  }

  prof->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  prof->setCurrentIndex(prof->findData(g.id()));
  connect(prof, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cboProfilesCurrentIndexChanged(int)));
}

ProfileChooserDialog::~ProfileChooserDialog()
{
  delete ui;
}

void ProfileChooserDialog::on_cboProfilesCurrentIndexChanged(int index)
{
  QComboBox *prof = ui->cboProfiles;

  if ((prof->currentIndex() > -1)) {
    int oldid = g.id();
    int newid = prof->currentData().toInt();
    if (oldid != newid)
      emit profileChanged(newid);
  }
}
