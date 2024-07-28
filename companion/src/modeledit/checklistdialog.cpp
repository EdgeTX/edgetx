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
#include <QTextStream>

ChecklistDialog::ChecklistDialog(QWidget *parent, const ModelData * model):
  QDialog(parent),
  ui(new Ui::ChecklistDialog),
  mDirty(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("edit.png"));

  mChecklistFolder = Helpers::getChecklistsPath();
  mModelChecklist = Helpers::getChecklistFilePath(model);
  ui->file->setText("File: " + mModelChecklist);
  ui->pteCheck->setPlainText(readFile(mModelChecklist, QFile::exists(mModelChecklist)));

  QFont f("Courier", 12, QFont::Normal);    // fixed width font
  ui->pteCheck->setFont(f);

  connect(ui->pteCheck, SIGNAL(textChanged()), this, SLOT(changed()));
  connect(ui->pteCheck, SIGNAL(cursorPositionChanged()), this, SLOT(cursorChanged()));
  connect(ui->pbImport, SIGNAL(clicked()), this, SLOT(import()));
  connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->pbOK, SIGNAL(clicked()), this, SLOT(update()));

  cursorChanged();  // force label to update
}

ChecklistDialog::~ChecklistDialog()
{
  delete ui;
}

void ChecklistDialog::changed()
{
  mDirty = true;
}

void ChecklistDialog::import()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Checklist"), mChecklistFolder, tr("Checklist Files (*.txt)"));
  if (!filename.isNull())
    ui->pteCheck->setPlainText(readFile(filename,true));
  mDirty = true;
}

void ChecklistDialog::update()
{
  if (mDirty) {
    QFile file(mModelChecklist);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::critical(this, tr("Model Checklist"), tr("Cannot open file for writing %1:\n%2.").arg(mModelChecklist, file.errorString()));
    }
    else {
      QTextStream out(&file);
      if (out.status()==QTextStream::Ok) {
        out << Helpers::removeAccents(ui->pteCheck->toPlainText());
        if (!(out.status()==QTextStream::Ok)) {
          QMessageBox::critical(this, tr("Model Checklist"), tr("Cannot write to file %1:\n%2.").arg(mModelChecklist, file.errorString()));
          if (!file.flush()) {
            QMessageBox::critical(this, tr("Model Checklist"), tr("Cannot write file %1:\n%2.").arg(mModelChecklist, file.errorString()));
          }
        }
      }
    }
    file.close();
  }
  emit accept();
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
    }
    else {
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

void ChecklistDialog::cursorChanged()
{
  QTextCursor tc = ui->pteCheck->textCursor();
  ui->lblCursorPos->setText(tr("Line %1, Col %2").arg(tc.blockNumber() + 1).arg(tc.positionInBlock() + 1));
}
