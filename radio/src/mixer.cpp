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

#include "edgetx.h"
#include "edgetx_types.h"
#include "timers.h"
#include "switches.h"
#include "input_mapping.h"
#include "mixes.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"

#define DELAY_POS_MARGIN   3

uint8_t s_mixer_first_run_done = false;

int8_t  virtualInputsTrims[MAX_INPUTS];
int16_t anas [MAX_INPUTS] = {0};
int16_t trims[MAX_TRIMS] = {0};
int32_t chans[MAX_OUTPUT_CHANNELS] = {0};
BeepANACenter bpanaCenter = 0;

int32_t act [MAX_MIXERS] = {0};
MixState mixState [MAX_MIXERS];

uint8_t mixWarning;

int16_t calibratedAnalogs[MAX_ANALOG_INPUTS];
int16_t channelOutputs[MAX_OUTPUT_CHANNELS] = {0};
int16_t ex_chans[MAX_OUTPUT_CHANNELS] = {0}; // Outputs (before LIMITS) of the last perMain;

#if defined(HELI)
int16_t cyc_anas[3] = {0};
#endif

// TOOD: find better home for this.
int32_t getSourceNumFieldValue(int16_t val, int16_t min, int16_t max)
{
  int32_t result;
  SourceNumVal v; v.rawValue = val;
  if (v.isSource) {
    result = getValue(v.value);
    if (abs(v.value) >= MIXSRC_FIRST_GVAR && v.value <= MIXSRC_LAST_GVAR) {
      // Mimic behviour of GET_GVAR_PREC1
      result = result * 10;
    } else {
      result = calcRESXto1000(result);
    }
  } else {
    result = v.value * 10;
  }
  return limit<int>(min * 10, result, max * 10);
}

// #define EXTENDED_EXPO
// increases range of expo curve but costs about 82 bytes flash

// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// don't know what this above should be, just confusing in my opinion,

// here is the real explanation
// actually the real formula is
/*
 f(x) = exp( ln(x) * 10^k)
 if it is 10^k or e^k or 2^k etc. just defines the max distortion of the expo curve; I think 10 is useful
 this gives values from 0 to 1 for x and output; k must be between -1 and +1
 we do not like to calculate with floating point. Therefore we rescale for x from 0 to 1024 and for k from -100 to +100
 f(x) = 1024 * ( e^( ln(x/1024) * 10^(k/100) ) )
 This would be really hard to be calculated by such a microcontroller
 Therefore Thomas Husterer compared a few usual function something like x^3, x^4*something, which look similar
 Actually the formula
 f(x) = k*x^3+x*(1-k)
 gives a similar form and should have even advantages compared to a original exp curve.
 This function again expect x from 0 to 1 and k only from 0 to 1
 Therefore rescaling is needed like before:
 f(x) = 1024* ((k/100)*(x/1024)^3 + (x/1024)*(100-k)/100)
 some mathematical tricks
 f(x) = (k*x*x*x/(1024*1024) + x*(100-k)) / 100
 for better rounding results we add the 50
 f(x) = (k*x*x*x/(1024*1024) + x*(100-k) + 50) / 100

 because we now understand the formula, we can optimize it further
 --> calc100to256(k) --> eliminates /100 by replacing with /256 which is just a simple shift right 8
 k is now between 0 and 256
 f(x) = (k*x*x*x/(1024*1024) + x*(256-k) + 128) / 256
 */

// input parameters;
//  x 0 to 1024;
//  k 0 to 100;
// output between 0 and 1024
unsigned int expou(unsigned int x, unsigned int k)
{
#if defined(EXTENDED_EXPO)
  bool extended;
  if (k > 80) {
    extended=true;
  }
  else {
    k += (k>>2);  // use bigger values before extend, because the effect is anyway very very low
    extended=false;
  }
#endif

  k = calc100to256(k);

  uint32_t value = (uint32_t) x*x;
  value *= (uint32_t)k;
  value >>= 8;
  value *= (uint32_t)x;

#if defined(EXTENDED_EXPO)
  if (extended) {  // for higher values do more multiplications to get a stronger expo curve
    value >>= 16;
    value *= (uint32_t)x;
    value >>= 4;
    value *= (uint32_t)x;
  }
#endif

  value >>= 12;
  value += (uint32_t)(256-k) * x + 128;

  return value >> 8;
}

int expo(int x, int k)
{
  if (k == 0) {
    return x;
  }

  int y;
  bool neg = (x < 0);

  if (neg) {
    x = -x;
  }
  if (x > (int)RESXu) {
    x = RESXu;
  }
  if (k < 0) {
    y = RESXu - expou(RESXu-x, -k);
  }
  else {
    y = expou(x, k);
  }
  return neg ? -y : y;
}

void applyExpos(int16_t * anas, uint8_t mode, int16_t ovwrIdx, int16_t ovwrValue)
{
  int8_t cur_chn = -1;

  for (uint8_t i=0; i<MAX_EXPOS; i++) {
    if (mode == e_perout_mode_normal) mixState[i].activeExpo = false;
    ExpoData * ed = expoAddress(i);
    mixsrc_t srcRaw = ed->srcRaw;
    mixsrc_t src = abs(srcRaw);
    if (!EXPO_VALID(ed)) break; // end of list
    if (ed->chn == cur_chn)
      continue;
    if (ed->flightModes & (1<<mixerCurrentFlightMode))
      continue;
    if (src >= MIXSRC_FIRST_TRAINER && src <= MIXSRC_LAST_TRAINER && !isTrainerValid())
      continue;
    if (getSwitch(ed->swtch)) {
      int32_t v;
      if (srcRaw == ovwrIdx) {
        v = ovwrValue;
      }
      else {
        v = getValue(srcRaw);
        if (src >= MIXSRC_FIRST_TELEM && ed->scale > 0) {
          v = (v * 1024) / convertTelemValue(src-MIXSRC_FIRST_TELEM+1, ed->scale);
        }
        v = limit<int32_t>(-1024, v, 1024);
      }
      if (EXPO_MODE_ENABLE(ed, v)) {
        if (mode == e_perout_mode_normal) mixState[i].activeExpo = true;
        cur_chn = ed->chn;

        //========== CURVE=================
        if (ed->curve.value) {
          v = applyCurve(v, ed->curve);
        }

        //========== WEIGHT ===============
        int32_t weight = getSourceNumFieldValue(ed->weight, -100, 100);
        v = divRoundClosest((int32_t)v * weight, 1000);

        //========== OFFSET ===============
        int32_t offset = getSourceNumFieldValue(ed->offset, -100, 100);
        if (offset) v += divRoundClosest(calc100toRESX(offset), 10);

        //========== TRIMS ================
        if (ed->trimSource < TRIM_ON)
          virtualInputsTrims[cur_chn] = -ed->trimSource - 1;
        else if (ed->trimSource == TRIM_ON && src >= MIXSRC_FIRST_STICK &&
                 src <= MIXSRC_LAST_STICK)
          virtualInputsTrims[cur_chn] = src - MIXSRC_FIRST_STICK;
        else
          virtualInputsTrims[cur_chn] = -1;
        // if (srcRaw < 0) v = -v;
        anas[cur_chn] = v;
      }
    }
  }
}

