/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#pragma once

#include "modeledit.h"

#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QGroupBox>

class EasyModePanel : public ModelPanel
{
    Q_OBJECT

  public:
    EasyModePanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~EasyModePanel();
    void update();

  private slots:
    void edited();
    void onConvertToExpert();

  private:
    QComboBox *modelTypeCombo;
    QComboBox *wingTypeCombo;
    QComboBox *tailTypeCombo;
    QComboBox *motorTypeCombo;
    QSpinBox *chAileron, *chAileron2, *chElevator, *chThrottle, *chRudder;
    QSpinBox *chFlap, *chFlap2, *chFlap3, *chFlap4, *chSteering;
    QSpinBox *chAux1, *chAux2;
    QSpinBox *expoAil, *expoEle, *expoRud;
    QSpinBox *dualRateLow, *ailDiff, *ail2Rud;
    QComboBox *chOrderCombo;
    QPushButton *convertBtn;

    QGroupBox *wingGroup, *tailGroup, *motorGroup;
    QGroupBox *channelGroup, *optionsGroup;

    void buildUI();
    void updateVisibility();
};
