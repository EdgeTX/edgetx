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

#include "opentx.h"
#include "switches.h"

#define CS_LAST_VALUE_INIT -32768

#if defined(COLORLCD)
  #define SWITCH_WARNING_LIST_X        WARNING_LINE_X
  #define SWITCH_WARNING_LIST_Y        WARNING_LINE_Y+3*FH
#elif LCD_W >= 212
  #define SWITCH_WARNING_LIST_X        60
  #define SWITCH_WARNING_LIST_Y        4*FH+4
#else
  #define SWITCH_WARNING_LIST_X        4
  #define SWITCH_WARNING_LIST_Y        4*FH+4
#endif

enum LogicalSwitchContextState {
  SWITCH_START,
  SWITCH_DELAY,
  SWITCH_ENABLE
};

PACK(struct LogicalSwitchContext {
  uint8_t state:1;
  uint8_t timerState:2;
  uint8_t spare:5;
  uint8_t timer;
  int16_t lastValue;
});

PACK(struct LogicalSwitchesFlightModeContext {
  LogicalSwitchContext lsw[MAX_LOGICAL_SWITCHES];
});
LogicalSwitchesFlightModeContext lswFm[MAX_FLIGHT_MODES];
CircularBuffer<uint8_t, 8> luaSetStickySwitchBuffer;

#define LS_LAST_VALUE(fm, idx) lswFm[fm].lsw[idx].lastValue

#if defined(PCBFRSKY) || defined(PCBFLYSKY)
#if defined(PCBX9E)
tmr10ms_t switchesMidposStart[16];
#else
tmr10ms_t switchesMidposStart[6]; // TODO constant
#endif
uint64_t  switchesPos = 0;
tmr10ms_t potsLastposStart[NUM_XPOTS];
uint8_t   potsPos[NUM_XPOTS];

#define SWITCH_POSITION(sw)  (switchesPos & ((MASK_CFN_TYPE)1<<(sw)))
#define POT_POSITION(sw)     ((potsPos[(sw)/XPOTS_MULTIPOS_COUNT] & 0x0f) == ((sw) % XPOTS_MULTIPOS_COUNT))

#if defined(FUNCTION_SWITCHES)
// Non pushed : SWSRC_Sx0 = -1024 = Sx(up) = state 0
// Pushed : SWSRC_Sx2 = +1024 = Sx(down) = state 1

uint8_t fsPreviousState = 0;

void setFSStartupPosition()
{
  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    uint8_t startPos = (g_model.functionSwitchStartConfig >> 2 * i) & 0x03;
    switch(startPos) {
      case FS_START_DOWN:
        g_model.functionSwitchLogicalState &= ~(1 << i);   // clear state
        break;

      case FS_START_UP:
        g_model.functionSwitchLogicalState |= 1 << i;
        break;

      case FS_START_PREVIOUS:
      default:
        // Do nothing, use existing g_model.functionSwitchLogicalState value
        break;
    }
  }
}

uint8_t getFSLogicalState(uint8_t index)
{
  return (uint8_t )(bfSingleBitGet(g_model.functionSwitchLogicalState, index) >> (index));
}

uint8_t getFSPhysicalState(uint8_t index)
{
  return switchState(((index + NUM_REGULAR_SWITCHES) * 3) + 2) ? 1 : 0;
}

uint8_t getFSPreviousPhysicalState(uint8_t index)
{
  return (uint8_t )(bfSingleBitGet(fsPreviousState, index) >> (index));
}

void evalFunctionSwitches()
{
  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    if (FSWITCH_CONFIG(i) == SWITCH_NONE) {
      fsLedOff(i);
      continue;
    }

    uint8_t physicalState = getFSPhysicalState(i);
    if (physicalState != getFSPreviousPhysicalState(i)) {      // FS was moved
      if ((FSWITCH_CONFIG(i) == SWITCH_2POS && physicalState == 1) || (FSWITCH_CONFIG(i) == SWITCH_TOGGLE)) {
        if (IS_FSWITCH_GROUP_ON(FSWITCH_GROUP(i)) != 0) { // In an always on group
          g_model.functionSwitchLogicalState |= 1 << i;   // Set bit
        }
        else {
          g_model.functionSwitchLogicalState ^= 1 << i;   // Toggle bit
        }
      }

      if (FSWITCH_GROUP(i) && physicalState == 1) {    // switch is in a group, other in group need to be turned off
        for (uint8_t j = 0; j < NUM_FUNCTIONS_SWITCHES; j++) {
          if (i == j)
            continue;
          if (FSWITCH_GROUP(j) == FSWITCH_GROUP(i)) {
            g_model.functionSwitchLogicalState &= ~(1 << j);   // clear state
          }
        }
      }

      fsPreviousState ^= 1 << i;    // Toggle state
      storageDirty(EE_MODEL);
    }

    if (!pwrPressed()) {
      if (getFSLogicalState(i))
        fsLedOn(i);
      else
        fsLedOff(i);
    }
  }
}
#endif

