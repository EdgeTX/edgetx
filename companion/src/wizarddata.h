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

#include <QtCore>

#define WIZ_MAX_CHANNELS 8

enum Input {
  NO_INPUT,
  RUDDER_INPUT,
  ELEVATOR_INPUT,
  THROTTLE_INPUT,
  AILERONS_INPUT,
  FLAPS_INPUT,
  AIRBRAKES_INPUT
};

enum Vehicle {
  NOVEHICLE,
  PLANE,
  MULTICOPTER,
  HELICOPTER
};

#define WIZ_MAX_OPTIONS 3
enum Options {
  FLIGHT_TIMER_OPTION,
  THROTTLE_CUT_OPTION,
  THROTTLE_TIMER_OPTION
};

enum WizardPage {
  Page_None = -1,
  Page_Models,
  Page_Throttle,
  Page_Wingtypes,
  Page_Ailerons,
  Page_Flaps,
  Page_Airbrakes,
  Page_Elevons,
  Page_Rudder,
  Page_Tails,
  Page_Tail,
  Page_Vtail,
  Page_Simpletail,
  Page_Cyclic,
  Page_Gyro,
  Page_Flybar,
  Page_Fblheli,
  Page_Helictrl,
  Page_Multirotor,
  Page_Options,
  Page_Conclusion
};

class Channel
{
  public:
    WizardPage page;     // Originating dialog, only of interest for producer
    bool prebooked;     // Temporary lock variable
    Input input1;
    Input input2;
    int weight1;
    int weight2;

    Channel();
    void clear();
};

class WizMix : public QObject
{
  Q_OBJECT

  public:
    bool complete;
    char name[MODEL_NAME_LEN + 1];
    unsigned int modelId;
    const GeneralSettings & settings;
    const ModelData & originalModelData;
    Vehicle vehicle;
    Channel channel[WIZ_MAX_CHANNELS];
    bool options[WIZ_MAX_OPTIONS];

    WizMix(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData);
    operator ModelData();

  private:
    WizMix();
    void addMix(ModelData & model, Input input, int weight, int channel, int & mixerIndex);
    void maxMixSwitch(char *name, MixData &mix, int destCh, int sw, int weight);

};
