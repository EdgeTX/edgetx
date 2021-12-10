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

const YamlLookupTable sticksLut = {
    {0, "0"},
    {1, "1"},
    {2, "2"},
    {3, "3"},
};

const YamlLookupTable switchesLut = {
    {0, "SA"}, {1, "SB"}, {2, "SC"}, {3, "SD"}, {4, "SE"},
    {5, "SF"}, {6, "SG"}, {7, "SH"}, {8, "SI"}, {9, "SJ"},
};

const YamlLookupTable switchConfigLut = {
    {Board::SWITCH_NOT_AVAILABLE, "none"},
    {Board::SWITCH_TOGGLE, "toggle"},
    {Board::SWITCH_2POS, "2pos"},
    {Board::SWITCH_3POS, "3pos"},
};

const YamlLookupTable potsLut = {
    {0, "S1"}, {1, "6POS"}, {2, "S2"},
    {3, "EXT1"}, {4, "EXT2"}, {5, "EXT3"}, {6, "EXT4"},
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