div_t switchInfo(int switchPosition)
{
  return div(switchPosition-SWSRC_FIRST_SWITCH, 3);
}

uint64_t check2PosSwitchPosition(uint8_t sw)
{
  uint32_t index = (switchState(sw) ? sw : sw + 2);
  uint64_t result = ((uint64_t)1 << index);

  if (!(switchesPos & result)) {
    PLAY_SWITCH_MOVED(index);
  }

  return result;
}

uint64_t check3PosSwitchPosition(uint8_t idx, uint8_t sw, bool startup)
{
  uint64_t result;
  uint32_t index;

  if (switchState(sw)) {
    index = sw;
    result = ((MASK_CFN_TYPE)1 << index);
    switchesMidposStart[idx] = 0;
  }
  else if (switchState(sw+2)) {
    index = sw + 2;
    result = ((MASK_CFN_TYPE)1 << index);
    switchesMidposStart[idx] = 0;
  }
  else {
    index = sw + 1;
    if (startup || SWITCH_POSITION(index) || g_eeGeneral.switchesDelay==SWITCHES_DELAY_NONE || (switchesMidposStart[idx] && (tmr10ms_t)(get_tmr10ms() - switchesMidposStart[idx]) > SWITCHES_DELAY())) {
      result = ((MASK_CFN_TYPE)1 << index);
      switchesMidposStart[idx] = 0;
    }
    else {
      result = (switchesPos & ((MASK_CFN_TYPE)0x7 << sw));
      if (!switchesMidposStart[idx]) {
        switchesMidposStart[idx] = get_tmr10ms();
      }
    }
  }

  if (!(switchesPos & result)) {
    PLAY_SWITCH_MOVED(index);
  }

  return result;
}

#define CHECK_2POS(sw)       newPos |= check2PosSwitchPosition(sw ## 0)
#define CHECK_3POS(idx, sw)  newPos |= check3PosSwitchPosition(idx, sw ## 0, startup)

