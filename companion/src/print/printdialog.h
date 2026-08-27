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

#ifndef _PRINTDIALOG_H_
#define _PRINTDIALOG_H_

#include <QtWidgets>
#include "eeprominterface.h"
#include "multimodelprinter.h"

namespace Ui {
  class PrintDialog;
}

class PrintDialog : public QDialog
{
  Q_OBJECT

  public:
    PrintDialog(QWidget * parent, Firmware * firmware, GeneralSettings & generalSettings, ModelData & model, const QString & filename="");
    ~PrintDialog();
    void closeEvent(QCloseEvent * event);

    Firmware * firmware;
    GeneralSettings & generalSettings;
    ModelData & model;

    QString printfilename;

  protected:
    Ui::PrintDialog *ui;
    MultiModelPrinter multiModelPrinter;

    void printToFile();

  private slots:
    void on_printButton_clicked();
    void on_printFileButton_clicked();
    void autoClose();
    void on_styleButton_clicked();
};

#endif // _PRINTDIALOG_H_
