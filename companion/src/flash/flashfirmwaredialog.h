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

#ifndef _FLASHFIRMWAREDIALOG_H_
#define _FLASHFIRMWAREDIALOG_H_

#include <QtWidgets>
#include "eeprominterface.h"
#include "firmwareinterface.h"

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
  void on_firmwareLoad_clicked();
  void on_burnButton_clicked();
  void on_cancelButton_clicked();
  void on_useProfileSplash_clicked();
  void on_useFirmwareSplash_clicked();
  void on_useLibrarySplash_clicked();
  void on_useExternalSplash_clicked();
  void shrink();

protected:
  void updateUI();
  void startFlash(const QString &filename);

private:
  Ui::FlashFirmwareDialog *ui;
  QString fwName;
  RadioData radioData;
  enum ImageSource {FIRMWARE, PROFILE, LIBRARY, EXTERNAL};
  ImageSource imageSource;
  QString imageFile;
};

#endif // _FLASHFIRMWAREDIALOG_H_
