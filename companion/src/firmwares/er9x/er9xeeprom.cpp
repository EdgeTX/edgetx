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

#include <algorithm>
#include "er9xeeprom.h"
#include "helpers.h"
#include <QObject>

RawSwitch er9xToSwitch(int8_t sw)
{
  uint8_t swa = abs(sw);
  if (swa == 0)
    return RawSwitch(SWITCH_TYPE_NONE);
  else if (swa <= 9)
    return RawSwitch(SWITCH_TYPE_SWITCH, sw);
  else if (swa <= 21)
    return RawSwitch(SWITCH_TYPE_VIRTUAL, sw > 0 ? sw-9 : sw+9);
  else if (sw == 22)
    return RawSwitch(SWITCH_TYPE_ON);
  else if (sw == -22)
    return RawSwitch(SWITCH_TYPE_OFF);
  else if (swa <= 22+9)
    return RawSwitch(SWITCH_TYPE_SWITCH, sw > 0 ? sw-22 : sw+22);
  else
    return RawSwitch(SWITCH_TYPE_VIRTUAL, sw > 0 ? sw-22-9 : sw+22+9);
}

t_Er9xTrainerMix::t_Er9xTrainerMix()
{
  memset(this, 0, sizeof(t_Er9xTrainerMix));
}

t_Er9xTrainerMix::operator TrainerMix()
{
  TrainerMix c9x;
  c9x.src = srcChn;
  c9x.swtch = er9xToSwitch(swtch);
  c9x.weight = (25 * studWeight) / 8;
  c9x.mode = mode;
  return c9x;
}

t_Er9xTrainerData::t_Er9xTrainerData()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(t_Er9xTrainerData));
}

t_Er9xTrainerData::operator TrainerData ()
{
  TrainerData c9x;
  for (int i=0; i<CPN_MAX_STICKS; i++) {
    c9x.calib[i] = calib[i];
    c9x.mix[i] = mix[i];
  }
  return c9x;
}

t_Er9xGeneral::t_Er9xGeneral()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(t_Er9xGeneral));
}

Er9xGeneral::operator GeneralSettings ()
{
  GeneralSettings result;
  result.version = myVers;
  for (int i=0; i<CPN_MAX_STICKSnPOTS; i++) {
    result.calibMid[i] = calibMid[i];
    result.calibSpanNeg[i] = calibSpanNeg[i];
    result.calibSpanPos[i] = calibSpanPos[i];
  }

  result.currModelIndex = currModel;
  result.contrast = contrast;
  result.vBatWarn = vBatWarn;
  result.txVoltageCalibration = txVoltageCalibration;
  result.trainer = trainer;

  result.view = std::min((uint8_t)4, view);
  result.disableThrottleWarning = disableThrottleWarning;
  result.disableMemoryWarning = disableMemoryWarning;

  switch (beeperVal) {
    case 0:
      result.beeperMode = GeneralSettings::BEEPER_QUIET;
      break;
    case 1:
      result.beeperMode = GeneralSettings::BEEPER_NOKEYS;
      break;
    default:
      result.beeperMode = GeneralSettings::BEEPER_ALL;
      result.beeperLength = beeperVal - 4;
      break;
  }

  result.disableAlarmWarning = disableAlarmWarning;
  result.stickMode = std::max((uint8_t)0, std::min(stickMode, (uint8_t)3));
  result.inactivityTimer = inactivityTimer + 10;
  result.minuteBeep = minuteBeep;
  result.preBeep = preBeep;
  result.flashBeep = flashBeep;
  result.splashMode = disableSplashScreen;

  result.backlightMode = 0;
  if (lightSw == 22) {
    result.backlightMode = 4;
  }
  else if (lightAutoOff) {
    result.backlightMode |= 1;
    result.backlightDelay = lightAutoOff;
  }
  else if (lightOnStickMove) {
    result.backlightMode |= 2;
    result.backlightDelay = lightOnStickMove;
  }

  result.templateSetup = templateSetup;
  result.PPM_Multiplier = PPM_Multiplier;
  getEEPROMString(result.ownerName, ownerName, sizeof(ownerName));
  result.speakerPitch = speakerPitch;
  result.hapticStrength = hapticStrength - 3;
  result.speakerMode = speakerMode;
  return result;
}