// #define PREVENT_ARITHMETIC_OVERFLOW
// because of optimizations the reserves before overruns occurs is only the half
// this defines enables some checks the greatly improves this situation
// It should nearly prevent all overruns (is still a chance for it, but quite low)
// negative side is code cost 96 bytes flash

// we do it now half way, only in applyLimits, which costs currently 50bytes
// according opinion poll this topic is currently not very important
// the change below improves already the situation
// the check inside mixer would slow down mix a little bit and costs additionally flash
// also the check inside mixer still is not bulletproof, there may be still situations a overflow could occur
// a bulletproof implementation would take about additional 100bytes flash
// therefore with go with this compromize, interested people could activate this define

// @@@2 open.20.fsguruh ;
// channel = channelnumber -1;
// value = outputvalue with 100 mulitplied usual range -102400 to 102400; output -1024 to 1024
// changed rescaling from *100 to *256 to optimize performance
// rescaled from -262144 to 262144
int16_t applyLimits(uint8_t channel, int32_t value)
{
#if defined(OVERRIDE_CHANNEL_FUNCTION)
  if (safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED) {
    // safety channel available for channel check
    return calc100toRESX(safetyCh[channel]);
  }
#endif

  if (isFunctionActive(FUNCTION_TRAINER_CHANNELS) && isTrainerValid()) {
    return trainerInput[channel] * 2;
  }

  LimitData * lim = limitAddress(channel);

  if (lim->curve) {
    // TODO we loose precision here, applyCustomCurve could work with int32_t on ARM boards...
    if (lim->curve > 0)
      value = 256 * applyCustomCurve(value/256, lim->curve-1);
    else
      value = 256 * applyCustomCurve(-value/256, -lim->curve-1);
  }

  int16_t ofs   = LIMIT_OFS_RESX(lim);
  int16_t lim_p = LIMIT_MAX_RESX(lim);
  int16_t lim_n = LIMIT_MIN_RESX(lim);

  if (ofs > lim_p) ofs = lim_p;
  if (ofs < lim_n) ofs = lim_n;

  // because the rescaling optimization would reduce the calculation reserve we activate this for all builds
  // it increases the calculation reserve from factor 20,25x to 32x, which it slightly better as original
  // without it we would only have 16x which is slightly worse as original, we should not do this

  // thanks to gbirkus, he motivated this change, which greatly reduces overruns
  // unfortunately the constants and 32bit compares generates about 50 bytes codes; didn't find a way to get it down.
  value = limit(int32_t(-RESXl*256), value, int32_t(RESXl*256));  // saves 2 bytes compared to other solutions up to now

#if defined(PPM_LIMITS_SYMETRICAL)
  if (value) {
    int16_t tmp;
    if (lim->symetrical)
      tmp = (value > 0) ? (lim_p) : (-lim_n);
    else
      tmp = (value > 0) ? (lim_p - ofs) : (-lim_n + ofs);
    value = (int32_t) value * tmp;   //  div by 1024*256 -> output = -1024..1024
#else
  if (value) {
    int16_t tmp = (value > 0) ? (lim_p - ofs) : (-lim_n + ofs);
    value = (int32_t) value * tmp;   //  div by 1024*256 -> output = -1024..1024
#endif

    // Round away from 0
    tmp = (value + (value < 0 ? (1<<17)-1 : (1<<17))) >> 18;

    ofs += tmp;  // ofs can to added directly because already recalculated,
  }

  if (ofs > lim_p)
    ofs = lim_p;
  if (ofs < lim_n)
    ofs = lim_n;
  if (lim->revert)
    ofs = -ofs; // finally do the reverse.

  return ofs;
}

static const getvalue_t _switch_2pos_lookup[] = {
  -1024, // SWITCH_HW_UP
  +1024, // SWITCH_HW_MID
  +1024, // SWITCH_HW_DOWN 
};

static const getvalue_t _switch_3pos_lookup[] = {
  -1024, // SWITCH_HW_UP
  0,     // SWITCH_HW_MID
  +1024, // SWITCH_HW_DOWN 
};

// TODO same naming convention than the drawSource
// *valid added to return status to Lua for invalid sources
getvalue_t _getValue(mixsrc_t i, bool* valid)
{
  if (i == MIXSRC_NONE) {
    if (valid != nullptr) *valid = false;
    return 0;
  }
  else if (i <= MIXSRC_LAST_INPUT) {
    return anas[i - MIXSRC_FIRST_INPUT];
  }
#if defined(LUA_INPUTS)
  else if (i <= MIXSRC_LAST_LUA) {
#if defined(LUA_MODEL_SCRIPTS)
    div_t qr = div((uint16_t)(i-MIXSRC_FIRST_LUA), MAX_SCRIPT_OUTPUTS);
    return scriptInputsOutputs[qr.quot].outputs[qr.rem].value;
#else
    if (valid != nullptr) *valid = false;
    return 0;
#endif
  }
#endif

  else if (i <= MIXSRC_LAST_STICK) {
    i -= MIXSRC_FIRST_STICK;
    if (i >= adcGetMaxInputs(ADC_INPUT_MAIN)) {
      if (valid != nullptr) *valid = false;
      return 0;
    }
    return calibratedAnalogs[inputMappingConvertMode(i)];
  }
  else if (i <= MIXSRC_LAST_POT) {
    i -= MIXSRC_FIRST_POT;
    if (i >= adcGetMaxInputs(ADC_INPUT_FLEX)) {
      if (valid != nullptr) *valid = false;
      return 0;
    }
    return calibratedAnalogs[i + adcGetInputOffset(ADC_INPUT_FLEX)];
  }

#if defined(IMU)
  else if (i == MIXSRC_TILT_X) {
    return gyro.scaledX();
  }
  else if (i == MIXSRC_TILT_Y) {
    return gyro.scaledY();
  }
#endif

#if defined(PCBHORUS)
  else if (i >= MIXSRC_FIRST_SPACEMOUSE && i <= MIXSRC_LAST_SPACEMOUSE) {
#if defined(SPACEMOUSE)
    return get_spacemouse_value(i - MIXSRC_FIRST_SPACEMOUSE);
#else
    return 0;
#endif
  }
#endif

  else if (i == MIXSRC_MIN) {
    return -RESX;
  }
  else if (i == MIXSRC_MAX) {
    return RESX;
  }

  else if (i <= MIXSRC_LAST_HELI) {
#if defined(HELI)
    return cyc_anas[i - MIXSRC_FIRST_HELI];
#else
    if (valid != nullptr) *valid = false;
    return 0;
#endif
  }

  else if (i <= MIXSRC_LAST_TRIM) {
    i -= MIXSRC_FIRST_TRIM;
    if (getRawTrimValue(mixerCurrentFlightMode, i).mode == TRIM_MODE_3POS) {
      // Trim set as 3POS toggle switch in FM
      uint8_t tidx = inputMappingConvertMode(i) * 2;
      if (trimDown(tidx)) return -RESX;
      else if (trimDown(tidx + 1)) return RESX;
      return 0;
    }
    return 8 * getTrimValue(mixerCurrentFlightMode, i);
  }
  else if (i >= MIXSRC_FIRST_SWITCH && i <= MIXSRC_LAST_SWITCH) {
    auto sw_idx = (uint8_t)(i - MIXSRC_FIRST_SWITCH);
#if defined(FUNCTION_SWITCHES)
    auto max_reg_switches = switchGetMaxSwitches();
    if (sw_idx >= max_reg_switches) {
      auto fct_idx = sw_idx - max_reg_switches;
      auto max_fct_switches = switchGetMaxFctSwitches();
      if (fct_idx < max_fct_switches) {
        return _switch_2pos_lookup[getFSLogicalState(fct_idx)];
      }
    }
#endif
    auto sw_cfg = (SwitchConfig)SWITCH_CONFIG(sw_idx);
    switch(sw_cfg) {
    case SWITCH_NONE:
      if (valid != nullptr) *valid = false;
      return 0;
    case SWITCH_TOGGLE:
    case SWITCH_2POS:
      return _switch_2pos_lookup[switchGetPosition(sw_idx)];
    case SWITCH_3POS:
      return _switch_3pos_lookup[switchGetPosition(sw_idx)];
    }
  }
#if defined(FUNCTION_SWITCHES)
    else if (i <= MIXSRC_LAST_CUSTOMSWITCH_GROUP) {
      uint8_t group_idx = (uint8_t)(i - MIXSRC_FIRST_CUSTOMSWITCH_GROUP + 1);
      uint8_t stepcount = getSwitchCountInFSGroup(group_idx);
      if (stepcount == 0)
        return 0;

      if (IS_FSWITCH_GROUP_ON(group_idx))
        stepcount--;

      int stepsize = (2 * RESX) / stepcount;
      int value = -RESX;

      for (uint8_t i =  0; i < switchGetMaxFctSwitches(); i++) {
        if(FSWITCH_GROUP(i) == group_idx) {
          if (getFSLogicalState(i) == 1)
            return value + (IS_FSWITCH_GROUP_ON(group_idx) ? 0 : stepsize);
          else
            value += stepsize;
        }
      }
      return -RESX;
  }
#endif

  else if (i <= MIXSRC_LAST_LOGICAL_SWITCH) {
    return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + i - MIXSRC_FIRST_LOGICAL_SWITCH) ? 1024 : -1024;
  } else if (i <= MIXSRC_LAST_TRAINER) {
    int16_t x = trainerInput[i - MIXSRC_FIRST_TRAINER];
    if (i < MIXSRC_FIRST_TRAINER + NUM_CAL_PPM) {
      x -= g_eeGeneral.trainer.calib[i - MIXSRC_FIRST_TRAINER];
    }
    return x * 2;
  } else if (i <= MIXSRC_LAST_CH) {
    return ex_chans[i - MIXSRC_FIRST_CH];
  }

  else if (i <= MIXSRC_LAST_GVAR) {
#if defined(GVARS)
    return GVAR_VALUE(i - MIXSRC_FIRST_GVAR, getGVarFlightMode(mixerCurrentFlightMode, i - MIXSRC_FIRST_GVAR));
#else
    if (valid != nullptr) *valid = false;
    return 0;
#endif
  }

  else if (i == MIXSRC_TX_VOLTAGE) {
    return g_vbat100mV;
  } else if (i < MIXSRC_FIRST_TIMER) {
    // TX_TIME + SPARES
#if defined(RTCLOCK)
    return (g_rtcTime % SECS_PER_DAY) / 60; // number of minutes from midnight
#else
    if (valid != nullptr) *valid = false;
    return 0;
#endif
  } else if (i <= MIXSRC_LAST_TIMER) {
    return timersStates[i - MIXSRC_FIRST_TIMER].val;
  }

  else if (i <= MIXSRC_LAST_TELEM) {
    if (IS_FAI_FORBIDDEN(i)) {
      if (valid != nullptr) *valid = false;
      return 0;
    }
    i -= MIXSRC_FIRST_TELEM;
    div_t qr = div((uint16_t)i, 3);
    TelemetryItem & telemetryItem = telemetryItems[qr.quot];
    switch (qr.rem) {
      case 1:
        return telemetryItem.valueMin;
      case 2:
        return telemetryItem.valueMax;
      default:
        return telemetryItem.value;
    }
  }
  
  if (valid != nullptr) *valid = false;
  return 0;
}

