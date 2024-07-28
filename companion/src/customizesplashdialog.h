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

#ifndef _CUSTOMIZESPLASHDIALOG_H_
#define _CUSTOMIZESPLASHDIALOG_H_

#include <QtWidgets>
#include "firmwareinterface.h"

namespace Ui
{
  class CustomizeSplashDialog;
}

const int WIDTH_TARANIS = 212;
const int HEIGHT_TARANIS = 64;
const int WIDTH_9X = 128;
const int HEIGHT_9X = 64;

enum Source {FW, PICT, PROFILE, UNDEFINED};
enum LCDFormat {LCD9X, LCDTARANIS};

class Side
{
public:
  Side(QDialog* parent);
  void markSourceButton();
  void copyImage( Side );
  bool displayImage( QString fileName, Source source );
  bool saveImage();
  bool refreshImage();

  QLabel *imageLabel;
  QLineEdit *fileNameEdit;
  QPushButton *saveButton;
  QPushButton *invertButton;
  QToolButton *libraryButton;
  QPushButton *loadFwButton;
  QPushButton *loadPictButton;
  QPushButton *loadProfileButton;

  QString *saveToFileName;

private:
  QDialog* parent;
  Source *source;
  LCDFormat *format;
};

class CustomizeSplashDialog : public QDialog
{
  Q_OBJECT
  Side left;
  Side right;

public:
  explicit CustomizeSplashDialog(QWidget *parent = 0);
  ~CustomizeSplashDialog();

private slots:
  void on_leftLoadFwButton_clicked();
  void on_leftLoadPictButton_clicked();
  void on_leftLoadProfileButton_clicked();
  void on_leftLibraryButton_clicked();
  void on_leftSaveButton_clicked();
  void on_leftInvertButton_clicked();

  void on_rightLoadFwButton_clicked();
  void on_rightLoadPictButton_clicked();
  void on_rightLoadProfileButton_clicked();
  void on_rightLibraryButton_clicked();
  void on_rightSaveButton_clicked();
  void on_rightInvertButton_clicked();

  void on_copyRightToLeftButton_clicked();
  void on_copyLeftToRightButton_clicked();

private:
  void loadFirmware( Side );
  void loadPicture( Side );
  void loadProfile( Side );
  void libraryButton_clicked( Side );
  void saveButton_clicked( Side );
  void invertButton_clicked( Side );

  Ui::CustomizeSplashDialog *ui;
  enum sideEnum { LEFT, RIGHT };
};

#endif // _CUSTOMIZESPLASHDIALOG_H_