void getSwitchesPosition(bool startup)
{
  uint64_t newPos = 0;
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  CHECK_2POS(SW_SA);
  CHECK_3POS(0, SW_SB);
  CHECK_3POS(1, SW_SC);
#elif defined(RADIO_TPRO)
  CHECK_3POS(0, SW_SA);
  CHECK_3POS(1, SW_SB);
  CHECK_2POS(SW_SC);
  CHECK_2POS(SW_SD);
#elif defined(PCBNV14)
  CHECK_2POS(SW_SA);
  CHECK_3POS(0, SW_SB);
  CHECK_2POS(SW_SC);
  CHECK_2POS(SW_SD);
#else
  CHECK_3POS(0, SW_SA);
  CHECK_3POS(1, SW_SB);
  CHECK_3POS(2, SW_SC);
#endif

#if defined(PCBX9LITES)
  CHECK_2POS(SW_SD);
  CHECK_2POS(SW_SE);
  CHECK_2POS(SW_SF);
  CHECK_2POS(SW_SG);
#elif defined(PCBX9LITE)
  CHECK_2POS(SW_SD);
  CHECK_2POS(SW_SE);
#elif defined(PCBXLITES)
  CHECK_3POS(3, SW_SD);
  CHECK_2POS(SW_SE);
  CHECK_2POS(SW_SF);
  // no SWG and SWH on XLITES
#elif defined(PCBXLITE)
  CHECK_3POS(3, SW_SD);
  // no SWE, SWF, SWG and SWH on XLITE
#elif defined(RADIO_ZORRO)
  CHECK_2POS(SW_SD);
  CHECK_2POS(SW_SE);
  CHECK_2POS(SW_SF);
  CHECK_2POS(SW_SG);
  CHECK_2POS(SW_SH);
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  CHECK_2POS(SW_SD);
  CHECK_3POS(2, SW_SE);
  CHECK_3POS(3, SW_SF);
#elif defined(RADIO_TPRO)
  CHECK_2POS(SW_SE);
  CHECK_2POS(SW_SF);
  CHECK_2POS(SW_SG);
  CHECK_2POS(SW_SH);
  CHECK_2POS(SW_SI);
  CHECK_2POS(SW_SJ);
#elif defined(PCBX7)
  CHECK_3POS(3, SW_SD);
  #if defined(HARDWARE_SWITCH_F)
    CHECK_2POS(SW_SF);
  #endif
  #if defined(HARDWARE_SWITCH_G)
    CHECK_2POS(SW_SG);
  #endif
  #if defined(HARDWARE_SWITCH_H)
    CHECK_2POS(SW_SH);
  #endif
#elif defined(PCBNV14)
  CHECK_2POS(SW_SE);
  CHECK_3POS(1, SW_SF);
  CHECK_3POS(2, SW_SG);
  CHECK_2POS(SW_SH);
#else
  CHECK_3POS(3, SW_SD);
  CHECK_3POS(4, SW_SE);
  CHECK_2POS(SW_SF);
  CHECK_3POS(5, SW_SG);
  CHECK_2POS(SW_SH);
#endif

#if defined(RADIO_X9DP2019)
  CHECK_2POS(SW_SI);
#endif

#if defined(PCBX7ACCESS)
  CHECK_2POS(SW_SI);
#elif defined(PCBHORUS) || (defined(PCBX7) && !defined(RADIO_ZORRO))
  CHECK_2POS(SW_SI);
  CHECK_2POS(SW_SJ);
#endif

#if defined(PCBX9E)
  CHECK_3POS(6, SW_SI);
  CHECK_3POS(7, SW_SJ);
  CHECK_3POS(8, SW_SK);
  CHECK_3POS(9, SW_SL);
  CHECK_3POS(10, SW_SM);
  CHECK_3POS(11, SW_SN);
  CHECK_3POS(12, SW_SO);
  CHECK_3POS(13, SW_SP);
  CHECK_3POS(14, SW_SQ);
  CHECK_3POS(15, SW_SR);
#endif

  switchesPos = newPos;

  for (int i=0; i<NUM_XPOTS; i++) {
    if (IS_POT_MULTIPOS(POT1+i)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[POT1+i];
      if (IS_MULTIPOS_CALIBRATED(calib)) {
        uint8_t pos = anaIn(POT1+i) / (2*RESX/calib->count);
        uint8_t previousPos = potsPos[i] >> 4;
        uint8_t previousStoredPos = potsPos[i] & 0x0F;
        if (startup) {
          potsPos[i] = (pos << 4) | pos;
        }
        else if (pos != previousPos) {
          potsLastposStart[i] = get_tmr10ms();
          potsPos[i] = (pos << 4) | previousStoredPos;
        }
        else if (g_eeGeneral.switchesDelay==SWITCHES_DELAY_NONE || (tmr10ms_t)(get_tmr10ms() - potsLastposStart[i]) > SWITCHES_DELAY()) {
          potsLastposStart[i] = 0;
          potsPos[i] = (pos << 4) | pos;
          if (previousStoredPos != pos) {
            PLAY_SWITCH_MOVED(SWSRC_LAST_SWITCH+i*XPOTS_MULTIPOS_COUNT+pos);
          }
        }
      }
    }
  }
}

getvalue_t getValueForLogicalSwitch(mixsrc_t i)
{
  getvalue_t result = getValue(i);
  if (i>=MIXSRC_FIRST_INPUT && i<=MIXSRC_LAST_INPUT) {
    int8_t trimIdx = virtualInputsTrims[i-MIXSRC_FIRST_INPUT];
    if (trimIdx >= 0) {
      int16_t trim = trims[trimIdx];
      if (trimIdx == THR_STICK && g_model.throttleReversed)
        result -= trim;
      else
        result += trim;
    }
  }
  return result;
}
#else
  #define getValueForLogicalSwitch(i) getValue(i)
#endif

PACK(typedef struct {
  uint8_t state;
  uint8_t last;
}) ls_sticky_struct;

PACK(typedef struct {
  uint16_t state:1;
  uint16_t duration:15;
}) ls_stay_struct;

