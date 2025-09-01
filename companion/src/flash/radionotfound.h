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

#ifndef _RADIONOTFOUND_H_
#define _RADIONOTFOUND_H_

#include <QDialog>

namespace Ui {
    class RadioNotFoundDialog;
}

class RadioNotFoundDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RadioNotFoundDialog(QWidget *parent = 0);
    ~RadioNotFoundDialog();
private:
    Ui::RadioNotFoundDialog *ui;
private slots:
    void on_OK_clicked();
};

#endif // _RADIONOTFOUND_H_
