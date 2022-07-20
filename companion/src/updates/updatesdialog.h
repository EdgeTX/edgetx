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

#pragma once

#include "updateinterface.h"
#include "appdata.h"

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

namespace Ui {
  class UpdatesDialog;
}

class UpdatesDialog : public QDialog
{
    Q_OBJECT

  public:
    UpdatesDialog(QWidget * parent, UpdateFactories * factories);
    virtual ~UpdatesDialog();

  signals:

  private slots:

  private:
    Ui::UpdatesDialog *ui;
    const UpdateFactories *factories;

    QCheckBox *chkUpdate[MAX_COMPONENTS];
    QLabel *lblCurrentRel[MAX_COMPONENTS];
    QLabel *lblUpdateRel[MAX_COMPONENTS];
    QPushButton *btnOptions[MAX_COMPONENTS];
};