void evalTrims()
{
  uint8_t phase = mixerCurrentFlightMode;
  for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {
    // do trim -> throttle trim if applicable
    int16_t trim = getTrimValue(phase, i);
    if (trimsCheckTimer > 0) {
      trim = 0;
    }

    trims[i] = trim * 2;
  }
}

// TODO: move to analogs.cpp
void evalInputs(uint8_t mode)
{
  BeepANACenter anaCenter = 0;

#if defined(STICK_DEAD_ZONE)
  int16_t deadZoneOffset = g_eeGeneral.stickDeadZone ? 2 << (g_eeGeneral.stickDeadZone - 1) : 0;
#endif

  auto max_calib_analogs = adcGetInputOffset(ADC_INPUT_VBAT);
  auto pots_offset = adcGetInputOffset(ADC_INPUT_FLEX);
  
  for (uint8_t i = 0; i < max_calib_analogs; i++) {
    int16_t v = anaIn(i);
    uint8_t ch = (i < pots_offset ? inputMappingConvertMode(i) : i);

    // [0..2048] -> [-1024..1024]
    v -= RESX;

#if defined(STICK_DEAD_ZONE)
    // dead zone invented by FlySky in my opinion it should goes into ADC
    if (g_eeGeneral.stickDeadZone && ch != inputMappingGetThrottle()) {
      if (v > deadZoneOffset) {
        // y=ax+b
        v = (int16_t)((int32_t)(v - deadZoneOffset) * 1024L / (1024L - deadZoneOffset));
      } else if (v < -deadZoneOffset) {
        // y=ax+b
        v = (int16_t)((int32_t)(v + deadZoneOffset) * 1024L / (1024L - deadZoneOffset));
      } else {
        v = 0;
      }
    }
#endif

    if (g_model.throttleReversed && ch == inputMappingGetThrottle()) {
      v = -v;
    }

    BeepANACenter mask = (BeepANACenter)1 << ch; // TODO

    calibratedAnalogs[i] = v; // for show in expo

    // filtering for center beep
    uint8_t tmp = (uint16_t)abs(v) / 16;
    if (mode == e_perout_mode_normal) {
      if (tmp==0 || (tmp==1 && (bpanaCenter & mask))) {
        anaCenter |= mask;
        if ((g_model.beepANACenter & mask) && !(bpanaCenter & mask) &&
            s_mixer_first_run_done && !menuCalibrationState) {
          if (i < pots_offset || IS_POT_SLIDER_AVAILABLE(i - pots_offset)) {
            AUDIO_POT_MIDDLE(i);
          }
        }
      }
    }

    if (ch < pots_offset) { // only do this for sticks
      if (mode & e_perout_mode_nosticks) {
        v = 0;
      }

      if (mode <= e_perout_mode_inactive_flight_mode &&
          isFunctionActive(FUNCTION_TRAINER_STICK1 + ch) &&
          isTrainerValid()) {
        // trainer mode
        TrainerMix* td = &g_eeGeneral.trainer.mix[ch];
        if (td->mode) {
          uint8_t chStud = td->srcChn;
          int32_t vStud =
              (trainerInput[chStud] - g_eeGeneral.trainer.calib[chStud]);
          vStud *= td->studWeight;
          vStud /= 50;
          switch (td->mode) {
            case TRAINER_ADD:
              // add-mode
              v = limit<int16_t>(-RESX, v + vStud, RESX);
              break;
            case TRAINER_REPL:
              // subst-mode
              v = vStud;
              break;
          }
        }
      }
      calibratedAnalogs[i] = v;
    }
  }

  // EXPOs
  applyExpos(anas, mode);

  // TRIMs
  // when no virtual inputs, the trims need the anas array calculated above
  // (when throttle trim enabled)
  evalTrims();

  if (mode == e_perout_mode_normal) {
    bpanaCenter = anaCenter;
  }
}