t_Er9xLimitData::t_Er9xLimitData()
{
  memset(this, 0, sizeof(t_Er9xLimitData));
}

t_Er9xLimitData::operator LimitData ()
{
  LimitData c9x;
  c9x.min = 10*(min-100);
  c9x.max = 10*(max+100);
  c9x.revert = revert;
  c9x.offset = offset;
  return c9x;
}

t_Er9xMixData::t_Er9xMixData()
{
  memset(this, 0, sizeof(t_Er9xMixData));
}

t_Er9xMixData::operator MixData ()
{
  MixData c9x;
  c9x.destCh = destCh;
  if (abs(weight)>125) {
    int gvar;
    if (weight>0) {
      gvar=weight-125;
    } else {
      gvar=weight+131;
    }
    gvar+=10000;
    c9x.weight=gvar;
  } else {
    c9x.weight = weight;
  }
  if (abs(sOffset)>125) {
    int gvar;
    if (sOffset>0) {
      gvar=sOffset-125;
    } else {
      gvar=sOffset+131;
    }
    gvar+=10000;
    c9x.sOffset=gvar;
  } else {
    c9x.sOffset=sOffset;
  }
  c9x.swtch = er9xToSwitch(swtch);

  if (srcRaw == 0) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_NONE);
  }
  else if (srcRaw <= 7) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_INPUT, srcRaw-1);
  }
  else if (srcRaw == 8) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_MAX);
  }
  else if (srcRaw == 9) {
    if (swtch < 0) {
      c9x.srcRaw = RawSource(SOURCE_TYPE_SWITCH, swtch < -3 ? -swtch-3 : -swtch);
      c9x.weight = -weight;
    }
    else if (swtch > 0) {
      c9x.srcRaw = RawSource(SOURCE_TYPE_SWITCH, swtch > 3 ? swtch-3 : swtch);
    }
    else {
      c9x.srcRaw = RawSource(SOURCE_TYPE_MAX);
    }
    if (mltpx != MLTPX_REP)
      c9x.swtch = RawSwitch(SWITCH_TYPE_NONE);
  }
  else if (srcRaw <= 12) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_CYC, srcRaw-10);
  }
  else if (srcRaw <= 20) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_PPM, srcRaw-13);
  }
  else if (srcRaw <=37) {
    c9x.srcRaw = RawSource(SOURCE_TYPE_CH, srcRaw-21);
  }
  else {
    c9x.srcRaw = RawSource(SOURCE_TYPE_GVAR, srcRaw-38);
  }

  if (differential==1) {
    c9x.curve.type = CurveReference::CURVE_REF_DIFF;
    c9x.curve.value = differential;
  }
  else if (curve > 6) {
    c9x.curve.type = CurveReference::CURVE_REF_CUSTOM;
    c9x.curve.value = curve - 6;
  }
  else if (curve > 0) {
    c9x.curve.type = CurveReference::CURVE_REF_FUNC;
    c9x.curve.value = curve;
  }

  // c9x.lateOffset=lateOffset;
  c9x.delayUp = delayUp;
  c9x.delayDown = delayDown;
  c9x.speedUp = speedUp;
  c9x.speedDown = speedDown;
  c9x.carryTrim = carryTrim;
  c9x.mltpx = (MltpxValue)mltpx;
  c9x.mixWarn = mixWarn;
  // c9x.enableFmTrim=enableFmTrim;
  return c9x;
}

RawSource er9xToSource(int8_t value)
{
  if (value == 0) {
    return RawSource(SOURCE_TYPE_NONE);
  }
  else if (value <= 7) {
    return RawSource(SOURCE_TYPE_INPUT, value - 1);
  }
  else if (value == 8) {
    return RawSource(SOURCE_TYPE_MAX);
  }
  else if (value == 9) {
    return RawSource(SOURCE_TYPE_MAX);
  }
  else if (value <= 12) {
    return RawSource(SOURCE_TYPE_CYC, value-10);
  }
  else if (value <= 20) {
    return RawSource(SOURCE_TYPE_PPM, value-13);
  }
  else if (value <= 36) {
    return RawSource(SOURCE_TYPE_CH, value-21);
  }
  else {
    return RawSource(SOURCE_TYPE_TELEMETRY, value-36);
  }
}

