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

#include "yaml_customfunctiondata.h"
#include "yaml_rawswitch.h"
#include "yaml_rawsource.h"
#include "eeprominterface.h"

static bool fnHasRepeat(AssignFunc fn)
{
  return (fn == FuncPlayPrompt)
    || (fn == FuncPlayValue)
    || (fn == FuncPlayHaptic)
    || (fn == FuncPlaySound)
    || (fn == FuncSetScreen)
    || (fn == FuncPlayScript)
    || (fn == FuncRGBLed);
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
  {  FuncSetScreen, "SET_SCREEN"},
  {  FuncDisableAudioAmp, "DISABLE_AUDIO_AMP"  },
  {  FuncRGBLed, "RGB_LED"  },
  {  FuncLCDtoVideo, "LCD_TO_VIDEO"  },
  {  FuncPushCustomSwitch1, "PUSH_CUST_SWITCH"  },
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
  { 4, "Wrn2" },
  { 5, "Chee" },
  { 6, "Rata" },
  { 7, "Tick" },
  { 8, "Sirn" },
  { 9, "Ring" },
  { 10, "SciF" },
  { 11, "Robt" },
  { 12, "Chrp" },
  { 13, "Tada" },
  { 14, "Crck" },
  { 15, "Alrm" },
};

static const YamlLookupTable resetLut = {
  { 0, "Tmr1" },
  { 1, "Tmr2" },
  { 2, "Tmr3" },
  { 3, "All" },
  { 4, "Tele" },
  { 5, "Trims" },
};