getvalue_t getValue(mixsrc_t i, bool* valid)
{
  bool invert = false;
  if (i < 0) {
    invert = true;
    i = -i;
  }
  getvalue_t v = _getValue(i, valid);
  if (invert) v = -v;
  return v;
}  

#if defined(SURFACE_RADIO)
  constexpr int IDLE_TRIM_SCALE = 1;
#else
  constexpr int IDLE_TRIM_SCALE = 2;
#endif

int getStickTrimValue(int stick, int stickValue)
{
  if (stick < 0)
    return 0;

  int trim = trims[stick];
  uint8_t thrTrimSw = g_model.getThrottleStickTrimSource() - MIXSRC_FIRST_TRIM;
  if (stick == thrTrimSw) {  // trim for throttle
    if (g_model.throttleReversed) trim = -trim;
    if (g_model.thrTrim) {   // throttle idle ON
      // evalTrims() store 2 * trim value in trims[]
      // so here, trim values range from -256 to 256
      // and extended trim values range from -1024 to 1024
      trim = (g_model.extendedTrims) ? 2 * TRIM_EXTENDED_MAX + trim
                                     : 2 * TRIM_MAX + trim;
      trim = trim * (1024 - stickValue) / (IDLE_TRIM_SCALE * RESX);
#if defined(SURFACE_RADIO)
      // Throttle Idle trim (g_model.thrTrim) should affect only forward stick (0 to 1024)
      // Return no trim for reverse side when throttle idle trim is enabled
      if (stickValue < 0) return 0;
#endif
    }
#if defined(SURFACE_RADIO)
    // divide throtle trim by two since since the full extend
    // of forward/reverse chan is only 1024 instead of 2048
    trim >>= 1;
#endif
  }
  return trim;
}

int getSourceTrimOrigin(int source)
{
  if (source >= MIXSRC_FIRST_STICK && source <= MIXSRC_LAST_STICK)
    return source - MIXSRC_FIRST_STICK;
  else if (source >= MIXSRC_FIRST_INPUT && source <= MIXSRC_LAST_INPUT)
    return virtualInputsTrims[source - MIXSRC_FIRST_INPUT];
  else
    return -1;
}

int getSourceTrimValue(int source, int stickValue=0)
{
  auto origin = getSourceTrimOrigin(source);
  if (origin >= 0) {
    return getStickTrimValue(origin, stickValue);
  }
  else {
    return 0;
  }
}

constexpr bitfield_channels_t all_channels_dirty = (bitfield_channels_t)-1;

static inline bitfield_channels_t channel_bit(uint16_t ch)
{
  return (bitfield_channels_t)1 << ch;
}

static inline bitfield_channels_t channel_dirty(bitfield_channels_t mask, uint16_t ch)
{
  return mask & channel_bit(ch);
}

static inline bitfield_channels_t upper_channels_mask(uint16_t ch)
{
  // take the 1's complement to generate a bit pattern
  // that has all bits of 'ch' order and above set
  //
  // Examples (mask for max 8 channels):
  // - channel 0: 0b11111111
  // - channel 1: 0b11111110
  // - channel 2: 0b11111100

  return ~(channel_bit(ch)) + 1;
}

uint8_t mixerCurrentFlightMode;

