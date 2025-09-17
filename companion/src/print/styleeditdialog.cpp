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

#include "styleeditdialog.h"
#include "ui_styleeditdialog.h"
#include "helpers.h"

StyleEditDialog::StyleEditDialog(QWidget * parent, const QString stylesheet):
  QDialog(parent),
  ui(new Ui::StyleEditDialog),
  mStylesheet(stylesheet),
  mDirty(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("edit.png"));
  if (!mStylesheet.load(Stylesheet::StyleType::STYLE_TYPE_EFFECTIVE))
    QMessageBox::critical(this, tr("Style Sheet Editor"), tr("Cannot retrieve style %1\nError: %2").arg(mStylesheet.name(), mStylesheet.errormsg()));
  QFont f;
  f.setFamily("monospace");
  f.setPointSize(10);
  ui->pteStyle->setFont(f);
  ui->pteStyle->setPlainText(mStylesheet.text());
  connect(ui->pteStyle, SIGNAL(textChanged()), this, SLOT(changed()));
  connect(ui->pbDefault, SIGNAL(clicked()), this, SLOT(resetToDefault()));
  connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->pbOK, SIGNAL(clicked()), this, SLOT(update()));
}

StyleEditDialog::~StyleEditDialog()
{
  delete ui;
}

void StyleEditDialog::changed()
{
  mDirty = true;
}

void StyleEditDialog::resetToDefault()
{
  mStylesheet.deleteCustom();   //  ignore result
  if (!mStylesheet.load(Stylesheet::StyleType::STYLE_TYPE_DEFAULT))
    QMessageBox::critical(this, tr("Style Sheet Editor"), tr("Cannot retrieve default style %1\nError: %2").arg(mStylesheet.name(), mStylesheet.errormsg()));
  ui->pteStyle->setPlainText(mStylesheet.text());
  mDirty = false;
}

void StyleEditDialog::update()
{
  if (mDirty) {
    mStylesheet.setText(ui->pteStyle->toPlainText());
    if (!mStylesheet.update())
      QMessageBox::critical(this, tr("Style Sheet Editor"), tr("Cannot update custom style %1\nError: %2").arg(mStylesheet.name(), mStylesheet.errormsg()));
  }
  emit accept();
}
