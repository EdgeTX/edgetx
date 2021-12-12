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

#include "yaml_customfunctiondata.h"
#include "yaml_rawswitch.h"
#include "yaml_rawsource.h"

static bool fnHasEnable(AssignFunc fn)
{
  return (fn <= FuncInstantTrim)
    || (fn >= FuncReset && fn <= FuncSetTimerLast)
    || (fn >= FuncAdjustGV1 && fn <= FuncBindExternalModule)
    || (fn == FuncVolume);
}

static bool fnHasRepeat(AssignFunc fn)
{
  return (fn == FuncPlayPrompt)
    || (fn == FuncPlayValue)
    || (fn == FuncPlayHaptic)
    || (fn == FuncPlaySound);
}

static const YamlLookupTable customFnLut = {
  {  FuncOverrideCH1, "OVERRIDE_CHANNEL"  },
  {  FuncTrainer, "TRAINER"  },
  {  FuncInstantTrim, "INSTANT_TRIM"  },
  {  FuncReset, "RESET"  },
  {  FuncSetTimer1, "SET_TIMER"  },
  {  FuncAdjustGV1, "ADJUST_GVAR"  },
  {  FuncVolume, "VOLUME"  },
  {  FuncSetFailsafe, "SET_FAILSAFE"  },
  {  FuncRangeCheckInternalModule, "RANGECHECK"  },
  {  FuncBindInternalModule, "BIND"  },
  {  FuncPlaySound, "PLAY_SOUND"  },
  {  FuncPlayPrompt, "PLAY_TRACK"  },
  {  FuncPlayValue, "PLAY_VALUE"  },
  {  FuncPlayScript, "PLAY_SCRIPT"  },
  {  FuncBackgroundMusic, "BACKGND_MUSIC"  },
  {  FuncBackgroundMusicPause, "BACKGND_MUSIC_PAUSE"  },
  {  FuncVario, "VARIO"  },
  {  FuncPlayHaptic, "HAPTIC"  },
  {  FuncLogs, "LOGS"  },
  {  FuncBacklight, "BACKLIGHT"  },
  {  FuncScreenshot, "SCREENSHOT"  },
  {  FuncRacingMode, "RACING_MODE"  },
  {  FuncDisableTouch, "DISABLE_TOUCH"  },    
};

static const YamlLookupTable trainerLut = {
    { 0, "sticks" },
    { 1, "Rud" },
    { 2, "Ele" },
    { 3, "Thr" },
    { 4, "Ail" },
    { 5, "chans" },
};

static const YamlLookupTable soundLut = {
  { 0, "Bp1" },
  { 1, "Bp2" },
  { 2, "Bp3" },
  { 3, "Wrn1" },
  { 5, "Wrn2" },
  { 6, "Chee" },
  { 7, "Rata" },
  { 8, "Tick" },
  { 9, "Sirn" },
  { 10, "Ring" },
  { 11, "SciF" },
  { 12, "Robt" },
  { 13, "Chrp" },
  { 14, "Tada" },
  { 15, "Crck" },
  { 16, "Alrm" },
};

static const YamlLookupTable resetLut = {
  { 0, "Tmr1" },
  { 1, "Tmr2" },
  { 2, "Tmr3" },
  { 3, "All" },
  { 5, "Tele" },
};

static const YamlLookupTable gvarModeLut = {
  { FUNC_ADJUST_GVAR_CONSTANT, "Cst" },
  { FUNC_ADJUST_GVAR_SOURCE, "Src" },
  { FUNC_ADJUST_GVAR_GVAR, "GVar" },
  { FUNC_ADJUST_GVAR_INCDEC, "IncDec" },
};

static const YamlLookupTable moduleLut = {
  { 0, "Int" },
  { 1, "Ext" },
};