void evalFlightModeMixes(uint8_t mode, uint8_t tick10ms)
{
  evalInputs(mode);

  if (tick10ms)
    evalLogicalSwitches(mode==e_perout_mode_normal);

#if defined(HELI)
  if (modelHeliEnabled()) {
    int heliEleValue = getValue(g_model.swashR.elevatorSource);
    int heliAilValue = getValue(g_model.swashR.aileronSource);
    if (g_model.swashR.value) {
      uint32_t v = ((int32_t)heliEleValue*heliEleValue + (int32_t)heliAilValue*heliAilValue);
      uint32_t q = calc100toRESX(g_model.swashR.value);
      q *= q;
      if (v>q) {
        uint16_t d = isqrt32(v);
        int16_t tmp = calc100toRESX(g_model.swashR.value);
        heliEleValue = (int32_t) heliEleValue*tmp/d;
        heliAilValue = (int32_t) heliAilValue*tmp/d;
      }
    }

  #define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
  #define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

    if (g_model.swashR.type) {
      getvalue_t vp = heliEleValue + getSourceTrimValue(g_model.swashR.elevatorSource);
      getvalue_t vr = heliAilValue + getSourceTrimValue(g_model.swashR.aileronSource);
      getvalue_t vc = 0;
      if (g_model.swashR.collectiveSource)
        vc = getValue(g_model.swashR.collectiveSource);

      vp = (vp * g_model.swashR.elevatorWeight) / 100;
      vr = (vr * g_model.swashR.aileronWeight) / 100;
      vc = (vc * g_model.swashR.collectiveWeight) / 100;

      switch (g_model.swashR.type) {
        case SWASH_TYPE_120:
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_X(vr);
          cyc_anas[0] = vc - vp;
          cyc_anas[1] = vc + vp/2 + vr;
          cyc_anas[2] = vc + vp/2 - vr;
          break;
        case SWASH_TYPE_120X:
          vp = REZ_SWASH_X(vp);
          vr = REZ_SWASH_Y(vr);
          cyc_anas[0] = vc - vr;
          cyc_anas[1] = vc + vr/2 + vp;
          cyc_anas[2] = vc + vr/2 - vp;
          break;
        case SWASH_TYPE_140:
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_Y(vr);
          cyc_anas[0] = vc - vp;
          cyc_anas[1] = vc + vp + vr;
          cyc_anas[2] = vc + vp - vr;
          break;
        case SWASH_TYPE_90:
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_Y(vr);
          cyc_anas[0] = vc - vp;
          cyc_anas[1] = vc + vr;
          cyc_anas[2] = vc - vr;
          break;
        default:
          break;
      }
    }
  } else {
    cyc_anas[0] = cyc_anas[1] = cyc_anas[2] = 0;
  }
#endif

  memclear(chans, sizeof(chans)); // all outputs to 0

  //========== MIXER LOOP ===============

  uint8_t pass = 0;
  uint8_t lv_mixWarning = 0;
  bitfield_channels_t dirtyChannels = all_channels_dirty;

  // Calculate locally and then copy to mixState array - prevent UI seeing phantom values while calculating
  bool activeMixes[MAX_MIXERS];

  do {
    bitfield_channels_t passDirtyChannels = 0;

    for (uint8_t i=0; i<MAX_MIXERS; i++) {
      if (mode == e_perout_mode_normal && pass == 0)
        activeMixes[i] = 0;

      MixData * md = mixAddress(i);

      if (md->srcRaw == 0) {
#if defined(COLORLCD)
        continue;
#else
        break;
#endif
      }

      if (!channel_dirty(dirtyChannels, md->destCh))
        continue;

      // if this is the first calculation for the destination channel,
      // initialize it with 0 (otherwise would be random)
      if (i == 0 || md->destCh != (md - 1)->destCh)
        chans[md->destCh] = 0;

      //========== FLIGHT MODE && SWITCH =====
      bool mixCondition = (md->flightModes != 0 || md->swtch);
      bool fmEnabled = (md->flightModes & (1 << mixerCurrentFlightMode)) == 0;
      bool mixLineActive = fmEnabled && getSwitch(md->swtch);
      delayval_t mixEnabled = (mixLineActive) ? DELAY_POS_MARGIN+1 : 0;

      if (mixLineActive) {
        // disable mixer using trainer channels if not connected
        if (md->srcRaw >= MIXSRC_FIRST_TRAINER &&
            md->srcRaw <= MIXSRC_LAST_TRAINER && !isTrainerValid()) {
          mixCondition = true;
          mixEnabled = 0;
        }

#if defined(LUA_MODEL_SCRIPTS)
        // disable mixer if Lua script is used as source and script was killed
        if (md->srcRaw >= MIXSRC_FIRST_LUA && md->srcRaw <= MIXSRC_LAST_LUA) {
          div_t qr = div(md->srcRaw - MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
          if (scriptInternalData[qr.quot].state != SCRIPT_OK) {
            mixCondition = true;
            mixEnabled = 0;
          }
        }
#endif
      }

      //========== VALUE ===============
      getvalue_t v = 0;

      if (mode > e_perout_mode_inactive_flight_mode) {
        if (mixEnabled)
          v = getValue(md->srcRaw);
        else
          continue;
      } else {
        mixsrc_t srcRaw = md->srcRaw;
        v = getValue(srcRaw);

        if (srcRaw >= MIXSRC_FIRST_CH) {

          auto srcChan = srcRaw - MIXSRC_FIRST_CH;
          if (srcChan <= MAX_OUTPUT_CHANNELS && md->destCh != srcChan) {

            // check whether we need to recompute the current channel later
            bitfield_channels_t upperChansMask = upper_channels_mask(md->destCh);
            bitfield_channels_t srcChanDirtyMask = channel_dirty(dirtyChannels, srcChan);

            // if the source is any of the channels marked as dirty
            // or contained in [ destCh, MAX_OUTPUT_CHANNELS [
            if (srcChanDirtyMask & (passDirtyChannels | upperChansMask)) {
              passDirtyChannels |= channel_bit(md->destCh);
            }

            // if the source has already be computed,
            // then use it!
            if (srcChan < md->destCh || pass > 0) {
              // channels are in [ -1024 * 256, 1024 * 256 ]
              v = chans[srcChan] >> 8;
            }
          }
        }
        if (!mixCondition)
          mixEnabled = v;
      }

      bool applyOffsetAndCurve = true;

      //========== DELAYS ===============
      delayval_t _swOn = mixState[i].now;
      delayval_t _swPrev = mixState[i].prev;
      bool swTog = (mixEnabled > _swOn+DELAY_POS_MARGIN || mixEnabled < _swOn-DELAY_POS_MARGIN);

      if (mode == e_perout_mode_normal && swTog) {
        if (!mixState[i].delay)
          _swPrev = _swOn;
        int32_t precMult = md->delayPrec ? 1 : 10;
        mixState[i].delay = (mixEnabled > _swOn ? md->delayUp : md->delayDown) * precMult;
        mixState[i].now = mixEnabled;
        mixState[i].prev = _swPrev;
      }
      if (mode == e_perout_mode_normal && mixState[i].delay > 0) {
        mixState[i].delay = max<int16_t>(0, (int16_t)mixState[i].delay - tick10ms);
        // Freeze value until delay expires
        if (!mixCondition)
          v = _swPrev;
        else if (mixEnabled)
          continue;
      }
      else {
        if (mode == e_perout_mode_normal) {
          mixState[i].now = mixState[i].prev = mixEnabled;
        }
        if (!mixEnabled) {
          if ((md->speedDown || md->speedUp) && md->mltpx != MLTPX_REPL) {
            if (mixCondition) {
              v = (md->mltpx == MLTPX_ADD ? 0 : RESX);
              applyOffsetAndCurve = false;
            }
          } else if (mixCondition) {
            continue;
          }
        }
      }

      if (mode == e_perout_mode_normal && (!mixCondition || mixEnabled || mixState[i].delay)) {
        if (md->mixWarn) lv_mixWarning |= 1 << (md->mixWarn - 1);
        activeMixes[i] = true;
      }

      if (applyOffsetAndCurve) {
        bool applyTrims = !(mode & e_perout_mode_notrims);
        if (!applyTrims && g_model.thrTrim) {
          auto origin = getSourceTrimOrigin(md->srcRaw);
          if (origin == g_model.getThrottleStickTrimSource() - MIXSRC_FIRST_TRIM) {
            applyTrims = true;
          }
        }
        if (applyTrims && md->carryTrim == 0) {
          v += getSourceTrimValue(md->srcRaw, v);
        }
      }

      int32_t weight = getSourceNumFieldValue(md->weight, -RESX, RESX);
      weight = calc100to256_16Bits(weight);
      //========== SPEED ===============
      // now its on input side, but without weight compensation. More like other remote controls
      // lower weight causes slower movement

      if (mode <= e_perout_mode_inactive_flight_mode && (md->speedUp || md->speedDown)) { // there are delay values
#define DEL_MULT_SHIFT 8
        // we recale to a mult 256 higher value for calculation
        int32_t tact = act[i];
        int16_t diff = v - (tact>>DEL_MULT_SHIFT);
        if (diff) {
          // open.20.fsguruh: speed is defined in % movement per second; In menu we specify the full movement (-100% to 100%) = 200% in total
          // the unit of the stored value is the value from md->speedUp or md->speedDown * 0.1s; e.g. value 4 means 0.4 seconds
          // because we get a tick each 10msec, we need 100 ticks for one second
          // the value in md->speedXXX gives the time it should take to do a full movement from -100 to 100 therefore 200%. This equals 2048 in recalculated internal range
          if (tick10ms || !s_mixer_first_run_done) {
            // only if already time is passed add or substract a value according the speed configured
            int32_t rate = (int32_t) tick10ms << (DEL_MULT_SHIFT+11);  // = DEL_MULT*2048*tick10ms
            // rate equals a full range for one second; if less time is passed rate is accordingly smaller
            // if one second passed, rate would be 2048 (full motion)*256(recalculated weight)*100(100 ticks needed for one second)
            int32_t currentValue = ((int32_t) v<<DEL_MULT_SHIFT);
            int32_t precMult = md->speedPrec ? 1 : 10;
            if (diff > 0) {
              if (s_mixer_first_run_done && md->speedUp > 0) {
                // if a speed upwards is defined recalculate the new value according configured speed; the higher the speed the smaller the add value is
                int32_t newValue = tact+rate/((int16_t)precMult*md->speedUp);
                if (newValue<currentValue) currentValue = newValue; // Endposition; prevent toggling around the destination
              }
            }
            else {  // if is <0 because ==0 is not possible
              if (s_mixer_first_run_done && md->speedDown > 0) {
                // see explanation in speedUp
                int32_t newValue = tact-rate/((int16_t)precMult*md->speedDown);
                if (newValue>currentValue) currentValue = newValue; // Endposition; prevent toggling around the destination
              }
            }
            act[i] = tact = currentValue;
            // open.20.fsguruh: this implementation would save about 50 bytes code
          } // endif tick10ms ; in case no time passed assign the old value, not the current value from source
          v = (tact >> DEL_MULT_SHIFT);
        }
      }

      //========== CURVES ===============
      if (applyOffsetAndCurve && md->curve.type != CURVE_REF_DIFF && md->curve.value) {
        v = applyCurve(v, md->curve);
      }

      //========== WEIGHT ===============
      int32_t dv = (int32_t)v * weight;
      dv = divRoundClosest(dv, 10);

      //========== OFFSET / AFTER ===============
      if (applyOffsetAndCurve) {
        int32_t offset = getSourceNumFieldValue(md->offset, -RESX, RESX);
        if (offset) dv += divRoundClosest(calc100toRESX_16Bits(offset), 10) << 8;
      }

      //========== DIFFERENTIAL =========
      if (md->curve.type == CURVE_REF_DIFF && md->curve.value) {
        dv = applyCurve(dv, md->curve);
      }

      int32_t * ptr = &chans[md->destCh]; // Save calculating address several times

      // If first mix line for a channel - ignore Multiplex setting
      if (i == 0 || mixAddress(i - 1)->destCh != md->destCh) {
        *ptr = dv;
      } else {
        switch (md->mltpx) {
          case MLTPX_REPL:
            *ptr = dv;
            if (mode == e_perout_mode_normal) {
              for (int8_t m = i - 1; m >= 0 && mixAddress(m)->destCh == md->destCh; m--)
                activeMixes[m] = false;
            }
            break;
          case MLTPX_MUL:
            // @@@2 we have to remove the weight factor of 256 in case of 100%; now we use the new base of 256
            dv >>= 8;
            dv *= *ptr;
            dv >>= RESX_SHIFT;   // same as dv /= RESXl;
            *ptr = dv;
            break;
          default: // MLTPX_ADD
            *ptr += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
            break;
        } // endswitch md->mltpx
      }
#ifdef PREVENT_ARITHMETIC_OVERFLOW
/*
      // a lot of assumptions must be true, for this kind of check; not really worth for only 4 bytes flash savings
      // this solution would save again 4 bytes flash
      int8_t testVar=(*ptr<<1)>>24;
      if ( (testVar!=-1) && (testVar!=0 ) ) {
        // this devices by 64 which should give a good balance between still over 100% but lower then 32x100%; should be OK
        *ptr >>= 6;  // this is quite tricky, reduces the value a lot but should be still over 100% and reduces flash need
      } */


      PACK( union u_int16int32_t {
        struct {
          int16_t lo;
          int16_t hi;
        } words_t;
        int32_t dword;
      });

      u_int16int32_t tmp;
      tmp.dword=*ptr;

      if (tmp.dword<0) {
        if ((tmp.words_t.hi&0xFF80)!=0xFF80) tmp.words_t.hi=0xFF86; // set to min nearly
      }
      else {
        if ((tmp.words_t.hi|0x007F)!=0x007F) tmp.words_t.hi=0x0079; // set to max nearly
      }
      *ptr = tmp.dword;
      // this implementation saves 18bytes flash

/*      dv=*ptr>>8;
      if (dv>(32767-RESXl)) {
        *ptr=(32767-RESXl)<<8;
      } else if (dv<(-32767+RESXl)) {
        *ptr=(-32767+RESXl)<<8;
      }*/
      // *ptr=limit( int32_t(int32_t(-1)<<23), *ptr, int32_t(int32_t(1)<<23));  // limit code cost 72 bytes
      // *ptr=limit( int32_t((-32767+RESXl)<<8), *ptr, int32_t((32767-RESXl)<<8));  // limit code cost 80 bytes
#endif
    } //endfor mixers

    tick10ms = 0;
    dirtyChannels &= passDirtyChannels;

  } while (++pass < 5 && dirtyChannels);

  for (uint8_t i=0; i<MAX_MIXERS; i++)
    mixState[i].activeMix = activeMixes[i];

  mixWarning = lv_mixWarning;
}



#define MAX_ACT 0xffff
uint8_t lastFlightMode = 255; // TODO reinit everything here when the model changes, no???

tmr10ms_t flightModeTransitionTime;
uint8_t   flightModeTransitionLast = 255;

void evalMixes(uint8_t tick10ms)
{
  int32_t sum_chans512[MAX_OUTPUT_CHANNELS];

  static uint16_t fp_act[MAX_FLIGHT_MODES] = {0};
  static uint16_t delta = 0;
  static uint16_t flightModesFade = 0;

  uint8_t fm = getFlightMode();

  if (lastFlightMode != fm) {
    flightModeTransitionTime = get_tmr10ms();

    if (lastFlightMode == 255) {
      fp_act[fm] = MAX_ACT;
    }
    else {
      uint8_t fadeTime = max(g_model.flightModeData[lastFlightMode].fadeOut, g_model.flightModeData[fm].fadeIn);
      uint16_t transitionMask = (0x01u << lastFlightMode) + (0x01u << fm);
      if (fadeTime) {
        flightModesFade |= transitionMask;
        delta = (MAX_ACT / 10) / fadeTime;
      }
      else {
        flightModesFade &= ~transitionMask;
        fp_act[lastFlightMode] = 0;
        fp_act[fm] = MAX_ACT;
      }
      logicalSwitchesCopyState(lastFlightMode, fm); // push last logical switches state from old to new flight mode
    }
    lastFlightMode = fm;
  }

  if (flightModeTransitionTime && get_tmr10ms() > flightModeTransitionTime+SWITCHES_DELAY()) {
    flightModeTransitionTime = 0;
    if (fm != flightModeTransitionLast) {
      if (flightModeTransitionLast != 255) {
        PLAY_PHASE_OFF(flightModeTransitionLast);
      }
      PLAY_PHASE_ON(fm);
      flightModeTransitionLast = fm;
    }
  }

  int32_t weight = 0;
  if (flightModesFade) {
    memclear(sum_chans512, sizeof(sum_chans512));
    for (uint8_t p=0; p<MAX_FLIGHT_MODES; p++) {
      if (flightModesFade & (0x01 << p)) {
        mixerCurrentFlightMode = p;
        evalFlightModeMixes(p==fm ? e_perout_mode_normal : e_perout_mode_inactive_flight_mode, p==fm ? tick10ms : 0);
        for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS; i++)
          sum_chans512[i] += limit<int32_t>(-0x6fff, chans[i] >> 4, 0x6fff) * fp_act[p];
        weight += fp_act[p];
      }
    }
    assert(weight);
    mixerCurrentFlightMode = fm;
  }
  else {
    mixerCurrentFlightMode = fm;
    evalFlightModeMixes(e_perout_mode_normal, tick10ms);
  }

  //========== FUNCTIONS ===============
  // must be done after mixing because some functions use the inputs/channels values
  // must be done before limits because of the applyLimit function: it checks for safety switches which would be not initialized otherwise
  if (tick10ms) {
#if defined(AUDIO)
    requiredSpeakerVolume = g_eeGeneral.speakerVolume + VOLUME_LEVEL_DEF;
#endif
  
    requiredBacklightBright = g_eeGeneral.getBrightness();

    if (radioGFEnabled()) {
      evalFunctions(g_eeGeneral.customFn, globalFunctionsContext);
    } else {
      globalFunctionsContext.reset();
    }
    if (modelSFEnabled()) {
      evalFunctions(g_model.customFn, modelFunctionsContext);
    } else {
      modelFunctionsContext.reset();
    }
#if defined(OVERRIDE_CHANNEL_FUNCTION)
    if (!radioGFEnabled() && !modelSFEnabled()) {
      for (uint8_t i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
        safetyCh[i] = OVERRIDE_CHANNEL_UNDEFINED;
      }
    }
#endif
  }

  //========== LIMITS ===============
  for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS; i++) {
    // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*256
    // later we multiply by the limit (up to 100) and then we need to normalize
    // at the end chans[i] = chans[i]/256 =>  -1024..1024
    // interpolate value with min/max so we get smooth motion from center to stop
    // this limits based on v original values and min=-1024, max=1024  RESX=1024
    int32_t q = (flightModesFade ? (sum_chans512[i] / weight) << 4 : chans[i]);

    ex_chans[i] = q / 256;

    int16_t value = applyLimits(i, q);  // applyLimits will remove the 256 100% basis

    channelOutputs[i] = value;  // copy consistent word to int-level
  }

  if (tick10ms && flightModesFade) {
    uint16_t tick_delta = delta * tick10ms;
    for (uint8_t p=0; p<MAX_FLIGHT_MODES; p++) {
      uint16_t flightModeMask = (0x01 << p);
      if (flightModesFade & flightModeMask) {
        if (p == fm) {
          if (MAX_ACT - fp_act[p] > tick_delta)
            fp_act[p] += tick_delta;
          else {
            fp_act[p] = MAX_ACT;
            flightModesFade -= flightModeMask;
          }
        }
        else {
          if (fp_act[p] > tick_delta)
            fp_act[p] -= tick_delta;
          else {
            fp_act[p] = 0;
            flightModesFade -= flightModeMask;
          }
        }
      }
    }
  }
}

