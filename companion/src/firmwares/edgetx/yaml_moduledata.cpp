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

#include "yaml_moduledata.h"
#include "yaml_generalsettings.h"
#include "eeprominterface.h"
#include "moduledata.h"
#include "rawsource.h"

//  type: TYPE_MULTIMODULE
//  subType: 15,2
//  channelsStart: 0
//  channelsCount: 12
//  failsafeMode: NOT_SET
//  mod:
//     multi:
//        disableTelemetry: 0
//        disableMapping: 0
//        autoBindMode: 0
//        lowPowerMode: 0
//        optionValue: 0
//        receiverTelemetryOff: 0
//        receiverHigherChannels: 0

//  type: TYPE_GHOST
//  subType: 0
//  channelsStart: 0
//  channelsCount: 16
//  failsafeMode: NOT_SET
//  mod:
//      ghost:
//        raw12bits: 0

static const YamlLookupTable protocolLut = {
  {  PULSES_OFF, "TYPE_NONE"  },
  {  PULSES_PPM, "TYPE_PPM"  },
  {  PULSES_PXX_XJT_X16, "TYPE_XJT_PXX1"  },
  {  PULSES_ACCESS_ISRM, "TYPE_ISRM_PXX2"  },
  {  PULSES_LP45, "TYPE_DSM2"  },
  {  PULSES_CROSSFIRE, "TYPE_CROSSFIRE"  },
  {  PULSES_MULTIMODULE, "TYPE_MULTIMODULE"  },
  {  PULSES_PXX_R9M, "TYPE_R9M_PXX1"  },
  {  PULSES_ACCESS_R9M, "TYPE_R9M_PXX2"  },
  {  PULSES_PXX_R9M_LITE, "TYPE_R9M_LITE_PXX1"  },
  {  PULSES_ACCESS_R9M_LITE, "TYPE_R9M_LITE_PXX2"  },
  {  PULSES_GHOST, "TYPE_GHOST"  },
  {  PULSES_ACCESS_R9M_LITE_PRO, "TYPE_R9M_LITE_PRO_PXX2"  },
  {  PULSES_SBUS, "TYPE_SBUS"  },
  {  PULSES_XJT_LITE_X16, "TYPE_XJT_LITE_PXX2"  },
  {  PULSES_AFHDS3, "TYPE_FLYSKY"  },
  {  PULSES_LEMON_DSMP, "TYPE_LEMON_DSMP"  },
};

static const YamlLookupTable xjtLut = {
  { 0, "D16" },
  { 1, "D8" },
  { 2, "LR12" },
};

static const YamlLookupTable isrmLut = {
  { 0, "ACCESS" },
  { 1, "D16" },
};

static const YamlLookupTable r9mLut = {
  { MODULE_SUBTYPE_R9M_FCC, "FCC" },
  { MODULE_SUBTYPE_R9M_EU, "EU" },
  { MODULE_SUBTYPE_R9M_EUPLUS, "EUPLUS" },
  { MODULE_SUBTYPE_R9M_AUPLUS, "AUPLUS" },
};

static const YamlLookupTable dsmLut = {
  { 0, "LP45" },
  { 1, "DSM2" },
  { 2, "DSMX" },
};

static const YamlLookupTable failsafeLut = {
  {  FAILSAFE_NOT_SET, "NOT_SET"  },
  {  FAILSAFE_HOLD, "HOLD"  },
  {  FAILSAFE_CUSTOM, "CUSTOM"  },
  {  FAILSAFE_NOPULSES, "NOPULSES"  },
  {  FAILSAFE_RECEIVER, "RECEIVER"  },
};

enum MMRFrskySubtypes {
  MM_RF_FRSKY_SUBTYPE_D16,
  MM_RF_FRSKY_SUBTYPE_D8,
  MM_RF_FRSKY_SUBTYPE_D16_8CH,
  MM_RF_FRSKY_SUBTYPE_V8,
  MM_RF_FRSKY_SUBTYPE_D16_LBT,
  MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH,
  MM_RF_FRSKY_SUBTYPE_D8_CLONED,
  MM_RF_FRSKY_SUBTYPE_D16_CLONED
};

