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

#include "generaledit.h"
#include "eeprominterface.h"

class FilteredItemModelFactory;

namespace Ui {
  class GeneralSetup;
}

class GeneralSetupPanel : public GeneralPanel
{
    Q_OBJECT

  public:
    GeneralSetupPanel(QWidget *parent, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~GeneralSetupPanel();

  private slots:
    void on_splashScreenDuration_currentIndexChanged(int index);
    void on_alarmwarnChkB_stateChanged(int);
    void on_rssiPowerOffWarnChkB_stateChanged(int);
    void on_trainerPowerOffWarnChkB_stateChanged(int);
    void on_gpsFormatCB_currentIndexChanged(int index);
    void on_displayTypeCB_currentIndexChanged(int index);
    void on_BLBright_SB_editingFinished();
    void on_OFFBright_SB_editingFinished();
    void on_countrycode_CB_currentIndexChanged(int index);
    void on_units_CB_currentIndexChanged(int index);
    void on_ppm_units_CB_currentIndexChanged(int index);
    void on_faimode_CB_stateChanged(int );
    void on_speakerPitchSB_editingFinished();
    void on_timezoneLE_textEdited(const QString &text);
    void on_adjustRTC_stateChanged(int);
    void on_hapticStrength_valueChanged();
    void on_soundModeCB_currentIndexChanged(int index);
    void on_beeperlenCB_currentIndexChanged(int index);
    void on_volume_SL_valueChanged();
    void on_hapticmodeCB_currentIndexChanged(int index);
    void on_hapticLengthCB_currentIndexChanged(int index);
    void on_backlightswCB_currentIndexChanged(int index);
    void on_usbModeCB_currentIndexChanged(int index);
    void on_jackModeCB_currentIndexChanged(int index);
    void on_backlightColor_SL_valueChanged();
    void on_mavbaud_CB_currentIndexChanged(int index);
    void on_voiceLang_CB_currentIndexChanged(int index);
    void stickReverseEdited();
    void on_switchesDelay_valueChanged(int);
    void on_blAlarm_ChkB_stateChanged();
    void unlockSwitchEdited();
    void on_beepVolume_SL_valueChanged();
    void on_bgVolume_SL_valueChanged();
    void on_varioVolume_SL_valueChanged();
    void on_wavVolume_SL_valueChanged();

    void on_varioP0_SB_editingFinished();
    void on_varioPMax_SB_editingFinished();
    void on_varioR0_SB_editingFinished();
    void on_stickmodeCB_currentIndexChanged(int index);
    void on_channelorderCB_currentIndexChanged(int index);
    void on_beeperCB_currentIndexChanged(int index);
    void on_memwarnChkB_stateChanged(int );
    void on_inactimerSB_editingFinished();
    void on_backlightautoSB_editingFinished();

    void on_battwarningDSB_editingFinished();
    void on_vBatMinDSB_editingFinished();
    void on_vBatMaxDSB_editingFinished();
    void on_contrastSB_editingFinished();
    void on_registrationId_editingFinished();

    void on_pwrOnDelay_currentIndexChanged(int);
    void on_pwrOffDelay_currentIndexChanged(int);
    void on_pwrOnOffHaptic_CB_stateChanged(int);

    void on_modelQuickSelect_CB_stateChanged(int);
    void on_startSoundCB_stateChanged(int);
    void on_modelSelectLayout_CB_currentIndexChanged(int);
    void on_labelSingleSelect_CB_currentIndexChanged(int);
    void on_labelMultiMode_CB_currentIndexChanged(int);
    void on_favMultiMode_CB_currentIndexChanged(int);

    void on_pwrOffIfInactiveSB_editingFinished();

  private:
    Ui::GeneralSetup *ui;
    FilteredItemModelFactory *panelFilteredModels;

    void showLabelSelectOptions();

    void setValues();
    void populateBacklightCB();
    void populateVoiceLangCB();
    void updateVarioPitchRange();
};