#if defined(THRTRACE)
uint8_t  s_traceBuf[MAXTRACE];
uint16_t s_traceWr;
uint8_t  s_cnt_10s;
uint16_t s_cnt_samples_thr_10s;
uint16_t s_sum_samples_thr_10s;
#endif

void doMixerPeriodicUpdates()
{
  static tmr10ms_t lastTMR = 0;

  tmr10ms_t tmr10ms = get_tmr10ms();

  uint8_t tick10ms = (tmr10ms >= lastTMR ? tmr10ms - lastTMR : 1);
  // handle tick10ms overrun
  // correct overflow handling costs a lot of code; happens only each 11 min;
  // therefore forget the exact calculation and use only 1 instead; good compromise
  lastTMR = tmr10ms;

  DEBUG_TIMER_START(debugTimerMixes10ms);
  if (tick10ms) {
    /* Throttle trace */
    int16_t val;

    if (g_model.thrTraceSrc > MAX_POTS) {
      uint8_t ch = g_model.thrTraceSrc - MAX_POTS - 1;
      val = channelOutputs[ch];

      LimitData * lim = limitAddress(ch);
      int16_t gModelMax = LIMIT_MAX_RESX(lim);
      int16_t gModelMin = LIMIT_MIN_RESX(lim);

      if (lim->revert)
        val = -val + gModelMax;
      else
        val = val - gModelMin;

#if defined(PPM_LIMITS_SYMETRICAL)
      if (lim->symetrical) {
        val -= calc1000toRESX(lim->offset);
      }
#endif

      gModelMax -= gModelMin; // we compare difference between Max and Mix for recaling needed; Max and Min are shifted to 0 by default
      // usually max is 1024 min is -1024 --> max-min = 2048 full range

      if (gModelMax != 0 && gModelMax != 2048)
        val = (int32_t) (val << 11) / (gModelMax); // rescaling only needed if Min, Max differs

      if (val < 0)
        val=0;  // prevent val be negative, which would corrupt throttle trace and timers; could occur if safetyswitch is smaller than limits
    }
    else {
      val = RESX + calibratedAnalogs[g_model.thrTraceSrc == 0 ? inputMappingConvertMode(inputMappingGetThrottle()) : g_model.thrTraceSrc + MAX_STICKS - 1];
    }

    val >>= (RESX_SHIFT-6); // calibrate it (resolution increased by factor 4)

    evalTimers(val, tick10ms);

    static uint8_t  s_cnt_100ms;
    static uint8_t  s_cnt_1s;
    static uint8_t  s_cnt_samples_thr_1s;
    static uint16_t s_sum_samples_thr_1s;

    s_cnt_samples_thr_1s++;
    s_sum_samples_thr_1s+=val;

    if ((s_cnt_100ms += tick10ms) >= 10) { // 0.1sec
      s_cnt_100ms -= 10;
      s_cnt_1s += 1;

      logicalSwitchesTimerTick();
      checkTrainerSignalWarning();

      if (s_cnt_1s >= 10) { // 1sec
        s_cnt_1s -= 10;
        sessionTimer += 1;
        inactivity.counter++;
        if ((((uint8_t)inactivity.counter) & 0x07) == 0x01 && g_eeGeneral.inactivityTimer && inactivity.counter > ((uint16_t)g_eeGeneral.inactivityTimer * 60))
          AUDIO_INACTIVITY();

#if defined(AUDIO)
        if (mixWarning & 1) if ((sessionTimer&0x03)==0) AUDIO_MIX_WARNING(1);
        if (mixWarning & 2) if ((sessionTimer&0x03)==1) AUDIO_MIX_WARNING(2);
        if (mixWarning & 4) if ((sessionTimer&0x03)==2) AUDIO_MIX_WARNING(3);
#endif

        val = s_sum_samples_thr_1s / s_cnt_samples_thr_1s;
        s_timeCum16ThrP += (val>>3);  // s_timeCum16ThrP would overrun if we would store throttle value with higher accuracy; therefore stay with 16 steps
        if (val)
          s_timeCumThr += 1;
        s_sum_samples_thr_1s >>= 2;  // correct better accuracy now, because trace graph can show this information; in case thrtrace is not active, the compile should remove this

#if defined(THRTRACE)
        // throttle trace is done every 10 seconds; Tracebuffer is adjusted to screen size.
        // in case buffer runs out, it wraps around
        // resolution for y axis is only 32, therefore no higher value makes sense
        s_cnt_samples_thr_10s += s_cnt_samples_thr_1s;
        s_sum_samples_thr_10s += s_sum_samples_thr_1s;

        if (++s_cnt_10s >= 10) { // 10s
          s_cnt_10s -= 10;
          val = s_sum_samples_thr_10s / s_cnt_samples_thr_10s;
          s_sum_samples_thr_10s = 0;
          s_cnt_samples_thr_10s = 0;
          s_traceBuf[s_traceWr % MAXTRACE] = val;
          s_traceWr++;
        }
#endif

        s_cnt_samples_thr_1s = 0;
        s_sum_samples_thr_1s = 0;
      }
    }

#if defined(PXX) || defined(DSM2)
    static uint8_t countRangecheck = 0;
    for (uint8_t i = 0; i < NUM_MODULES; ++i) {
      if (isModuleBeeping(i)) {
        if (++countRangecheck >= 250) {
          countRangecheck = 0;
          AUDIO_PLAY(AU_SPECIAL_SOUND_CHEEP);
        }
      }
    }
#endif

    checkTrims();
  }

  DEBUG_TIMER_STOP(debugTimerMixes10ms);

  s_mixer_first_run_done = true;
}