// from radio/src/pulses/multi.cpp
static void convertMultiProtocolToEtx(int *protocol, int *subprotocol)
{
  if (*protocol == 3 && *subprotocol == 0) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_D8;
    return;
  }

  if (*protocol == 3 && *subprotocol == 1) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_D8_CLONED;
    return;
  }

  if (*protocol == 25) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_V8;
    return;
  }

  if (*protocol == 15) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;

    if (*subprotocol == 0)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16;
    else if (*subprotocol == 1)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_8CH;
    else if (*subprotocol == 2)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_LBT;
    else if (*subprotocol == 3)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH;
    else if (*subprotocol == 4)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_CLONED;

    return;
  }

  if (*protocol >= 25)
    *protocol -= 1;

  if (*protocol >= 16)
    *protocol -= 1;
}

void convertEtxProtocolToMulti(int *protocol, int *subprotocol)
{
  // Special treatment for the FrSky entry...
  if (*protocol == MODULE_SUBTYPE_MULTI_FRSKY + 1) {
    if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D8) {
      //D8
      *protocol = 3;
      *subprotocol = 0;
    }
    else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D8_CLONED) {
      //D8
      *protocol = 3;
      *subprotocol = 1;
    }
    else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_V8) {
      //V8
      *protocol = 25;
      *subprotocol = 0;
    }
    else {
      *protocol = 15;
      if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_8CH)
        *subprotocol = 1;
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16)
        *subprotocol = 0; // D16
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_LBT)
        *subprotocol = 2;
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH)
        *subprotocol = 3;
      else
        *subprotocol = 4; // D16_CLONED
    }
  }
  else {
    // 15  for Multimodule is FrskyX or D16 which we map as a protocol of 3 (FrSky)
    // all protos > frskyx are therefore also off by one
    if (*protocol >= 15)
      *protocol += 1;
    // 25 is again a FrSky *protocol (FrskyV) so shift again
    if (*protocol >= 25)
      *protocol += 1;
  }
}

static int exportPpmDelay(int delay) { return (delay - 300) / 50; }
static int importPpmDelay(int delay) { return 300 + 50 * delay; }

