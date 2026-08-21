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

#include "prefs_panel.h"
#include "eeprominterface.h"

#include <QDialog>

class UpdateFactories;

namespace Ui {
  class PrefsEdit;
}

class PrefsEditDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PrefsEditDialog(QWidget * parent, UpdateFactories * factories);
    virtual ~PrefsEditDialog();

  public slots:
    void setMainWinHasDirtyChild(bool value);

  signals:
    void firmwareProfileChanged();
    void firmwareProfileAboutToChange(bool saveFiles = true);
    void resetFirmware();

  protected slots:
    void accept() Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void done(int r) Q_DECL_OVERRIDE;
    void reject() Q_DECL_OVERRIDE;

  private slots:

  private:
    Ui::PrefsEdit *ui;
    bool mainWinHasDirtyChild;
    Firmware *firmware;
    Board::Type board;
    Profile &profile;
    QList<PrefsPanel *> panels;
    bool dirty;

    PrefsPanel * addTab(PrefsPanel * panel, QString text);
    bool maybeSave();
    bool save();
    void shrink();
};
