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

#ifndef _SPLASHLIBRARYDIALOG_H_
#define _SPLASHLIBRARYDIALOG_H_

#include <QtWidgets>

namespace Ui {
    class SplashLibraryDialog;
}

class SplashLibraryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SplashLibraryDialog(QWidget *parent = 0, QString * fileName=NULL);
    ~SplashLibraryDialog();

private slots:
    void dclose();
    void onButtonPressed(int button);
    void on_nextPage_clicked();
    void on_prevPage_clicked();
 
private:
    void getFileList();
    void setupPage(int page);
    Ui::SplashLibraryDialog *ui;
    QString * splashFileName;
    QString libraryPath;
    QStringList imageList;
    int page;
};

#endif // _SPLASHLIBRARYDIALOG_H_
