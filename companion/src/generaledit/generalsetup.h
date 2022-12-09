/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _GENERALSETUP_H_
#define _GENERALSETUP_H_

#include "generaledit.h"
#include "eeprominterface.h"

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
    void on_splashScreenChkB_stateChanged(int);
    void on_splashScreenDuration_currentIndexChanged(int index);
    void on_alarmwarnChkB_stateChanged(int);
    void on_rssiPowerOffWarnChkB_stateChanged(int);
    void on_gpsFormatCB_currentIndexChanged(int index);
    void on_displayTypeCB_currentIndexChanged(int index);
    void on_BLBright_SB_editingFinished();
    void on_OFFBright_SB_editingFinished();
    void on_re_CB_currentIndexChanged(int index);
    void on_countrycode_CB_currentIndexChanged(int index);
    void on_units_CB_currentIndexChanged(int index);
    void on_faimode_CB_stateChanged(int );
    void on_rotEncMode_CB_currentIndexChanged(int index);
    void on_speakerPitchSB_editingFinished();
    void on_timezoneSB_editingFinished();
    void on_adjustRTC_stateChanged(int);
    void on_hapticStrength_valueChanged();
    void on_soundModeCB_currentIndexChanged(int index);
    void on_beeperlenCB_currentIndexChanged(int index);
    void on_volume_SB_editingFinished();
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

    void on_pwrOnDelay_valueChanged(int);
    void on_pwrOffDelay_valueChanged(int);

  private:
    Ui::GeneralSetup *ui;

    void setValues();
    void populateBacklightCB();
    void populateVoiceLangCB();
    void populateRotEncCB(int reCount);
    void populateRotEncModeCB();
    void updateVarioPitchRange();
};

#endif // _GENERALSETUP_H_
