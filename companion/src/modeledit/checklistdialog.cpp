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

#include "checklistdialog.h"
#include "ui_checklistdialog.h"
#include "helpers.h"

#include <QFile>

ChecklistDialog::ChecklistDialog(QWidget *parent, ModelData * model):
  QDialog(parent),
  ui(new Ui::ChecklistDialog),
  model(model),
  mDirty(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("edit.png"));

  QFont f("Courier", 10, QFont::Normal);    // fixed width font
  ui->pteCheck->setFont(f);

  connect(ui->pteCheck, &QPlainTextEdit::textChanged, [&]() { mDirty = true; });
  connect(ui->pteCheck, &QPlainTextEdit::cursorPositionChanged, [&]()
    {
      QTextCursor tc = ui->pteCheck->textCursor();
      ui->lblCursorPos->setText(tr("Line %1, Col %2").arg(tc.blockNumber() + 1).arg(tc.positionInBlock() + 1));
    }
  );
  connect(ui->pbImport, &QPushButton::clicked, this, &ChecklistDialog::import);
  connect(ui->pbCancel, &QPushButton::clicked, [this]() { reject(); });
  connect(ui->pbOK, &QPushButton::clicked, this, &ChecklistDialog::update);

  ui->pteCheck->setPlainText(model->checklistData.data());
  mDirty = false;
}

ChecklistDialog::~ChecklistDialog()
{
  delete ui;
}

void ChecklistDialog::update()
{
  if (mDirty) {
    model->checklistData = Helpers::removeAccents(ui->pteCheck->toPlainText()).toUtf8();
    if (model->checklistData.trimmed().isEmpty())
      model->checklistData.clear();
    emit updated();
  }

  emit accept();
}

void ChecklistDialog::import()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Import Checklist File"), QString(), tr("Checklist Files (*.txt)"));

  if (!filename.isEmpty()) {
    ui->pteCheck->setPlainText(readFile(filename,true));
    mDirty = true;
  }
}

QString ChecklistDialog::readFile(const QString & filepath, const bool exists)
{
  QString data = "";

  if (!filepath.isNull()) {
    QFile file(filepath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      if (exists) {
        QMessageBox::critical(this, tr("Model Checklist"), tr("Cannot open file %1:\n%2.").arg(QDir::toNativeSeparators(filepath), file.errorString()));
      }
    } else {
      QTextStream in(&file);

      if (in.status()==QTextStream::Ok) {
        data = Helpers::removeAccents(in.readAll());

        if (!(in.status()==QTextStream::Ok)) {
          QMessageBox::critical(this, tr("Model Checklist"), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(filepath), file.errorString()));
          data = "";
        }
      }
    }
    file.close();
  }
  return data;
}