Er9xLogicalSwitchData::operator LogicalSwitchData ()
{
  LogicalSwitchData c9x;
  c9x.func = func;
  c9x.val1 = v1;
  c9x.val2 = v2;

  if ((c9x.func >= LS_FN_VPOS && c9x.func <= LS_FN_ANEG) || c9x.func >= LS_FN_EQUAL) {
    c9x.val1 = er9xToSource(v1).toValue();
  }

  if (c9x.func >= LS_FN_EQUAL) {
    c9x.val2 = er9xToSource(v2).toValue();
  }

  if (c9x.func >= LS_FN_AND && c9x.func <= LS_FN_XOR) {
    c9x.val1 = er9xToSwitch(v1).toValue();
    c9x.val2 = er9xToSwitch(v2).toValue();
  }

  return c9x;
}


t_Er9xSafetySwData::t_Er9xSafetySwData()
{
  memset(this, 0, sizeof(t_Er9xSafetySwData));
}

t_Er9xFrSkyChannelData::t_Er9xFrSkyChannelData()
{
  memset(this, 0, sizeof(t_Er9xFrSkyChannelData));
}

t_Er9xFrSkyChannelData::operator FrSkyChannelData ()
{
  FrSkyChannelData c9x;
  c9x.ratio = ratio;
  c9x.alarms[0].value = alarms_value[0];
  c9x.alarms[0].level =  alarms_level & 3;
  c9x.alarms[0].greater = alarms_greater & 1;
  c9x.alarms[1].value = alarms_value[1];
  c9x.alarms[1].level =  (alarms_level >> 2) & 3;
  c9x.alarms[1].greater = (alarms_greater >> 1) & 1;
  if (type==2) {
    c9x.type = 0;
    c9x.multiplier=1;
  } else {
    c9x.type = type;
  }
  return c9x;
}

t_Er9xFrSkyData::t_Er9xFrSkyData()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(t_Er9xFrSkyData));
}

t_Er9xFrSkyData::operator FrSkyData ()
{
  FrSkyData c9x;
  c9x.channels[0] = channels[0];
  c9x.channels[1] = channels[1];
  return c9x;
}

RawSwitch getEr9xTimerMode(int mode)
{
  /*
  if (mode <= -33)
    return TimerMode(TMRMODE_FIRST_NEG_SWITCH+(mode+33));
  else if (mode <= -1)
    return TimerMode(TMRMODE_FIRST_NEG_SWITCH+(mode+1));
  else if (mode < 16)
    return TimerMode(mode);
  else if (mode < 16+21)
    return TimerMode(TMRMODE_FIRST_SWITCH+(mode-16));
  else
    return TimerMode(TMRMODE_FIRST_SWITCH+(mode-16-21));
    */
  return RawSwitch();
}

