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

class WizMix : public QObject
{
    Q_OBJECT

    public:
    bool complete;
    char name[MODEL_NAME_LEN + 1];
    unsigned int modelId;
    const GeneralSettings & settings;
    const ModelData & originalModelData;
    int vehicle;
    MixerChannel channel[WIZ_MAX_CHANNELS];
    bool options[AirVehicle::WIZ_MAX_OPTIONS];

    WizMix(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData);
    operator ModelData();

    private:
    WizMix();
    void addMix(ModelData & model, int input, int weight, int channel, int & mixerIndex);
    void maxMixSwitch(char *name, MixData &mix, int destCh, int sw, int weight);

};
