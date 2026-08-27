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

#ifndef _COMPAREDIALOG_H_
#define _COMPAREDIALOG_H_

#include <QtWidgets>
#include "multimodelprinter.h"

namespace Ui {
  class CompareDialog;
}

class CompareDialog : public QDialog
{
  Q_OBJECT

  public:
    CompareDialog(QWidget *parent, Firmware * firmware);
    ~CompareDialog();

  protected:
    struct GMData {
        ModelData model;
        GeneralSettings gs;
        QString fname;
    };

    MultiModelPrinter * multimodelprinter;
    QVector<GMData> modelsList;
    QMap<int, GMData> modelsMap;
    Ui::CompareDialog * ui;

  protected slots:
    void removeModelBtnClicked();
    void on_printButton_clicked();
    void on_printFileButton_clicked();
    void on_styleButton_clicked();

  protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    bool handleMimeData(const QMimeData * mimeData);
    void compare();
    void removeModel(int idx);
};

#endif // _COMPAREDIALOG_H_
