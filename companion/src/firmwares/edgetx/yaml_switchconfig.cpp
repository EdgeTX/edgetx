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

#include "yaml_switchconfig.h"

#include "boards.h"
#include "eeprominterface.h"

const YamlLookupTable switchConfigLut = {
    {Board::SWITCH_NOT_AVAILABLE, "none"},
    {Board::SWITCH_TOGGLE, "toggle"},
    {Board::SWITCH_2POS, "2pos"},
    {Board::SWITCH_3POS, "3pos"},
};


const YamlLookupTable potConfigLut = {
    {Board::POT_NONE, "none"},
    {Board::POT_WITH_DETENT, "with_detent"},
    {Board::POT_MULTIPOS_SWITCH, "multipos_switch"},
    {Board::POT_WITHOUT_DETENT, "without_detent"},
};

const YamlLookupTable slidersLut = {
    {0, "LS"},
    {1, "RS"},
};

const YamlLookupTable sliderConfigLut = {
    {Board::SLIDER_NONE, "none"},
    {Board::SLIDER_WITH_DETENT, "with_detent"},
};

int YamlStickLookup::name2idx(const std::string& name)
{
    try {
        int idx = std::stoi(name);
        if (idx < 4) {
            return idx;
        }
    } catch(...) {}

    return -1;
}

std::string YamlStickLookup::idx2name(unsigned int idx)
{
    if (idx < 4) {
        return std::to_string(idx);
    }

    return std::string();
}

int YamlSwitchLookup::name2idx(const std::string& name)
{
    auto fw = getCurrentFirmware();
    int idx = fw->getSwitchesIndex(name.c_str());
    if (idx < 0) return -1;

    return idx;
}

std::string YamlSwitchLookup::idx2name(unsigned int idx)
{
    auto fw = getCurrentFirmware();
    unsigned int switches = Boards::getCapability(fw->getBoard(), Board::Switches);
    if (idx >= switches) return std::string();

    return fw->getSwitchesTag(idx);
}

int YamlPotLookup::name2idx(const std::string& name)
{
    auto fw = getCurrentFirmware();
    int idx = fw->getAnalogInputIndex(name.c_str());
    if (idx < 0) return idx;

    int sticks = Boards::getCapability(fw->getBoard(), Board::Sticks);
    if (idx < sticks) return -1;
    idx -= sticks;

    int pots = Boards::getCapability(fw->getBoard(), Board::Pots);
    if (idx >= pots) return -1;

    return idx;
}

std::string YamlPotLookup::idx2name(unsigned int idx)
{
    auto fw = getCurrentFirmware();
    unsigned int pots = Boards::getCapability(fw->getBoard(), Board::Pots);
    if (idx >= pots) return std::string();

    unsigned int sticks = Boards::getCapability(fw->getBoard(), Board::Sticks);
    return fw->getAnalogInputTag(idx + sticks);
}

int YamlSliderLookup::name2idx(const std::string& name)
{
    auto fw = getCurrentFirmware();
    int idx = fw->getAnalogInputIndex(name.c_str());
    if (idx < 0) return idx;

    int sticks = Boards::getCapability(fw->getBoard(), Board::Sticks);
    if (idx < sticks) return -1;
    idx -= sticks;

    int pots = Boards::getCapability(fw->getBoard(), Board::Pots);
    if (idx < pots) return -1;
    idx -= pots;

    int sliders = Boards::getCapability(fw->getBoard(), Board::Sliders);
    if (idx >= sliders) return -1;

    return idx;
}

std::string YamlSliderLookup::idx2name(unsigned int idx)
{
    auto fw = getCurrentFirmware();
    unsigned int sliders = Boards::getCapability(fw->getBoard(), Board::Sliders);
    if (idx >= sliders) return std::string();

    unsigned int sticks = Boards::getCapability(fw->getBoard(), Board::Sticks);
    unsigned int pots = Boards::getCapability(fw->getBoard(), Board::Pots);
    return fw->getAnalogInputTag(idx + pots + sticks);
}
