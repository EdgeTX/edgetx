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

#ifndef _FLASHEEPROMDIALOG_H_
#define _FLASHEEPROMDIALOG_H_

#include <constants.h>
#include <QtWidgets>

namespace Ui
{
  class FlashEEpromDialog;
}

class RadioData;

class FlashEEpromDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlashEEpromDialog(QWidget *parent, const QString &fileName="");
  ~FlashEEpromDialog();

private slots:
  void on_eepromLoad_clicked();
  void on_burnButton_clicked();
  void on_cancelButton_clicked();
  void shrink();

protected:
  void updateUI();
  bool isValidEEprom(const QString &filename);
  int getEEpromVersion(const QString &filename);
  bool patchCalibration(RadioData * radioData);
  bool patchHardwareSettings(RadioData * radioData);
  virtual void showEvent(QShowEvent *);

private:
  Ui::FlashEEpromDialog *ui;
  QString eepromFilename;
};

#endif // _FLASHEEPROMDIALOG_H_
