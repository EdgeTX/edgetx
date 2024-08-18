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

#include "comparedialog.h"
#include "ui_comparedialog.h"
#include "appdata.h"
#include "helpers.h"
#include "modelslist.h"
#include "styleeditdialog.h"
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

CompareDialog::CompareDialog(QWidget * parent, Firmware * firmware):
  QDialog(parent, Qt::Window),
  multimodelprinter(new MultiModelPrinter(firmware)),
  ui(new Ui::CompareDialog)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("compare.png"));
  setAcceptDrops(true);
  if (!g.compareWinGeo().isEmpty()) {
    restoreGeometry(g.compareWinGeo());
  }
}

CompareDialog::~CompareDialog()
{
  delete multimodelprinter;
  delete ui;
}

void CompareDialog::dragMoveEvent(QDragMoveEvent * event)
{
  if (event->mimeData()->hasFormat("application/x-companion-modeldata")) {
    event->acceptProposedAction();
  }
  else {
    event->ignore();
  }
}

void CompareDialog::dragEnterEvent(QDragEnterEvent * event)
{
  if (event->mimeData()->hasFormat("application/x-companion-modeldata")) {
    event->acceptProposedAction();
  }
  else {
    event->ignore();
  }
}

void CompareDialog::dragLeaveEvent(QDragLeaveEvent * event)
{
  event->accept();
}

bool CompareDialog::handleMimeData(const QMimeData * mimeData)
{
  QVector<ModelData> mList;
  GeneralSettings gs;
  QString fdata;

  if (!ModelsListModel::decodeMimeData(mimeData, &mList, &gs) || mList.isEmpty())
    return false;

  if (!ModelsListModel::decodeFileData(mimeData, &fdata))
    return false;

  for (int i=0; i < mList.size(); ++i) {
    GMData data;
    data.model = mList[i];
    data.gs = gs;
    data.fname = fdata;
    qDebug() << "filename" << data.fname;
    modelsList.append(data);
  }

  return true;
}

void CompareDialog::closeEvent(QCloseEvent * event)
{
  g.compareWinGeo(saveGeometry());
}

void CompareDialog::dropEvent(QDropEvent *event)
{
  if (handleMimeData(event->mimeData())) {
    event->accept();
    compare();
  }
}

void CompareDialog::compare()
{
  QLayoutItem *child;
  while ((child = ui->layout_modelNames->takeAt(0))) {
    if (child->widget())
      delete child->widget();
    delete child;
  }

  multimodelprinter->clearModels();
  ui->textEdit->clear();

  for (int i=0; i < modelsList.size(); ++i) {
    multimodelprinter->setModel(i, &modelsList[i].model, &modelsList[i].gs);
    QString name(modelsList.at(i).model.name);
    if (name.isEmpty())
      name = tr("Unnamed Model %1").arg(i+1);

    name.append(QString(" (%1)").arg(modelsList[i].fname));
    QWidget * hdr = new QWidget(this);
    hdr->setLayout(new QHBoxLayout());
    hdr->layout()->setContentsMargins(0, 0, 0, 0);
    hdr->layout()->setSpacing(2);
    QToolButton * btn = new QToolButton(hdr);
    btn->setIcon(CompanionIcon("clear.png"));
    btn->setProperty("index", i);
    btn->setFixedSize(18, 18);
    btn->setToolTip(tr("Click to remove this model."));
    hdr->layout()->addWidget(btn);
    QLabel * lbl = new QLabel(name, this);
    lbl->setStyleSheet("font-weight: bold;");
    hdr->layout()->addWidget(lbl);
    connect(btn, &QToolButton::clicked, this, &CompareDialog::removeModelBtnClicked);

    ui->layout_modelNames->addWidget(hdr);
  }
  if (modelsList.size())
    ui->textEdit->setHtml(multimodelprinter->print(ui->textEdit->document()));
}

void CompareDialog::removeModel(int idx)
{
  if (idx < modelsList.size()) {
    modelsList.remove(idx);
    compare();
  }
}

void CompareDialog::removeModelBtnClicked()
{
  if (sender() && sender()->property("index").isValid())
    removeModel(sender()->property("index").toInt());
}

void CompareDialog::on_printButton_clicked()
{
  QPrinter printer;
  printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
  QPrintDialog * dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Document"));
  if (dialog->exec() != QDialog::Accepted)
    return;
  ui->textEdit->print(&printer);
}

void CompareDialog::on_printFileButton_clicked()
{
  QPrinter printer;
  QString filename = QFileDialog::getSaveFileName(this, tr("Select PDF output file"), QString(), "Pdf File(*.pdf)");
  printer.setPageMargins(QMarginsF(10.0, 10.0, 10.0, 10.0), QPageLayout::Millimeter);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPageOrientation(QPageLayout::Portrait);
  printer.setColorMode(QPrinter::Color);
  if (!filename.isEmpty()) {
    if (QFileInfo(filename).suffix().isEmpty())
      filename.append(".pdf");
    printer.setOutputFileName(filename);
    ui->textEdit->print(&printer);
  }
}

void CompareDialog::on_styleButton_clicked()
{
  StyleEditDialog *g = new StyleEditDialog(this, MODEL_PRINT_CSS);
  if (g->exec() == QDialog::Accepted)
    compare();
}