t_Er9xModelData::operator ModelData ()
{
  ModelData c9x;
  c9x.used = true;
  getEEPROMString(c9x.name, name, sizeof(name));
  c9x.timers[0].mode = getEr9xTimerMode(tmrMode);
  c9x.timers[0].val = tmrVal;
  switch(protocol) {
    case 1:
      c9x.moduleData[0].protocol = PULSES_PXX_DJT;
      break;
    case 2:
      c9x.moduleData[0].protocol = PULSES_DSM2;
      break;
    case 3:
      c9x.moduleData[0].protocol = PULSES_PPM16;
      break;
    default:
      c9x.moduleData[0].protocol = PULSES_PPM;
      break;
  }
  c9x.moduleData[0].ppm.frameLength=ppmFrameLength;
  c9x.moduleData[0].channelsCount = 8 + 2 * ppmNCH;
  c9x.thrTrim = thrTrim;
  c9x.trimInc = trimInc-2;
  c9x.moduleData[0].ppm.delay = 300 + 50 * ppmDelay;
  c9x.customFn[0].func = FuncInstantTrim;
  if (trimSw) {
    c9x.customFn[0].swtch = er9xToSwitch(trimSw);
  }
  c9x.beepANACenter = beepANACenter;
  c9x.moduleData[0].ppm.pulsePol = pulsePol;
  c9x.extendedLimits = extendedLimits;
  c9x.swashRingData.elevatorWeight = swashInvertELE ? -100 : 100;
  c9x.swashRingData.aileronWeight = swashInvertAIL ? -100 : 100;
  c9x.swashRingData.collectiveWeight = swashInvertCOL ? -100 : 100;
  c9x.swashRingData.type = swashType;
  c9x.swashRingData.collectiveSource = er9xToSource(swashCollectiveSource);
  c9x.swashRingData.value = swashRingValue;

  for (int i=0; i<ER9X_MAX_MIXERS; i++) {
    Er9xMixData mix = mixData[i];
    c9x.mixData[i] = mix;
  }

  for (int i=0; i<ER9X_NUM_CHNOUT; i++)
    c9x.limitData[i] = limitData[i];

  // expoData
  int e = 0;
  for (int ch = 0; ch < 4 && e < CPN_MAX_EXPOS; ch++) {
    for (int dr = 0, pos = 0; dr < 3 && e < CPN_MAX_EXPOS; dr++, pos++) {
      if ((dr == 0 && !expoData[ch].drSw1) || (dr == 1 && !expoData[ch].drSw2))
        dr = 2;
      if (dr == 2 && !expoData[ch].expo[0][0][0] && !expoData[ch].expo[0][0][1] && !expoData[ch].expo[0][1][0] && !expoData[ch].expo[0][1][1])
        break;
      if (expoData[ch].drSw1 && !expoData[ch].drSw2) {
        c9x.expoData[e].swtch = er9xToSwitch(dr == 0 ? expoData[ch].drSw1 : 0);
        pos = dr == 0 ? 1 : 0;
      }
      else {
        c9x.expoData[e].swtch = er9xToSwitch(dr == 0 ? -expoData[ch].drSw1 : (dr == 1 ? -expoData[ch].drSw2 : 0));
      }
      c9x.expoData[e].chn = ch;
      if (expoData[ch].expo[pos][0][0]) {
        c9x.expoData[e].curve.type = CurveReference::CURVE_REF_EXPO;
        c9x.expoData[e].curve.value = expoData[ch].expo[pos][0][0];
      }
      c9x.expoData[e].weight = 100 + expoData[ch].expo[pos][1][0];
      if (expoData[ch].expo[pos][0][0] == expoData[ch].expo[pos][0][1] && expoData[ch].expo[pos][1][0] == expoData[ch].expo[pos][1][1]) {
        c9x.expoData[e++].mode = 3;
      }
      else {
        c9x.expoData[e].mode = 2;
        if (e < CPN_MAX_EXPOS - 1) {
          c9x.expoData[e + 1].swtch = c9x.expoData[e].swtch;
          c9x.expoData[++e].chn = ch;
          c9x.expoData[e].mode = 1;
          if (expoData[ch].expo[pos][0][1]) {
            c9x.expoData[e].curve.type = CurveReference::CURVE_REF_EXPO;
            c9x.expoData[e].curve.value = expoData[ch].expo[pos][0][1];
          }
          c9x.expoData[e++].weight = 100 + expoData[ch].expo[pos][1][1];
        }
      }
    }
  }

  for (int i=0; i<CPN_MAX_STICKS; i++)
    c9x.flightModeData[0].trim[i] = trim[i];

  for (int i=0; i<ER9X_MAX_CURVE5; i++) {
    c9x.curves[i].count = 5;
    for (int j = 0; j < 5; j++) {
      c9x.curves[i].points[j].x = -100 + 50 * i;
      c9x.curves[i].points[j].y = curves5[i][j];
    }
  }
  for (int i=0; i<ER9X_MAX_CURVE9; i++) {
    c9x.curves[ER9X_MAX_CURVE5 + i].count = 9;
    for (int j = 0; j < 9; j++) {
      c9x.curves[ER9X_MAX_CURVE5 + i].points[j].x = -100 + 25 * i;
      c9x.curves[ER9X_MAX_CURVE5 + i].points[j].y = curves9[i][j];
    }
  }

  for (int i=0; i<ER9X_NUM_CSW; i++)
    c9x.logicalSw[i] = logicalSw[i];

  // for (int i=0; i<ER9X_NUM_CHNOUT; i++)
  //   c9x.safetySw[i] = safetySw[i];

  c9x.frsky = frsky;
  c9x.frsky.usrProto=FrSkyUsrProto;
  return c9x;
}

