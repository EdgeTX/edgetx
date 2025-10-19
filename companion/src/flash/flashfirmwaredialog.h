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

#include <QtWidgets>

namespace Ui
{
  class FlashFirmwareDialog;
}

class FlashFirmwareDialog : public QDialog
{
    Q_OBJECT

  public:
    FlashFirmwareDialog(QWidget *parent = 0);
    ~FlashFirmwareDialog();

  private slots:
    void detectClicked(bool atLoad = false);
    void loadClicked();
    void writeButtonClicked();
    void useProfileSplashClicked();
    void useFirmwareSplashClicked();
    void useLibrarySplashClicked();
    void useExternalSplashClicked();

  protected:
    void updateUI();
    void startWrite(const QString &filename);

  private:
    enum ConnectionModes {
      CONNECTION_NONE,
      CONNECTION_UF2,
      CONNECTION_DFU
    };

    enum ImageSource {
      FIRMWARE,
      PROFILE,
      LIBRARY,
      EXTERNAL
    };

    Ui::FlashFirmwareDialog *ui;
    QString fwName;
    ImageSource imageSource;
    QString imageFile;
    int connectionMode;
    QCheckBox *chkHWComp;
    QCheckBox *chkProfComp;
    bool isFileConnectionCompatible();
    void shrink();
};
