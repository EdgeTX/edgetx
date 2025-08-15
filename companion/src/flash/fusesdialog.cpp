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

#include "fusesdialog.h"
#include "ui_fusesdialog.h"
#include "helpers.h"
#include "progressdialog.h"
#include "radiointerface.h"

FusesDialog::FusesDialog(QWidget *parent) :
QDialog(parent),
ui(new Ui::FusesDialog)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("fuses.png"));
}

FusesDialog::~FusesDialog()
{
  delete ui;
}

void FusesDialog::on_resetFuses_EEprotect_clicked()
{
  ProgressDialog progressDialog(this, tr("Reset Radio Fuses"), CompanionIcon("fuses.png"), true);
  return resetAvrdudeFuses(true, progressDialog.progress());
}

void FusesDialog::on_resetFuses_EEdelete_clicked()
{
  ProgressDialog progressDialog(this, tr("Reset Radio Fuses"), CompanionIcon("fuses.png"), true);
  return resetAvrdudeFuses(false, progressDialog.progress());
}

void FusesDialog::on_readFuses_clicked()
{
  ProgressDialog progressDialog(this, tr("Read Fuses from Radio"), CompanionIcon("fuses.png"), true);
  return readAvrdudeFuses(progressDialog.progress());
}
