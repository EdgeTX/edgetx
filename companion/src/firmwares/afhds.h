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

#pragma once

#include "moduledata.h"

#include <QString>

class AfhdsData
{
  public:
    static void setDefaults(ModuleData & module);

    static QString subTypeToString(unsigned int subType);
    static QString modeToString(unsigned int mode);

    static unsigned int rxFreq(ModuleData & module);
    static void setRxFreq(ModuleData & module, unsigned int freq);

    static unsigned int mode(ModuleData & module);
    static void setMode(ModuleData & module, unsigned int mode);
};

class Afhds2aData
{
  friend class AfhdsData;

  protected:
    static void setDefaults(ModuleData::Afhds2a & afhds2a);
    static void setMode(ModuleData::Afhds2a & afhds2a, unsigned int mode);
    static void setRxFreq(ModuleData::Afhds2a & afhds2a, unsigned int freq);
    static unsigned int rxFreq(ModuleData::Afhds2a & afhds2a);
};

class Afhds3Data
{
  friend class AfhdsData;

  protected:
    static void setDefaults(ModuleData::Afhds3 & afhds3);
    static void setMode(ModuleData::Afhds3 & afhds3, unsigned int mode);
    static void setRxFreq(ModuleData::Afhds3 & afhds3, unsigned int freq);
    static unsigned int rxFreq(ModuleData::Afhds3 & afhds3);
};