bool getLogicalSwitch(uint8_t idx)
{
  LogicalSwitchData * ls = lswAddress(idx);
  bool result;

  swsrc_t s = ls->andsw;

  if (ls->func == LS_FUNC_NONE || (s && !getSwitch(s))) {
    if (ls->func != LS_FUNC_STICKY && ls->func != LS_FUNC_EDGE ) {
      // AND switch must not affect STICKY and EDGE processing
      LS_LAST_VALUE(mixerCurrentFlightMode, idx) = CS_LAST_VALUE_INIT;
    }
    result = false;
  }
  else if ((s=lswFamily(ls->func)) == LS_FAMILY_BOOL) {
    bool res1 = getSwitch(ls->v1);
    bool res2 = getSwitch(ls->v2);
    switch (ls->func) {
      case LS_FUNC_AND:
        result = (res1 && res2);
        break;
      case LS_FUNC_OR:
        result = (res1 || res2);
        break;
      // case LS_FUNC_XOR:
      default:
        result = (res1 ^ res2);
        break;
    }
  }
  else if (s == LS_FAMILY_TIMER) {
    result = (LS_LAST_VALUE(mixerCurrentFlightMode, idx) <= 0);
  }
  else if (s == LS_FAMILY_STICKY) {
    result = (LS_LAST_VALUE(mixerCurrentFlightMode, idx) & (1<<0));
  }
  else if (s == LS_FAMILY_EDGE) {
    result = (LS_LAST_VALUE(mixerCurrentFlightMode, idx) & (1<<0));
  }
  else {
    getvalue_t x = getValueForLogicalSwitch(ls->v1);
    getvalue_t y;
    if (s == LS_FAMILY_COMP) {
      y = getValueForLogicalSwitch(ls->v2);

      switch (ls->func) {
        case LS_FUNC_EQUAL:
          result = (x==y);
          break;
        case LS_FUNC_GREATER:
          result = (x>y);
          break;
        default:
          result = (x<y);
          break;
      }
    }
    else {
      mixsrc_t v1 = ls->v1;
      // Telemetry
      if (v1 >= MIXSRC_FIRST_TELEM) {
        if (!TELEMETRY_STREAMING() || IS_FAI_FORBIDDEN(v1-1)) {
          result = false;
          goto DurationAndDelayProcessing;
        }

        y = convertLswTelemValue(ls);


      }
      else if (v1 >= MIXSRC_GVAR1) {
        y = ls->v2;
      }
      else {
        y = calc100toRESX(ls->v2);
      }

      switch (ls->func) {
        case LS_FUNC_VEQUAL:
          result = (x==y);
          break;
        case LS_FUNC_VALMOSTEQUAL:
#if defined(GVARS)
          if (v1 >= MIXSRC_GVAR1 && v1 <= MIXSRC_LAST_GVAR)
            result = (x==y);
          else
#endif
          result = (abs(x-y) < (1024 / STICK_TOLERANCE));
          break;
        case LS_FUNC_VPOS:
          result = (x>y);
          break;
        case LS_FUNC_VNEG:
          result = (x<y);
          break;
        case LS_FUNC_APOS:
          result = (abs(x)>y);
          break;
        case LS_FUNC_ANEG:
          result = (abs(x)<y);
          break;
        default:
        {
          if (LS_LAST_VALUE(mixerCurrentFlightMode, idx) == CS_LAST_VALUE_INIT) {
            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x;
          }
          int16_t diff = x - LS_LAST_VALUE(mixerCurrentFlightMode, idx);
          bool update = false;
          if (ls->func == LS_FUNC_DIFFEGREATER) {
            if (y >= 0) {
              result = (diff >= y);
              if (diff < 0)
                update = true;
            }
            else {
              result = (diff <= y);
              if (diff > 0)
                update = true;
            }
          }
          else {
            result = (abs(diff) >= y);
          }
          if (result || update) {
            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x;
          }
          break;
        }
      }
    }
  }

DurationAndDelayProcessing:

    if (ls->delay || ls->duration) {
      LogicalSwitchContext &context = lswFm[mixerCurrentFlightMode].lsw[idx];
      if (result) {
        if (context.timerState == SWITCH_START) {
          // set delay timer
          context.timerState = SWITCH_DELAY;
          context.timer = (ls->func == LS_FUNC_EDGE ? 0 : ls->delay);
        }

        if (context.timerState == SWITCH_DELAY) {
          if (context.timer) {
            result = false;   // return false while delay timer running
          }
          else {
            // set duration timer
            context.timerState = SWITCH_ENABLE;
            context.timer = ls->duration;
          }
        }

        if (context.timerState == SWITCH_ENABLE) {
          result = (ls->duration==0 || context.timer>0); // return false after duration timer runs out
          if (!result && ls->func == LS_FUNC_STICKY) {
            ls_sticky_struct & lastValue = (ls_sticky_struct &)context.lastValue;
            lastValue.state = 0;
          }
        }
      }
      else if (context.timerState == SWITCH_ENABLE && ls->duration > 0 && context.timer > 0) {
        result = true;
      }
      else {
        context.timerState = SWITCH_START;
        context.timer = 0;
      }
    }

  return result;
}