static const YamlLookupTable gvarModeLut = {
  { FUNC_ADJUST_GVAR_CONSTANT, "Cst" },
  { FUNC_ADJUST_GVAR_SOURCE, "Src" },
  { FUNC_ADJUST_GVAR_SOURCERAW, "SrcRaw" },
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

  if (fn >= FuncOverrideCH1 && fn <= FuncOverrideCHLast) {
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
  } else if (fn >= FuncPushCustomSwitch1 && fn <= FuncPushCustomSwitchLast) {
    p1 = fn - (int)FuncPushCustomSwitch1;
    fn = (int)FuncPushCustomSwitch1;
  }

  node["func"] = LookupValue(customFnLut, fn);

  bool add_comma = true;
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
    def += LookupValue(soundLut, rhs.param);
    break;
  case FuncPlayHaptic:
    def += std::to_string(rhs.param);
    break;
  case FuncPlayPrompt:
  case FuncBackgroundMusic: {
    std::string temp(rhs.paramarm);
    temp.resize(getCurrentFirmware()->getCapability(VoicesMaxLength));
    def += temp;
  } break;
  case FuncPlayScript:
  case FuncRGBLed:
    def += std::string(rhs.paramarm);
    break;
  case FuncReset:
    def += LookupValue(resetLut, rhs.param);
    if (def.empty()) {
      def += std::to_string(rhs.param - resetLut.size());
    }
    break;
  case FuncSetTimer1: {
    def += LookupValue(resetLut, p1);
    def += ",";
    def += std::to_string(rhs.param);
  } break;
  case FuncPlayValue:
  case FuncVolume:
  case FuncBacklight: {
    def += YamlRawSourceEncode(RawSource(rhs.param));
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
    case FUNC_ADJUST_GVAR_SOURCE:
    case FUNC_ADJUST_GVAR_SOURCERAW: {
      def += YamlRawSourceEncode(RawSource(rhs.param));
    } break;
    }
  } break;
  case FuncRangeCheckInternalModule:
  case FuncBindInternalModule: {
    def += LookupValue(moduleLut, p1);
  } break;
  case FuncLogs:
    def += std::to_string(rhs.param);
    break;
  case FuncSetScreen:
    def += std::to_string(rhs.param);
    break;
  case FuncPushCustomSwitch1:
    def += std::to_string(p1);
    def += ",";
    def += std::to_string(rhs.param);
    break;
  default:
    add_comma = false;
    break;
  }

  if (add_comma) {
    def += ",";
  }

  def += std::to_string((int)rhs.enabled);

  if(fnHasRepeat(rhs.func)) {
    def += ",";

    if (rhs.func == FuncPlayScript || rhs.func == FuncRGBLed) {
      def += ((rhs.repeatParam == 0) ? "On" : "1x");
    } else if (rhs.repeatParam == 0) {
      def += "1x";
    } else if (rhs.repeatParam == -1) {
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
  node["func"] >> customFnLut >> func;
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
    std::string value_str;
    getline(def, value_str, ',');
    int value=0;
    Node(value_str) >> trainerLut >> value;
    rhs.func = (AssignFunc)((int)rhs.func + value);
  } break;
  case FuncPlaySound: {
    std::string snd;
    getline(def, snd, ',');
    Node(snd) >> soundLut >> rhs.param;
  } break;
  case FuncPlayHaptic: {
    std::string haptic;
    getline(def, haptic, ',');
    try {
      rhs.param = std::stoi(haptic);
    } catch(...) {}
  } break;
  case FuncPlayPrompt:
  case FuncBackgroundMusic: {
    std::string file_str;
    getline(def, file_str, ',');
    file_str.resize(getCurrentFirmware()->getCapability(VoicesMaxLength));
    strncpy(rhs.paramarm, file_str.c_str(), sizeof(rhs.paramarm) - 1);
    } break;
  case FuncRGBLed:
  case FuncPlayScript: {
    std::string file_str;
    getline(def, file_str, ',');
    strncpy(rhs.paramarm, file_str.c_str(), sizeof(rhs.paramarm) - 1);
    } break;
  case FuncReset: {
    std::string rst_str;
    getline(def, rst_str, ',');
    try {
      rhs.param = std::stoi(rst_str);
      rhs.param += resetLut.size();
      break;
    } catch(...) {}
    Node(rst_str.c_str()) >> resetLut >> rhs.param;
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
  case FuncPlayValue:
  case FuncVolume:
  case FuncBacklight: {
    std::string src_str;
    getline(def, src_str, ',');
    if (def_str.size() >= 4 && def_str.substr(0, 4) == "lua(") {
      std::string tmp_str;
      getline(def, tmp_str, ',');
      src_str += ("," + tmp_str);
    }
    rhs.param = YamlRawSourceDecode(src_str).toValue();
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
    case FUNC_ADJUST_GVAR_SOURCE:
    case FUNC_ADJUST_GVAR_SOURCERAW: {
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
  case FuncLogs: {
    int param = 0;
    def >> param;
    rhs.param = param;
  } break;
  case FuncSetScreen: {
    int param = 0;
    def >> param;
    rhs.param = param;
  } break;
  case FuncPushCustomSwitch1: {
    int sw = 0;
    def >> sw;
    rhs.func = (AssignFunc)((int)rhs.func + sw);
    def.ignore();
    int param = 0;
    def >> param;
    rhs.param = param;
  } break;
  default:
    break;
  }

  if (def.peek() == ',') {
    def.ignore();
  }

  // Need to handle older YAML files where only one of enabled/repeat was present
  std::string en, repeat;
  getline(def, en, ',');
  getline(def, repeat);

  if (repeat.empty()) {
    // Only one value left to parse
    if (fnHasRepeat(rhs.func)) {
      // Assume it is repeat and set enabled to true
      repeat = en;
      rhs.enabled = 1;
    } else {
      // Func does not have repeat
      rhs.enabled = en[0] == '1' ? 1 : 0;
    }
  } else {
    // Two values - first is 'enabled' flag
    rhs.enabled = en[0] == '1' ? 1 : 0;
  }

  if(fnHasRepeat(rhs.func)) {
    if (rhs.func == FuncPlayScript || rhs.func == FuncRGBLed) {
      rhs.repeatParam = (repeat == "1x") ? 1 : 0;
    } else if (repeat == "1x") {
      rhs.repeatParam = 0;
    } else if (repeat == "!1x") {
      rhs.repeatParam = -1;
    } else {
      try {
        rhs.repeatParam = std::stoi(repeat);
      } catch(...) {}
    }
  }

  return true;
}
}  // namespace YAML
