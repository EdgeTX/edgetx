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

#include "prefs_edit.h"

namespace Ui {
  class PrefsUpdate;
}

class UpdateFactories;

class PrefsUpdatePanel : public PrefsPanel
{
    Q_OBJECT

  public:
    PrefsUpdatePanel(QWidget * parent, Firmware * fw, Board::Type & bd, Profile & prof, UpdateFactories * factories);
    virtual ~PrefsUpdatePanel();

    virtual void save() override;
    virtual void update() override;

  public slots:
    void onSDPathChanged(QString path);

  private:
    Ui::PrefsUpdate *ui;
    UpdateFactories *factories;
    // general
    // folders
    AutoLineEdit   *leDownloadDir;
    AutoCheckBox   *chkDecompressDirUseDwnld;
    AutoLineEdit   *leDecompressDir;
    AutoCheckBox   *chkUpdateDirUseSD;
    AutoLineEdit   *leUpdateDir;
    //components
    AutoLabel      *lblName[MAX_COMPONENTS];
    AutoCheckBox   *chkCheckForUpdate[MAX_COMPONENTS];
    AutoComboBox   *cboReleaseChannel[MAX_COMPONENTS];
    AutoPushButton *btnComponentOptions[MAX_COMPONENTS];
    // options
    AutoCheckBox   *chkDelDownloads;
    AutoCheckBox   *chkDelDecompress;
    AutoComboBox   *cboLogLevel;
    // post update
    AutoCheckBox   *chkPrmptFlash;
    AutoCheckBox   *chkPrmptSDSync;
    AutoCheckBox   *chkPrmptCpnInstall;

    QString profileSDPath;

    void sectionFolders();
    void sectionComponents();
    void sectionOptions();
    void sectionPostUpdate();

    void setValuesFromSettings();
};
