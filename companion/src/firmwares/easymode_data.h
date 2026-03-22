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

#include <QtCore>

class EasyModeData {
  Q_DECLARE_TR_FUNCTIONS(EasyModeData)

  public:
    enum ModelType {
      EASYMODE_NONE = 0,
      EASYMODE_AIRPLANE,
      EASYMODE_HELICOPTER,
      EASYMODE_GLIDER,
      EASYMODE_MULTIROTOR,
      EASYMODE_CAR,
      EASYMODE_BOAT,
      EASYMODE_TYPE_COUNT
    };

    enum WingType {
      EASYWING_SINGLE_AIL = 0,
      EASYWING_DUAL_AIL,
      EASYWING_FLAPERON,
      EASYWING_1AIL_1FLAP,
      EASYWING_2AIL_1FLAP,
      EASYWING_2AIL_2FLAP,
      EASYWING_2AIL_4FLAP,
      EASYWING_ELEVON,
      EASYWING_DELTA,
      EASYWING_COUNT
    };

    enum TailType {
      EASYTAIL_NORMAL = 0,
      EASYTAIL_V_TAIL,
      EASYTAIL_TAILLESS,
      EASYTAIL_DUAL_ELEVATOR,
      EASYTAIL_AILEVATOR,
      EASYTAIL_COUNT
    };

    enum MotorType {
      EASYMOTOR_NONE = 0,
      EASYMOTOR_SINGLE_ELECTRIC,
      EASYMOTOR_SINGLE_NITRO,
      EASYMOTOR_COUNT
    };

    enum MultiChannelOrder {
      EASYMULTI_AETR = 0,
      EASYMULTI_TAER,
      EASYMULTI_RETA,
      EASYMULTI_ORDER_COUNT
    };

    EasyModeData() { clear(); }

    unsigned int modelType;
    unsigned int wingType;
    unsigned int tailType;
    unsigned int motorType;

    // Channel map (-1 = none)
    int throttle;
    int aileron;
    int aileron2;
    int elevator;
    int elevator2;
    int rudder;
    int flap;
    int flap2;
    int flap3;
    int flap4;
    int steering;
    int aux1;
    int aux2;

    // Options
    int expoAileron;
    int expoElevator;
    int expoRudder;
    int dualRateLow;
    int aileronDifferential;
    int aileronToRudderMix;
    int flapToElevatorComp;
    unsigned int multiChannelOrder;
    unsigned int swashType;
    unsigned int crowEnabled;

    bool isActive() const { return modelType != EASYMODE_NONE; }

    void clear() {
      memset(reinterpret_cast<void *>(this), 0, sizeof(EasyModeData));
      throttle = aileron = aileron2 = elevator = elevator2 = -1;
      rudder = flap = flap2 = flap3 = flap4 = steering = -1;
      aux1 = aux2 = -1;
    }
};
