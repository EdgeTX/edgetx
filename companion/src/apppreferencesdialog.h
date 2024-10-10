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

#include "eeprominterface.h"
#include "appdata.h"

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>

class FilteredItemModelFactory;

namespace Ui {
  class AppPreferencesDialog;
}

class Joystick;
class UpdateFactories;

class AppPreferencesDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit AppPreferencesDialog(QWidget * parent, UpdateFactories * factories);
    ~AppPreferencesDialog();

    Joystick * joystick;

  public slots:
    void accept() Q_DECL_OVERRIDE;
    void setMainWinHasDirtyChild(bool value);

  signals:
    void firmwareProfileChanged();
    void firmwareProfileAboutToChange(bool saveFiles = true);

  protected slots:
    void shrink();
    void onBaseFirmwareChanged();
    void onFirmwareOptionChanged(bool state);
    void toggleAppLogSettings();

    void on_libraryPathButton_clicked();
    void on_snapshotPathButton_clicked();
    void on_snapshotClipboardCKB_clicked();
    void on_backupPathButton_clicked();
    void on_ProfilebackupPathButton_clicked();
    void on_ge_pathButton_clicked();

    void on_sdPathButton_clicked();
    void on_SplashSelect_clicked();
    void on_clearImageButton_clicked();
    void on_btn_appLogsDir_clicked();
    void on_btnClearPos_clicked();

#if defined(JOYSTICKS)
    void on_joystickChkB_clicked();
    void on_joystickcalButton_clicked();
#endif

  private:
    void initSettings();
    void populateFirmwareOptions(const Firmware *);
    Firmware * getBaseFirmware() const;
    Firmware * getFirmwareVariant() const;
    bool displayImage(const QString & fileName);

    Ui::AppPreferencesDialog *ui;
    QMap<QString, QCheckBox *> optionsCheckBoxes;
    bool updateLock;
    bool mainWinHasDirtyChild;

    UpdateFactories *factories;
    QLabel *lblName[MAX_COMPONENTS];
    QCheckBox *chkCheckForUpdate[MAX_COMPONENTS];
    QComboBox *cboReleaseChannel[MAX_COMPONENTS];
    QPushButton *btnComponentOptions[MAX_COMPONENTS];

    FilteredItemModelFactory *panelItemModels;

    void loadUpdatesTab();

};
