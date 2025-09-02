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
#include "wizardwizmix.h"

#include <QtCore>

#define WIZ_MAX_CHANNELS 8

class Vehicle
{
public:
        //unique per vehicle type
        // air:           1xx range
        // on/off road:   2xx range
        // marine:        3xx range
        static const int TYPE = 0;

        static const int NO_INPUT = 0;

        static const int NO_OPTION = 0;

        static const int PAGE_NONE = 0;

        // ALL BASIC PAGE HAVE 0-9 VALUES
        static const int PAGE_CONCLUSION = 1;
        static const int PAGE_MODELS = 2;

        //
        class MixerChannel;
      };

class AirVehicle : public Vehicle
{
public:
    static const int TYPE = 100;
    static const int RUDDER_INPUT =     1;
    static const int ELEVATOR_INPUT =   2;
    static const int THROTTLE_INPUT =   3;
    static const int AILERONS_INPUT =   4;
    static const int FLAPS_INPUT =      5;
    static const int AIRBRAKES_INPUT =  6;

    static const int WIZ_MAX_OPTIONS      =   3;
    static const int FLIGHT_TIMER_OPTION =    0;
    static const int THROTTLE_CUT_OPTION =    1;
    static const int THROTTLE_TIMER_OPTION =  2;

    // ALL GROUP PAGES HAVE 10-99 VALUES
    static const int PAGE_THROTTLE =    10;

    // class to hold info for the mixer-page-channels. this (for now) assumes a maximum of 2 inputs being mixed.
    // this is specific for AirVehicles at the moment. Plane::MixerChannel could be a shell void in Vehicle class which gets
    // overwritten in the AirVehicle class. This allows for a different handling of Plane::MixerChannel in SurfaceVehicle
    // or MarineVehicle as well.
    class MixerChannel{
      public:
        int page;     // Originating dialog, only of interest for producer
        bool prebooked;     // Temporary lock variable
        int input1;
        int input2;
        int weight1;
        int weight2;

        Channel();
        void clear();
    };

    class WizMix;
    // {
    //   Q_OBJECT

    //   public:
    //     bool complete;
    //     char name[MODEL_NAME_LEN + 1];
    //     unsigned int modelId;
    //     const GeneralSettings & settings;
    //     const ModelData & originalModelData;
    //     int vehicle;
    //     MixerChannel channel[WIZ_MAX_CHANNELS];
    //     bool options[AirVehicle::WIZ_MAX_OPTIONS];

    //     WizMix(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData);
    //     operator ModelData();

    //   private:
    //     WizMix();
    //     void addMix(ModelData & model, int input, int weight, int channel, int & mixerIndex);
    //     void maxMixSwitch(char *name, MixData &mix, int destCh, int sw, int weight);

    // };

};



class Plane : public AirVehicle
{
public:
    static const int TYPE = 101;

    // ALL VEHICLE SPECIFIC PAGES HAVE 100-999 VALUES
    static const int PAGE_WINGTYPES =   121;
    static const int PAGE_AILERONS =    122;
    static const int PAGE_FLAPS =       123;
    static const int PAGE_AIRBRAKES =   124;
    static const int PAGE_ELEVONS =     125;
    static const int PAGE_RUDDER =      126;
    static const int PAGE_TAILS =       127;
    static const int PAGE_TAIL =        128;
    static const int PAGE_VTAIL =       129;
    static const int PAGE_SIMPLETAIL =  130;
    static const int PAGE_LIGHTS =      131;
    static const int PAGE_OPTIONS =     132;


};


class Helicopter : public AirVehicle
{
public:
    static const int TYPE = 103;

    // ALL VEHICLE SPECIFIC PAGES HAVE 100-999 VALUES
    static const int PAGE_CYCLIC =      100;
    static const int PAGE_GYRO =        101;
    static const int PAGE_FLYBAR =      102;
    static const int PAGE_FLYBARLESS =  103;
    static const int PAGE_HELICONTROL = 104;
    static const int PAGE_LIGHTS =      199;
    static const int PAGE_OPTIONS =     200;


};

class MultiRotor : public AirVehicle
{
public:
    static const int TYPE = 102;

    // ALL VEHICLE SPECIFIC PAGES HAVE 100-999 VALUES
    static const int PAGE_MULTIROTOR =  100;
    static const int PAGE_LIGHTS =      199;
    static const int PAGE_OPTIONS =     200;


};

enum VehicleName {
  NOVEHICLE,
  PLANE=101,
  MULTICOPTER,
  HELICOPTER
};



