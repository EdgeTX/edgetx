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

#include "printdialog.h"
#include "ui_printdialog.h"
#include "helpers.h"
#include "styleeditdialog.h"
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

PrintDialog::PrintDialog(QWidget *parent, Firmware * firmware, GeneralSettings & generalSettings, ModelData & model, const QString & filename) :
  QDialog(parent),
  firmware(firmware),
  generalSettings(generalSettings),
  model(model),
  printfilename(filename),
  ui(new Ui::PrintDialog),
  multiModelPrinter(firmware)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("print.png"));
  setWindowTitle(model.name);
  multiModelPrinter.setModel(0, &model, &generalSettings);
  ui->textEdit->setHtml(multiModelPrinter.print(ui->textEdit->document()));
  if (!printfilename.isEmpty()) {
    printToFile();
    QTimer::singleShot(0, this, SLOT(autoClose()));
  }
}

void PrintDialog::closeEvent(QCloseEvent *event)
{
}

PrintDialog::~PrintDialog()
{
  delete ui;
}

void PrintDialog::on_printButton_clicked()
{
  QPrinter printer;
  printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
  QPrintDialog *dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Document"));
  if (dialog->exec() != QDialog::Accepted)
    return;
  ui->textEdit->print(&printer);
}

void PrintDialog::on_printFileButton_clicked()
{
  QString fn = QFileDialog::getSaveFileName(this,tr("Select output file"),QString(),tr("PDF files(*.pdf);;HTML files (*.htm *.html);;All files (*)"));
  if (fn.isEmpty())
    return;
  if (! (fn.endsWith(".pdf", Qt::CaseInsensitive) || fn.endsWith(".htm", Qt::CaseInsensitive) || fn.endsWith(".html", Qt::CaseInsensitive)) )
    fn += ".pdf"; // default
  if (fn.endsWith(".pdf", Qt::CaseInsensitive)) {
    QPrinter printer;
    printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(fn);
    ui->textEdit->print(&printer);
  }
  else {
    QTextDocumentWriter writer(fn);
     writer.write(ui->textEdit->document());
  }
}

void PrintDialog::printToFile()
{
  if (printfilename.isEmpty())
    return;
  if (! (printfilename.endsWith(".pdf", Qt::CaseInsensitive) || printfilename.endsWith(".htm", Qt::CaseInsensitive) || printfilename.endsWith(".html", Qt::CaseInsensitive)) )
    printfilename += ".pdf"; // default
  if (printfilename.endsWith(".pdf", Qt::CaseInsensitive)) {
    QPrinter printer;
    printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(printfilename);
    ui->textEdit->print(&printer);
  }
  else {
    QTextDocumentWriter writer(printfilename);
    writer.write(ui->textEdit->document());
  }
}

void PrintDialog::autoClose()
{
  this->close();
}

void PrintDialog::on_styleButton_clicked()
{
  StyleEditDialog *g = new StyleEditDialog(this, MODEL_PRINT_CSS);
  if (g->exec() == QDialog::Accepted)
    ui->textEdit->setHtml(multiModelPrinter.print(ui->textEdit->document()));
}