bool getSwitch(swsrc_t swtch, uint8_t flags)
{
  bool result;

  if (swtch == SWSRC_NONE)
    return true;

  uint8_t cs_idx = abs(swtch);

  if (cs_idx == SWSRC_ONE) {
    result = !s_mixer_first_run_done;
  }
  else if (cs_idx == SWSRC_ON) {
    result = true;
  }
#if defined(DEBUG_LATENCY)
  else if (cs_idx == SWSRC_LATENCY_TOGGLE) {
    result = latencyToggleSwitch;
  }
#endif
  else if (cs_idx <= (SWSRC_LAST_SWITCH - 3 * NUM_FUNCTIONS_SWITCHES)) {
#if defined(PCBFRSKY) || defined(PCBFLYSKY)
    if (flags & GETSWITCH_MIDPOS_DELAY)
      result = SWITCH_POSITION(cs_idx-SWSRC_FIRST_SWITCH);
    else
      result = switchState(cs_idx-SWSRC_FIRST_SWITCH);
#else
    result = switchState(cs_idx-SWSRC_FIRST_SWITCH);
#endif
  }
#if defined(FUNCTION_SWITCHES)
  else if (cs_idx <= SWSRC_LAST_SWITCH) {
    div_t qr = div(cs_idx - 3 * NUM_FUNCTIONS_SWITCHES, 3);
    auto value = getFSLogicalState(qr.quot + 1);
    result = qr.rem == -2 ? 1 - value : value;
  }
#endif
#if NUM_XPOTS > 0
  else if (cs_idx <= SWSRC_LAST_MULTIPOS_SWITCH) {
    result = POT_POSITION(cs_idx-SWSRC_FIRST_MULTIPOS_SWITCH);
  }
#endif
  else if (cs_idx <= SWSRC_LAST_TRIM) {
    uint8_t idx = cs_idx - SWSRC_FIRST_TRIM;
    idx = (CONVERT_MODE_TRIMS(idx/2) << 1) + (idx & 1);
    result = trimDown(idx);
  }
  else if (cs_idx == SWSRC_RADIO_ACTIVITY) {
    result = (inactivity.counter < 2);
  }
  else if (cs_idx >= SWSRC_FIRST_SENSOR) {
    result = !telemetryItems[cs_idx-SWSRC_FIRST_SENSOR].isOld();
  }
  else if (cs_idx == SWSRC_TELEMETRY_STREAMING) {
    result = TELEMETRY_STREAMING();
  }
  else if (cs_idx >= SWSRC_FIRST_FLIGHT_MODE) {
#if defined(FLIGHT_MODES)
    uint8_t idx = cs_idx - SWSRC_FIRST_FLIGHT_MODE;
    if (flags & GETSWITCH_MIDPOS_DELAY)
      result = (idx == flightModeTransitionLast);
    else
      result = (idx == mixerCurrentFlightMode);
#else
    result = false;
#endif
   }
  else {
    cs_idx -= SWSRC_FIRST_LOGICAL_SWITCH;
    result = lswFm[mixerCurrentFlightMode].lsw[cs_idx].state;
  }

  return swtch > 0 ? result : !result;
}

/**
  @brief Calculates new state of logical switches for mixerCurrentFlightMode
*/
void evalLogicalSwitches(bool isCurrentFlightmode)
{
  for (unsigned int idx=0; idx<MAX_LOGICAL_SWITCHES; idx++) {
    LogicalSwitchContext & context = lswFm[mixerCurrentFlightMode].lsw[idx];
    bool result = getLogicalSwitch(idx);
    if (isCurrentFlightmode) {
      if (result) {
        if (!context.state) PLAY_LOGICAL_SWITCH_ON(idx);
      }
      else {
        if (context.state) PLAY_LOGICAL_SWITCH_OFF(idx);
      }
    }
    context.state = result;
  }
}

