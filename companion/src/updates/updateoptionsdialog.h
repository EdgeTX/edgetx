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

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QVector>
#include <QSpinBox>

namespace Ui {
  class UpdateOptionsDialog;
}

class QAbstractButton;

class UpdateOptionsDialog : public QDialog
{
    Q_OBJECT

  public:
    UpdateOptionsDialog(QWidget * parent, UpdateInterface * iface, const int idx, const bool isRun);
    virtual ~UpdateOptionsDialog();

  signals:
    void changed(int index);

  private slots:
    void update();

  private:
    Ui::UpdateOptionsDialog *ui;
    UpdateInterface *iface;
    UpdateParameters *params;
    const int idx;
    const bool isRun;

    QVector<QComboBox *> cboAssetFilterTypes;
    QVector<QLineEdit *> leAssetFilters;
    QVector<QSpinBox *>  sbMaxExpects;
    QVector<QCheckBox *> chkDownloads;
    QVector<QCheckBox *> chkDecompresses;
    QVector<QCheckBox *> chkInstalls;
    QVector<QCheckBox *> chkCopies;
    QVector<QLabel *>    lblCopyFilters;
    QVector<QComboBox *> cboCopyFilterTypes;
    QVector<QLineEdit *> leCopyFilters;
    QVector<QLabel *>    lblSubFolders;
    QVector<QLineEdit *> leSubFolders;
};