namespace YAML
{
Node convert<CustomFunctionData>::encode(const CustomFunctionData& rhs)
{
  Node node;
  node["swtch"] = rhs.swtch;

  int fn = rhs.func;
  int p1 = 0;
  if(fn >= FuncOverrideCH1 && fn <= FuncOverrideCHLast) {
    p1 = fn - (int)FuncOverrideCH1;
    fn = (int)FuncOverrideCH1;
  } else if (fn >= FuncTrainer && fn <= FuncTrainerChannels) {
    p1 = fn - (int)FuncTrainer;
    fn = (int)FuncTrainer;
  } else if (fn >= FuncSetTimer1 && fn <= FuncSetTimerLast) {
    p1 = fn - (int)FuncSetTimer1;
    fn = (int)FuncSetTimer1;
  } else if (fn >= FuncAdjustGV1 && fn <= FuncAdjustGVLast) {
    p1 = fn - (int)FuncAdjustGV1;
    fn = (int)FuncAdjustGV1;
  } else if (fn >= FuncRangeCheckInternalModule && fn <= FuncRangeCheckExternalModule) {
    p1 = fn - (int)FuncRangeCheckInternalModule;
    fn = (int)FuncRangeCheckInternalModule;
  } else if (fn >= FuncBindInternalModule && fn <= FuncBindExternalModule) {
    p1 = fn - (int)FuncBindInternalModule;
    fn = (int)FuncBindInternalModule;
  }
  node["func"] = LookupValue(customFnLut, fn);

  std::string def;
  switch(fn) {
  case FuncOverrideCH1:
    def += std::to_string(p1);
    def += ",";
    def += std::to_string(rhs.param);
    break;
  case FuncTrainer:
    def += LookupValue(trainerLut, p1);
    break;
  case FuncPlaySound:
    def += std::to_string(rhs.param);
    break;
  case FuncPlayPrompt:
  case FuncPlayScript:
  case FuncBackgroundMusic:
    def += std::string(rhs.paramarm);
    break;
  case FuncReset:
    def += LookupValue(resetLut, rhs.param);
    if (def.empty()) {
      def += std::to_string(rhs.param);
    }
    break;
  case FuncSetTimer1: {
    def += LookupValue(resetLut, p1);
    def += ",";
    def += std::to_string(rhs.param);
  } break;
  case FuncAdjustGV1: {
    // + GV #
    def += std::to_string(p1);
    def += ",";
    def += LookupValue(gvarModeLut, rhs.adjustMode);
    def += ",";

    switch(rhs.adjustMode) {
    case FUNC_ADJUST_GVAR_CONSTANT:
    case FUNC_ADJUST_GVAR_INCDEC:
      def += std::to_string(rhs.param);
      break;
    case FUNC_ADJUST_GVAR_GVAR:
    case FUNC_ADJUST_GVAR_SOURCE: {
      def += YamlRawSourceEncode(RawSource(rhs.param));
    } break;
    }
  } break;
  case FuncRangeCheckInternalModule:
  case FuncBindInternalModule: {
    def += LookupValue(moduleLut, p1);
  } break;
  default:
    break;
  }

  if (fnHasEnable(rhs.func)) {
    if (!def.empty()) {
      def += ",";
    }
    def += std::to_string((int)rhs.enabled);
  } else if(fnHasRepeat(rhs.func)) {
    if (!def.empty()) {
      def += ",";
    }
    if (rhs.repeatParam == 0) {
      def += "1x";
    } else if (rhs.repeatParam == 0xFF) {
      def += "!1x";
    } else {
      def += std::to_string(rhs.repeatParam);
    }
  }

  if (!def.empty()) {
    node["def"] = def;
  }
  
  return node;
}

bool convert<CustomFunctionData>::decode(const Node& node,
                                        CustomFunctionData& rhs)
{
  node["swtch"] >> rhs.swtch;

  int func = 0;
  node["func"] >> func;
  rhs.func = (AssignFunc)func;

  std::string def_str;
  node["def"] >> def_str;
  std::stringstream def(def_str);
      
  switch(rhs.func) {
  case FuncOverrideCH1: {
      int ch=0;
      def >> ch;
      rhs.func = (AssignFunc)((int)rhs.func + ch);
      def.ignore();
      def >> rhs.param;
  } break;
  case FuncTrainer: {
      int value=0;
      Node(def.str()) >> trainerLut >> value;
      rhs.func = (AssignFunc)((int)rhs.func + value);
  } break;
  case FuncPlaySound:
    Node(def.str()) >> soundLut >> rhs.param;
    break;
  case FuncPlayPrompt:
  case FuncPlayScript:
  case FuncBackgroundMusic:
    strncpy(rhs.paramarm, def.str().c_str(), sizeof(rhs.paramarm)-1);
    break;
  case FuncReset: {
    try {
      def >> rhs.param;
      rhs.param += 5; // + FUNC_RESET_PARAM_FIRST_TELEM
      break;
    } catch(...) {}
    Node(def.str()) >> resetLut >> rhs.param;
  } break;
  case FuncSetTimer1: {
    // + timer #
    std::string tmr_str;
    getline(def, tmr_str, ',');
    int tmr = 0;
    Node(tmr_str) >> resetLut >> tmr;
    rhs.func = (AssignFunc)((int)rhs.func + tmr);
    def >> rhs.param;
  } break;
  case FuncAdjustGV1: {
    // + GV #
    int gvar_idx=0;
    def >> gvar_idx;
    rhs.func = (AssignFunc)((int)rhs.func + gvar_idx);
    def.ignore();

    std::string mode;
    getline(def, mode, ',');
    Node(mode) >> gvarModeLut >> rhs.adjustMode;
    switch(rhs.adjustMode) {
    case FUNC_ADJUST_GVAR_CONSTANT:
    case FUNC_ADJUST_GVAR_INCDEC:
      def >> rhs.param;
      break;
    case FUNC_ADJUST_GVAR_GVAR:
    case FUNC_ADJUST_GVAR_SOURCE: {
      std::string src_str;
      getline(def, src_str, ',');
      RawSource src;
      Node(src_str) >> src;
      rhs.param = src.toValue();
    } break;
    }
  } break;
  case FuncRangeCheckInternalModule:
  case FuncBindInternalModule: {
    // DANGEROUS_FUNCTIONS...
    // + module # ???
    std::string mod;
    getline(def, mod, ',');
    int module=0;
    Node(mod) >> moduleLut >> module;
    rhs.func = (AssignFunc)((int)rhs.func + module);
  } break;
  default:
    break;
  }

  if (fnHasEnable(rhs.func)) {
    int en = 0;
    def >> en;
    rhs.enabled = en;
  } else if(fnHasRepeat(rhs.func)) {
    std::string repeat;
    getline(def, repeat);
    if (repeat == "1x") {
      rhs.repeatParam = 0;
    } else if (repeat == "!1x") {
      rhs.repeatParam = 0xFF;
    } else {
      rhs.repeatParam = std::stoi(repeat);
    }
  }
  
  return true;
}
}  // namespace YAML