swarnstate_t switches_states = 0;
uint8_t fsswitches_states = 0;
swsrc_t getMovedSwitch()
{
  static tmr10ms_t s_move_last_time = 0;
  swsrc_t result = 0;

  // Switches
  for (int i = 0; i < NUM_SWITCHES - NUM_FUNCTIONS_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      swarnstate_t mask = ((swarnstate_t) 0x07 << (i * 3));
      uint8_t prev = (switches_states & mask) >> (i * 3);
      uint8_t next = (1024 + getValue(MIXSRC_SA + i)) / 1024 + 1;
      if (prev != next) {
        switches_states =
            (switches_states & (~mask)) | ((swarnstate_t)(next) << (i * 3));
        result = (3 * i) + next;
      }
    }
  }

#if defined(FUNCTION_SWITCHES)
  for (int i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    if (FSWITCH_CONFIG(i) != SWITCH_NONE) {
      auto prev = (uint8_t )(bfSingleBitGet(fsswitches_states, i) >> (i));
      uint8_t next = getFSLogicalState(i);
      if (prev != next) {
        switches_states ^= (-next ^ switches_states) & (1 << i);
        result = 2 + (3 * (i + NUM_REGULAR_SWITCHES)) + next;
      }
    }
  }
#endif

  // Multipos
  for (int i = 0; i < NUM_XPOTS; i++) {
    if (IS_POT_MULTIPOS(POT1 + i)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[POT1 + i];
      if (IS_MULTIPOS_CALIBRATED(calib)) {
        uint8_t prev = potsPos[i] & 0x0F;
        uint8_t next = anaIn(POT1 + i) / (2 * RESX / calib->count);
        if (prev != next) {
          result = SWSRC_LAST_SWITCH + i * XPOTS_MULTIPOS_COUNT + next + 1;
        }
      }
    }
  }

  if ((tmr10ms_t)(get_tmr10ms() - s_move_last_time) > 10)
    result = 0;

  s_move_last_time = get_tmr10ms();
  return result;
}

bool isSwitchWarningRequired(uint16_t &bad_pots)
{
  swarnstate_t states = g_model.switchWarningState;

  GET_ADC_IF_MIXER_NOT_RUNNING();
  getMovedSwitch();

  bool warn = false;
  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      swarnstate_t mask = ((swarnstate_t)0x07 << (i * 3));
      if ((states & mask) && !((states & mask) == (switches_states & mask))) {
        warn = true;
      }
    }
  }

  if (g_model.potsWarnMode) {
    evalFlightModeMixes(e_perout_mode_normal, 0);
    bad_pots = 0;
    for (int  i = 0; i < NUM_POTS + NUM_SLIDERS; i++) {
      if (!IS_POT_SLIDER_AVAILABLE(POT1 + i)) {
        continue;
      }
      if ((g_model.potsWarnEnabled & (1 << i)) &&
          (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1)) {
        warn = true;
        bad_pots |= (1 << i);
      }
    }
  }

  return warn;
}

#if defined(COLORLCD)
#include "switch_warn_dialog.h"
void checkSwitches()
{
  uint16_t bad_pots = 0;
  if (!isSwitchWarningRequired(bad_pots))
    return;

  LED_ERROR_BEGIN();
  auto dialog = new SwitchWarnDialog();
  dialog->runForever();
  LED_ERROR_END();
}
#elif defined(GUI)

