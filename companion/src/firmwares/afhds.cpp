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

#include "afhds.h"
#include "macros.h"

// static
QString AfhdsData::subTypeToString(unsigned int subType)
{
  const char * const subTypes[] = {
    "AFHDS3",
    "AFHDS2A"
  };

  return CHECK_IN_ARRAY(subTypes, subType);
}

// static
QString AfhdsData::modeToString(unsigned int mode)
{
  const char * const modes[] = {
    "PWM/IBUS",
    "PWM/SBUS",
    "PPM/IBUS",
    "PPM/SBUS",
  };

  return CHECK_IN_ARRAY(modes, mode);
}

// static
unsigned int AfhdsData::rxFreq(ModuleData & module)
{
  return (module.protocol == PULSES_FLYSKY_AFHDS2A ? Afhds2aData::rxFreq(module.afhds2a) : Afhds3Data::rxFreq(module.afhds3));
}

// static
void AfhdsData::setRxFreq(ModuleData & module, unsigned int value)
{
  module.protocol == PULSES_FLYSKY_AFHDS2A ? Afhds2aData::setRxFreq(module.afhds2a, value) : Afhds3Data::setRxFreq(module.afhds3, value);
}

// static
unsigned int AfhdsData::mode(ModuleData & module)
{
  return (module.protocol == PULSES_FLYSKY_AFHDS2A ? module.afhds2a.mode : module.afhds3.mode);
}

// static
void AfhdsData::setMode(ModuleData & module, unsigned int value)
{
  module.protocol == PULSES_FLYSKY_AFHDS2A ? Afhds2aData::setMode(module.afhds2a, value) : Afhds3Data::setMode(module.afhds3, value);
}

// static
void AfhdsData::setDefaults(ModuleData & module)
{
  module.rfProtocol = 0;
  module.channelsCount = 14 - 8;
  module.failsafeMode = 1;

  if (module.protocol == PULSES_FLYSKY_AFHDS2A) {
    module.subType = MODULE_SUBTYPE_FLYSKY_AFHDS2A;
    Afhds2aData::setDefaults(module.afhds2a);
  }
  else if (module.protocol == PULSES_FLYSKY_AFHDS3){
    module.subType = MODULE_SUBTYPE_FLYSKY_AFHDS3;
    Afhds3Data::setDefaults(module.afhds3);
  }
}

/*
    Afhds2aData
*/

// static
void Afhds2aData::setDefaults(ModuleData::Afhds2a & afhds2a)
{
  afhds2a.mode = 3;
  afhds2a.rx_freq[0] = 50;
  afhds2a.rx_freq[1] = 0;
  afhds2a.rx_id[0] = afhds2a.rx_id[1] = afhds2a.rx_id[2] = afhds2a.rx_id[3] = 0;
  afhds2a.rfPower = 0;
}

// static
void Afhds2aData::setMode(ModuleData::Afhds2a & afhds2a, unsigned int mode)
{
  afhds2a.mode = mode;
}

// static
void Afhds2aData::setRxFreq(ModuleData::Afhds2a & afhds2a, unsigned int freq)
{
  afhds2a.rx_freq[0] = freq & 0xFF;
  afhds2a.rx_freq[1] = freq >> 8;
}

// static
unsigned int Afhds2aData::rxFreq(ModuleData::Afhds2a & afhds2a)
{
  return afhds2a.rx_freq[0] | (afhds2a.rx_freq[1] << 8);
}

/*
    Afhds3Data
*/

// static
void Afhds3Data::setDefaults(ModuleData::Afhds3 & afhds3)
{
  afhds3.mode = 3;
  afhds3.rx_freq[0] = 50;
  afhds3.rx_freq[1] = 0;
  afhds3.bindPower = 0;
  afhds3.runPower = 0;
  afhds3.emi = 0;
  afhds3.telemetry = 1;
  afhds3.failsafeTimeout = 1000;
}

// static
void Afhds3Data::setMode(ModuleData::Afhds3 & afhds3, unsigned int mode)
{
  afhds3.mode = mode;
}

// static
void Afhds3Data::setRxFreq(ModuleData::Afhds3 & afhds3, unsigned int freq)
{
  afhds3.rx_freq[0] = freq & 0xFF;
  afhds3.rx_freq[1] = freq >> 8;
}

// static
unsigned int Afhds3Data::rxFreq(ModuleData::Afhds3 & afhds3)
{
  return afhds3.rx_freq[0] | (afhds3.rx_freq[1] << 8);
}