namespace YAML
{
Node convert<ModuleData>::encode(const ModuleData& rhs)
{
  Node node;
  int protocol = rhs.protocol;
  int subtype = rhs.subType;
  if (rhs.protocol >= PULSES_LP45 && rhs.protocol <= PULSES_DSMX) {
      protocol = PULSES_LP45;
      subtype = rhs.protocol - PULSES_LP45;
  } else if (rhs.protocol == PULSES_ACCST_ISRM_D16 || rhs.protocol == PULSES_ACCESS_ISRM) {
      protocol = PULSES_ACCESS_ISRM;
      subtype = rhs.protocol - PULSES_ACCESS_ISRM;
  } else if (rhs.protocol >= PULSES_PXX_XJT_X16 && rhs.protocol <= PULSES_PXX_XJT_LR12) {
      protocol = PULSES_PXX_XJT_X16;
      subtype = rhs.protocol - PULSES_PXX_XJT_X16;
  } else if (rhs.protocol >= PULSES_XJT_LITE_X16 && rhs.protocol <= PULSES_XJT_LITE_LR12) {
      protocol = PULSES_XJT_LITE_X16;
      subtype = rhs.protocol - PULSES_XJT_LITE_X16;
  }
  node["type"] = LookupValue(protocolLut, protocol);

  switch (protocol) {
    case PULSES_PXX_XJT_X16:
    case PULSES_XJT_LITE_X16:
      node["subType"] = LookupValue(xjtLut, subtype);
      break;
    case PULSES_ACCESS_ISRM:
      node["subType"] = LookupValue(isrmLut, subtype);
      break;
    case PULSES_PXX_R9M:
    case PULSES_PXX_R9M_LITE:
      node["subType"] = LookupValue(r9mLut, subtype);
      break;
    case PULSES_LP45:
      node["subType"] = LookupValue(dsmLut, subtype);
      break;
    case PULSES_MULTIMODULE: {
      int rfProtocol = rhs.multi.rfProtocol + 1;
      int subType = rhs.subType;
      convertEtxProtocolToMulti(&rfProtocol, &subType);
      std::string st_str = std::to_string(rfProtocol);
      st_str += ",";
      st_str += std::to_string(subType);
      node["subType"] = st_str;
    } break;
  }

  node["channelsStart"] = rhs.channelsStart;
  node["channelsCount"] = rhs.channelsCount;
  node["failsafeMode"] = LookupValue(failsafeLut, rhs.failsafeMode);

  Node mod;
  switch (protocol) {
    case PULSES_PXX_XJT_X16:
    case PULSES_PXX_R9M:
    case PULSES_PXX_R9M_LITE: {
        Node pxx;
        pxx["power"] = rhs.pxx.power;
        // pxx["receiverTelemetryOff"] = rhs.pxx.receiverTelemetryOff;
        // pxx["receiverHigherChannels"] = rhs.pxx.receiverHigherChannels;
        pxx["antennaMode"] = rhs.pxx.antennaMode;
        mod["pxx"] = pxx;
    } break;
    case PULSES_ACCESS_ISRM:
    case PULSES_ACCESS_R9M:
    case PULSES_ACCESS_R9M_LITE:
    case PULSES_ACCESS_R9M_LITE_PRO:
    case PULSES_XJT_LITE_X16: {
        Node pxx2;
        pxx2["receivers"] = rhs.access.receivers;
        pxx2["racingMode"] = rhs.access.racingMode;
        for (int i=0; i < PXX2_MAX_RECEIVERS_PER_MODULE; i++) {
          if (rhs.access.receivers & (1 << i)) {
            pxx2["receiverName"][std::to_string(i)]["val"] = rhs.access.receiverName[i];
          }
        }
        mod["pxx2"] = pxx2;
    } break;
    case PULSES_MULTIMODULE: {
        Node multi;;
        multi["disableTelemetry"] = (int)rhs.multi.disableTelemetry;
        multi["disableMapping"] = (int)rhs.multi.disableMapping;
        multi["autoBindMode"] = (int)rhs.multi.autoBindMode;
        multi["lowPowerMode"] = (int)rhs.multi.lowPowerMode;
        multi["optionValue"] = rhs.multi.optionValue;
        // multi["receiverTelemetryOff"] = (int)rhs.pxx.receiverTelemetryOff;
        // multi["receiverHigherChannels"] = (int)rhs.pxx.receiverHigherChannels;
        mod["multi"] = multi;
    } break;
    case PULSES_SBUS: {
        Node sbus;
        sbus["refreshRate"] = rhs.ppm.frameLength;
        mod["sbus"] = sbus;
    } break;
    case PULSES_GHOST: {
        Node ghost;
        ghost["raw12bits"] = (int)rhs.ghost.raw12bits;
        YamlTelemetryBaudrate br(&rhs.ghost.telemetryBaudrate);
        ghost["telemetryBaudrate"] = br.value;
        mod["ghost"] = ghost;
    } break;
    case PULSES_CROSSFIRE: {
        Node crsf;
        YamlTelemetryBaudrate br(&rhs.crsf.telemetryBaudrate);
        crsf["telemetryBaudrate"] = br.value;
        mod["crsf"] = crsf;
    } break;
    case PULSES_LEMON_DSMP: {
        Node dsmp;
        dsmp["flags"] = rhs.dsmp.flags;
        mod["dsmp"] = dsmp;
    } break;
    // TODO: afhds3, flysky
    default: {
        Node ppm;
        ppm["delay"] = exportPpmDelay(rhs.ppm.delay);
        ppm["pulsePol"] = (int)rhs.ppm.pulsePol;
        ppm["outputType"] = (int)rhs.ppm.outputType;
        ppm["frameLength"] = rhs.ppm.frameLength;
        mod["ppm"] = ppm;
    } break;
  }
  if (mod && mod.IsMap())
      node["mod"] = mod;

  return node;
}

bool convert<ModuleData>::decode(const Node& node, ModuleData& rhs)
{
  node["type"] >> protocolLut >> rhs.protocol;

  Node subType;
  node["subType"] >> subType;

  switch (rhs.protocol) {
    case PULSES_PXX_XJT_X16:
    case PULSES_XJT_LITE_X16: {
      int subProto = 0;
      subType >> xjtLut >> subProto;
      rhs.protocol += subProto;
    } break;
    case PULSES_ACCESS_ISRM: {
      int subProto = 0;
      subType >> isrmLut >> subProto;
      rhs.protocol += subProto;
    } break;
    case PULSES_PXX_R9M:
    case PULSES_PXX_R9M_LITE: {
      subType >> r9mLut >> rhs.subType;
    } break;
    case PULSES_LP45: {
      int subProto = 0;
      subType >> dsmLut >> subProto;
      rhs.protocol += subProto;
    } break;
    case PULSES_MULTIMODULE: {
      std::string st_str;
      subType >> st_str;
      if (!st_str.empty()) {
        size_t pos = 0;
        try {
          int rfProtocol = std::stoi(st_str, &pos);
          st_str = st_str.substr(pos + 1);
          int rfSubType = std::stoi(st_str);
          convertMultiProtocolToEtx(&rfProtocol, &rfSubType);
          if (rfProtocol > 0) {
            rhs.multi.rfProtocol = rfProtocol - 1;
            rhs.subType = rfSubType;
          }
        } catch(...) {}
      }
    } break;
  }

  node["channelsStart"] >> rhs.channelsStart;
  node["channelsCount"] >> rhs.channelsCount;
  node["failsafeMode"] >> failsafeLut >> rhs.failsafeMode;

  if (node["mod"]) {
      const Node& mod = node["mod"];
      if (mod["ppm"]) {
          const Node& ppm = mod["ppm"];
          ppm["delay"] >> rhs.ppm.delay;
          rhs.ppm.delay = importPpmDelay(rhs.ppm.delay);
          ppm["pulsePol"] >> rhs.ppm.pulsePol;
          ppm["outputType"] >> rhs.ppm.outputType;
          ppm["frameLength"] >> rhs.ppm.frameLength;
      } else if (mod["multi"]) {
          Node multi = mod["multi"];
          multi["disableTelemetry"] >> rhs.multi.disableTelemetry;
          multi["disableMapping"] >> rhs.multi.disableMapping;
          multi["autoBindMode"] >> rhs.multi.autoBindMode;
          multi["lowPowerMode"] >> rhs.multi.lowPowerMode;
          multi["optionValue"] >> rhs.multi.optionValue;
          // multi["receiverTelemetryOff"] >> rhs.pxx.receiverTelemetryOff;
          // multi["receiverHigherChannels"] >> rhs.pxx.receiverHigherChannels;
      } else if (mod["pxx"]) {
          Node pxx = mod["pxx"];
          pxx["power"] >> rhs.pxx.power;
          // pxx["receiverTelemetryOff"] >> rhs.pxx.receiverTelemetryOff;
          // pxx["receiverHigherChannels"] >> rhs.pxx.receiverHigherChannels;
          pxx["antennaMode"] >> rhs.pxx.antennaMode;
      } else if (mod["sbus"]) {
          mod["sbus"]["refreshRate"] >> rhs.ppm.frameLength;
      } else if (mod["pxx2"]) {
          Node pxx2 = mod["pxx2"];
          pxx2["receivers"] >> rhs.access.receivers;
          pxx2["racingMode"] >> rhs.access.racingMode;
          for (int i=0; i < PXX2_MAX_RECEIVERS_PER_MODULE; i++) {
            if (pxx2["receiverName"][std::to_string(i)]) {
              Node rcvrname = pxx2["receiverName"][std::to_string(i)];
              if (rcvrname.IsMap()) {
                if (rcvrname["val"]) {
                  rcvrname["val"] >> rhs.access.receiverName[i];
                }
              }
            }
          }
      } else if (mod["ghost"]) {
          Node ghost = mod["ghost"];
          YamlTelemetryBaudrate telemetryBaudrate;
          ghost["telemetryBaudrate"] >> telemetryBaudrate.value;
          telemetryBaudrate.toCpn(&rhs.ghost.telemetryBaudrate, getCurrentFirmware()->getBoard());
          ghost["raw12bits"] >> rhs.ghost.raw12bits;
      } else if (mod["crsf"]) {
          Node crsf = mod["crsf"];
          YamlTelemetryBaudrate telemetryBaudrate;
          crsf["telemetryBaudrate"] >> telemetryBaudrate.value;
          telemetryBaudrate.toCpn(&rhs.crsf.telemetryBaudrate, getCurrentFirmware()->getBoard());
      } else if (mod["dsmp"]) {
          Node dsmp = mod["dsmp"];
          dsmp["flags"] >> rhs.dsmp.flags;
      } else if (mod["flysky"]) {
          //TODO
      } else if (mod["afhds3"]) {
          //TODO
      }
  }

  return true;
}
}