void checkSwitches()
{
  swarnstate_t last_bad_switches = 0xff;
  swarnstate_t states = g_model.switchWarningState;
  uint16_t bad_pots = 0, last_bad_pots = 0xff;

#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  while (true) {

    if (!isSwitchWarningRequired(bad_pots))
      break;

    LED_ERROR_BEGIN();
    resetBacklightTimeout();

    // first - display warning
    if (last_bad_switches != switches_states || last_bad_pots != bad_pots) {
      drawAlertBox(STR_SWITCHWARN, nullptr, STR_PRESS_ANY_KEY_TO_SKIP);
      if (last_bad_switches == 0xff || last_bad_pots == 0xff) {
        AUDIO_ERROR_MESSAGE(AU_SWITCH_ALERT);
      }
      int x = SWITCH_WARNING_LIST_X;
      int y = SWITCH_WARNING_LIST_Y;
      int numWarnings = 0;
      for (int i=0; i<NUM_SWITCHES; ++i) {
        if (SWITCH_WARNING_ALLOWED(i)) {
          swarnstate_t mask = ((swarnstate_t)0x07 << (i*3));
          if (states & mask) {
            LcdFlags attr =
                ((states & mask) == (switches_states & mask)) ? 0 : INVERS;
            if (attr) {
              if (++numWarnings < 6) {
                const char* s = getSwitchWarnSymbol((states & mask) >> (i * 3));
                drawSource(x, y, MIXSRC_FIRST_SWITCH + i, attr);
                lcdDrawText(lcdNextPos, y, s, attr);
                x = lcdNextPos + 3;
              }
            }
          }
        }
      }

      if (g_model.potsWarnMode) {
        for (int i=0; i<NUM_POTS+NUM_SLIDERS; i++) {
          if (!IS_POT_SLIDER_AVAILABLE(POT1+i)) {
            continue;
          }
          if (g_model.potsWarnEnabled & (1 << i)) {
            if (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1) {
              if (++numWarnings < 6) {
                lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS + 1 + i, INVERS);
                if (IS_POT(POT1 + i))
                  lcdDrawChar(
                      lcdNextPos, y,
                      g_model.potsWarnPosition[i] > GET_LOWRES_POT_POSITION(i)
                          ? 126
                          : 127,
                      INVERS);  // TODO: use constants for chars
                else
                  lcdDrawText(
                      lcdNextPos, y,
                      g_model.potsWarnPosition[i] > GET_LOWRES_POT_POSITION(i)
                          ? STR_CHAR_UP
                          : STR_CHAR_DOWN,
                      INVERS);
                x = lcdNextPos + 3;
              }
            }
          }
        }
      }

      if (numWarnings >= 6) {
        lcdDrawText(x, y, "...", 0);
      }

      last_bad_pots = bad_pots;

      lcdRefresh();
      lcdSetContrast();
      waitKeysReleased();

      last_bad_switches = switches_states;
    }

    if (keyDown())
      break;

#if defined(PWR_BUTTON_PRESS)
    uint32_t power = pwrCheck();
    if (power == e_power_off) {
      drawSleepBitmap();
      boardOff();
      break;
    }
    else if (power == e_power_press) {
      refresh = true;
    }
    else if (power == e_power_on && refresh) {
      last_bad_switches = 0xff;
      last_bad_pots = 0xff;
      refresh = false;
    }
#else
    if (pwrCheck() == e_power_off) {
      break;
    }
#endif

    checkBacklight();

    WDG_RESET();

    RTOS_WAIT_MS(10);
  }

  LED_ERROR_END();
}
#endif // GUI

void logicalSwitchesTimerTick()
{
#if (MAX_LOGICAL_SWITCHES != 64)
#warning "The following code assumes that MAX_LOGICAL_SWITCHES == 64!"
#endif
  // Read messages from Lua in the buffer and flick switches
  uint8_t msg = luaSetStickySwitchBuffer.read();
  while(msg) {
    uint8_t i = msg & 0x3F;
    uint8_t s = msg >> 7;
    LogicalSwitchData * ls = lswAddress(i);
    if (ls->func == LS_FUNC_STICKY) {
      for (uint8_t fm=0; fm<MAX_FLIGHT_MODES; fm++) {
        ls_sticky_struct & lastValue = (ls_sticky_struct &)LS_LAST_VALUE(fm, i);
        lastValue.state = s;
        bool now;
        if (s)
          now = getSwitch(ls->v2);
        else
          now = getSwitch(ls->v1);
        if (now)
          lastValue.last |= 1;
        else
          lastValue.last &= ~1;
      }
    }
    msg = luaSetStickySwitchBuffer.read();
  }

  // Update logical switches
  for (uint8_t fm=0; fm<MAX_FLIGHT_MODES; fm++) {
    for (uint8_t i=0; i<MAX_LOGICAL_SWITCHES; i++) {
      LogicalSwitchData * ls = lswAddress(i);
      if (ls->func == LS_FUNC_TIMER) {
        int16_t * lastValue = &LS_LAST_VALUE(fm, i);
        if (*lastValue == 0 || *lastValue == CS_LAST_VALUE_INIT) {
          *lastValue = -lswTimerValue(ls->v1);
        }

        else if (*lastValue < 0) {
          if (++(*lastValue) == 0)
            *lastValue = lswTimerValue(ls->v2);
        }
        else { // if (*lastValue > 0)
          *lastValue -= 1;
        }
      }
      else if (ls->func == LS_FUNC_STICKY) {
        ls_sticky_struct & lastValue = (ls_sticky_struct &)LS_LAST_VALUE(fm, i);
        bool before = lastValue.last & 0x01;
        if (lastValue.state) {
            if (ls->v2 != SWSRC_NONE) { // only if used / source set
                bool now = getSwitch(ls->v2);
                if (now != before) {
                  lastValue.last ^= 1;
                  if (!before) {
                    lastValue.state = 0;
                  }
                }
            }
        }
        else {
            if (ls->v1 != SWSRC_NONE) { // only if used / source set
                bool now = getSwitch(ls->v1);
                if (before != now) {
                  lastValue.last ^= 1;
                  if (!before) {
                    lastValue.state = 1;
                  }
                }
            }
        }
      }
      else if (ls->func == LS_FUNC_EDGE) {
        ls_stay_struct & lastValue = (ls_stay_struct &)LS_LAST_VALUE(fm, i);
        // if this ls was reset by the logicalSwitchesReset() the lastValue will be set to CS_LAST_VALUE_INIT(0x8000)
        // when it is unpacked into ls_stay_struct the lastValue.duration will have a value of 0x4000
        // this will produce an instant true for edge logical switch if the second parameter is big enough.
        // So we reset it here.
        if (LS_LAST_VALUE(fm, i) == CS_LAST_VALUE_INIT) {
          lastValue.duration = 0;
        }
        lastValue.state = false;
        bool state = getSwitch(ls->v1);
        if (state) {
          if (ls->v3 == -1 && lastValue.duration == lswTimerValue(ls->v2))
            lastValue.state = true;
          if (lastValue.duration < 1000)
            lastValue.duration++;
        }
        else {
          if (lastValue.duration > lswTimerValue(ls->v2) && (ls->v3 == 0 || lastValue.duration <= lswTimerValue(ls->v2+ls->v3)))
            lastValue.state = true;
          lastValue.duration = 0;
        }
      }

      // decrement delay/duration timer
      LogicalSwitchContext &context = lswFm[fm].lsw[i];
      if (context.timer) {
        context.timer--;
      }
    }
  }
}

LogicalSwitchData * lswAddress(uint8_t idx)
{
  return &g_model.logicalSw[idx];
}

uint8_t lswFamily(uint8_t func)
{
  if (func <= LS_FUNC_ANEG)
    return LS_FAMILY_OFS;
  else if (func <= LS_FUNC_XOR)
    return LS_FAMILY_BOOL;
  else if (func == LS_FUNC_EDGE)
    return LS_FAMILY_EDGE;
  else if (func <= LS_FUNC_LESS)
    return LS_FAMILY_COMP;
  else if (func <= LS_FUNC_ADIFFEGREATER)
    return LS_FAMILY_DIFF;
  else
    return LS_FAMILY_TIMER+func-LS_FUNC_TIMER;
}

// val = [-129,-110] => [0,19]     (step  1)
// val = [-109,6]    => [20,595]   (step  5)
// val = [7,122]     => [600,1750] (step 10)
//
int16_t lswTimerValue(delayval_t val)
{
  return (val < -109 ? 129+val : (val < 7 ? (113+val)*5 : (53+val)*10));
}

void logicalSwitchesReset()
{
  memset(lswFm, 0, sizeof(lswFm));

  for (uint8_t fm=0; fm<MAX_FLIGHT_MODES; fm++) {
    for (uint8_t i=0; i<MAX_LOGICAL_SWITCHES; i++) {
      LS_LAST_VALUE(fm, i) = CS_LAST_VALUE_INIT;
    }
  }

  luaSetStickySwitchBuffer.clear();
}

getvalue_t convertLswTelemValue(LogicalSwitchData * ls)
{
  getvalue_t val;
  val = convert16bitsTelemValue(ls->v1 - MIXSRC_FIRST_TELEM + 1, ls->v2);
  return val;
}

void logicalSwitchesCopyState(uint8_t src, uint8_t dst)
{
  lswFm[dst] = lswFm[src];
}
