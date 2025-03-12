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

#include "hal/switch_driver.h"
#include "hal/adc_driver.h"
#include "hal/rgbleds.h"

#include "myeeprom.h"
#include "edgetx.h"
#include "edgetx_constants.h"
#include "switches.h"
#include "input_mapping.h"
#include "inactivity_timer.h"
#include "tasks/mixer_task.h"

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
  uint8_t spare:1;
  uint8_t deltaTimer:4; // Timer for holding delta function state change active
  uint8_t timer;
  int16_t lastValue;
});

PACK(struct LogicalSwitchesFlightModeContext {
  LogicalSwitchContext lsw[MAX_LOGICAL_SWITCHES];
});
LogicalSwitchesFlightModeContext lswFm[MAX_FLIGHT_MODES];
CircularBuffer<uint8_t, 8> luaSetStickySwitchBuffer;

#define LS_LAST_VALUE(fm, idx) lswFm[fm].lsw[idx].lastValue

tmr10ms_t switchesMidposStart[MAX_SWITCHES];
uint64_t  switchesPos = 0;

static_assert(sizeof(uint64_t) * 8 >= ((MAX_SWITCHES - 1) / 2) + 1,
              "MAX_SWITCHES too big for uint64_t position state");

tmr10ms_t potsLastposStart[MAX_POTS];
uint8_t   potsPos[MAX_POTS];

#define SWITCH_POSITION(sw) (switchesPos & ((MASK_CFN_TYPE)1 << (sw)))
#define POT_POSITION(sw)                            \
  ((potsPos[(sw) / XPOTS_MULTIPOS_COUNT] & 0x0f) == \
   ((sw) % XPOTS_MULTIPOS_COUNT))

#if defined(FUNCTION_SWITCHES)
// Customizable switches
// 
// Non pushed : SWSRC_Sx0 = -1024 = Sx(up) = state 0
// Pushed : SWSRC_Sx2 = +1024 = Sx(down) = state 1

uint8_t fsPreviousState = 0;
uint8_t functionSwitchFunctionState = 0;

void setFSStartupPosition()
{
  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    uint8_t startPos = (g_model.functionSwitchStartConfig >> 2 * i) & 0x03;
    if (FSWITCH_CONFIG(i) == SWITCH_TOGGLE)
      startPos = FS_START_OFF;
    switch(startPos) {
      case FS_START_OFF:
        g_model.functionSwitchLogicalState &= ~(1 << i);   // clear state
        break;

      case FS_START_ON:
        g_model.functionSwitchLogicalState |= 1 << i;
        break;

      case FS_START_PREVIOUS:
      default:
        // Do nothing, use existing g_model.functionSwitchLogicalState value
        break;
    }
  }
}

uint8_t getFSLogicalState()
{
  return g_model.functionSwitchLogicalState;
}

uint8_t getFSLogicalState(uint8_t index)
{
  return (uint8_t )(bfSingleBitGet(getFSLogicalState(), index) >> (index));
}

void setFSLogicalState(uint8_t index, uint8_t value)
{
  if (value)
    g_model.functionSwitchLogicalState |= 1 << index;  // Set bit
  else
    g_model.functionSwitchLogicalState &= ~(1 << index);  // clear state
}

uint8_t getFSPhysicalState(uint8_t index)
{
#if defined(FUNCTION_SWITCHES)
  if (bfSingleBitGet(functionSwitchFunctionState, index))
    return true;
#endif

  index += switchGetMaxSwitches();
  return switchGetPosition(index) != SWITCH_HW_UP;
}

uint8_t getFSPreviousPhysicalState(uint8_t index)
{
  return (uint8_t )(bfSingleBitGet(fsPreviousState, index) >> (index));
}

uint8_t getSwitchCountInFSGroup(uint8_t index)
{
  uint8_t count = 0;

  for (uint8_t i = 0; i < switchGetMaxFctSwitches(); i++) {
    if (FSWITCH_GROUP(i) == index)
      count++;
  }

  return count;
}

bool isFSGroupUsed(uint8_t index)
{
  return getSwitchCountInFSGroup(index) != 0;
}

void evalFunctionSwitches()
{
  uint8_t fct_switches = switchGetMaxFctSwitches();
  for (uint8_t i = 0; i < fct_switches; i++) {
    if (FSWITCH_CONFIG(i) == SWITCH_NONE) {
      setFSLedOFF(i);
      //fsLedRGB(i, 0);
      continue;
    }

    uint8_t physicalState = getFSPhysicalState(i);
    if (physicalState != getFSPreviousPhysicalState(i)) {
      // FS was moved
      inactivityTimerReset(ActivitySource::MainControls);
      if ((FSWITCH_CONFIG(i) == SWITCH_2POS && physicalState == 1) ||
          (FSWITCH_CONFIG(i) == SWITCH_TOGGLE)) {
        if (IS_FSWITCH_GROUP_ON(FSWITCH_GROUP(i)) != 0) {
          // In an always on group
          g_model.functionSwitchLogicalState |= 1 << i;  // Set bit
        } else {
          g_model.functionSwitchLogicalState ^= 1 << i;  // Toggle bit
        }
      }

      if (FSWITCH_GROUP(i) && physicalState == 1) {
        // switch is in a group, other in group need to be turned off
        for (uint8_t j = 0; j < NUM_FUNCTIONS_SWITCHES; j++) {
          if (i == j) continue;
          if (FSWITCH_GROUP(j) == FSWITCH_GROUP(i)) {
            g_model.functionSwitchLogicalState &= ~(1 << j);  // clear state
          }
        }
      }

      fsPreviousState ^= 1 << i;  // Toggle state
      storageDirty(EE_MODEL);
    }

    if (!pwrPressed()) {
      if (getFSLogicalState(i))
        setFSLedON(i);
      else
        setFSLedOFF(i);
    }
  }
}

bool groupHasSwitchOn(uint8_t group)
{
  for (int j = 0; j < NUM_FUNCTIONS_SWITCHES; j += 1)
    if (FSWITCH_GROUP(j) == group && getFSLogicalState(j))
      return true;
  return false;
}

int firstSwitchInGroup(uint8_t group)
{
  for (int j = 0; j < NUM_FUNCTIONS_SWITCHES; j += 1)
    if (FSWITCH_GROUP(j) == group)
      return j;
  return -1;
}

int groupDefaultSwitch(uint8_t group)
{
  bool allOff = true;
  for (int j = 0; j < NUM_FUNCTIONS_SWITCHES; j += 1) {
    if (FSWITCH_GROUP(j) == group) {
      if (FSWITCH_STARTUP(j) == FS_START_ON)
        return j;
      if (FSWITCH_STARTUP(j) != FS_START_OFF)
        allOff = false;
    }
  }
  if (allOff)
    return NUM_FUNCTIONS_SWITCHES;
  return -1;
}

void setGroupSwitchState(uint8_t group, int defaultSwitch)
{
  // Check rules for always on group
  //  - Toggle switch type not valid, change all switches to 2POS
  //  - One switch must be turned on, turn on first switch if needed
  if (IS_FSWITCH_GROUP_ON(group)) {
    for (int j = 0; j < NUM_FUNCTIONS_SWITCHES; j += 1) {
      if (FSWITCH_GROUP(j) == group) {
        FSWITCH_SET_CONFIG(j, SWITCH_2POS); // Toggle not valid
      }
    }
    if (!groupHasSwitchOn(group)) {
      int sw = firstSwitchInGroup(group);
      if (sw >= 0)
        setFSLogicalState(sw, 1); // Make sure a switch is on
    }
    if (groupDefaultSwitch(group) == NUM_FUNCTIONS_SWITCHES) {
      // Start state for all switches is off - set all to 'last'
      for (int j = 0; j < NUM_FUNCTIONS_SWITCHES; j += 1)
        if (FSWITCH_GROUP(j) == group)
          FSWITCH_SET_STARTUP(j, FS_START_PREVIOUS);
    }
  }
}
#else
uint8_t getFSLogicalState(uint8_t) { return false; }
#endif // FUNCTION_SWITCHES

div_t switchInfo(int switchPosition)
{
  return div(switchPosition - SWSRC_FIRST_SWITCH, 3);
}

int switchLookupIdx(char c)
{
  uint8_t idx = 1; // Sx
  if (c >= '1' && c <= '9') {
      idx = 2; // SWx
  }

  auto max_switches = switchGetMaxSwitches() + switchGetMaxFctSwitches();
  for (int i = 0; i < max_switches; i++) {
    const char *name = switchGetName(i);
    if (name[idx] == c) return i;
  }

  return -1;
}

int switchLookupIdx(const char* name, size_t len)
{
  if (len < 2 || (name[0] != 'S' && name[0] != 'F')) return -1;

  auto max_switches = switchGetMaxSwitches() + switchGetMaxFctSwitches();
  for (int i = 0; i < max_switches; i++) {
    const char *sw_name = switchGetCanonicalName(i);
    if (strncmp(sw_name, name, len) == 0) return i;
  }

  return -1;  
}

char switchGetLetter(uint8_t idx)
{
  if (idx >= switchGetMaxSwitches() + switchGetMaxFctSwitches())
    return -1;

  uint8_t c = 1;
  if (idx >= switchGetMaxSwitches() || switchIsFlex(idx)) c = 2;
  
  const char* name = switchGetName(idx);
  if (!name) return -1;
  
  return name[c];
}

void switchSetCustomName(uint8_t idx, const char* str, size_t len)
{
  strncpy(g_eeGeneral.switchNames[idx], str, min<size_t>(LEN_SWITCH_NAME, len));
  g_eeGeneral.switchNames[idx][LEN_SWITCH_NAME] = '\0';  
}

const char* switchGetCustomName(uint8_t idx)
{
#if defined(FUNCTION_SWITCHES)
  if (idx >= switchGetMaxSwitches()) // Switch is a customisable switch
    return g_model.switchNames[idx - switchGetMaxSwitches()];
  else
#endif
    return g_eeGeneral.switchNames[idx];
}

bool switchHasCustomName(uint8_t idx)
{
  return *switchGetCustomName(idx) != 0;
}

const char* switchGetCanonicalName(uint8_t idx)
{
  return switchGetName(idx);
}

const char* fsSwitchGroupGetCanonicalName(uint8_t idx)
{
  static const char fsgroupname[3][4] = {"GR1", "GR2", "GR3"};

  return &fsgroupname[idx][0];
}

SwitchConfig switchGetMaxType(uint8_t idx)
{
  auto hw_type = switchGetHwType(idx);
  if (hw_type == SWITCH_HW_2POS) {
    return SWITCH_2POS;
  } else {
    return SWITCH_3POS;
  }
}

static uint64_t checkSwitchPosition(uint8_t idx, bool startup)
{
  uint64_t result = 0;
  uint32_t index = idx * 3;

  auto pos = switchGetPosition(idx);
  switch(pos) {

  case SWITCH_HW_UP:
    result = ((MASK_CFN_TYPE)1 << index);
    switchesMidposStart[idx] = 0;
    break;

  case SWITCH_HW_DOWN:
    index += 2;
    result = ((MASK_CFN_TYPE)1 << index);
    switchesMidposStart[idx] = 0;
    break;

  case SWITCH_HW_MID:
    if (startup || SWITCH_POSITION(index + 1) ||
        g_eeGeneral.switchesDelay == SWITCHES_DELAY_NONE ||
        (switchesMidposStart[idx] &&
         (tmr10ms_t)(get_tmr10ms() - switchesMidposStart[idx]) >
         SWITCHES_DELAY())) {
      index += 1;
      result = ((MASK_CFN_TYPE)1 << index);
      switchesMidposStart[idx] = 0;
    } else {
      result = (switchesPos & ((MASK_CFN_TYPE)0x7 << index));
      if (!switchesMidposStart[idx]) {
        switchesMidposStart[idx] = get_tmr10ms();
      }
    }
    break;
  }

  if (!(switchesPos & result)) {
    PLAY_SWITCH_MOVED(index);
  }

  return result;
}

void getSwitchesPosition(bool startup)
{
  uint64_t newPos = 0;
  for (unsigned i = 0; i < switchGetMaxSwitches(); i++) {
    if (!SWITCH_EXISTS(i)) continue;
    newPos |= checkSwitchPosition(i, startup);
  }
  
  switchesPos = newPos;

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  auto offset = adcGetInputOffset(ADC_INPUT_FLEX);

  for (int i = 0; i < max_pots; i++) {
    if (IS_POT_MULTIPOS(i)) {
      auto analog_idx = offset + i;
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[analog_idx];
      if (IS_MULTIPOS_CALIBRATED(calib)) {
        uint8_t pos = anaIn(analog_idx) / (2 * RESX / calib->count);
        uint8_t previousPos = potsPos[i] >> 4;
        uint8_t previousStoredPos = potsPos[i] & 0x0F;
        if (startup) {
          potsPos[i] = (pos << 4) | pos;
        }
        else if (pos != previousPos) {
          potsLastposStart[i] = get_tmr10ms();
          potsPos[i] = (pos << 4) | previousStoredPos;
        } else if (g_eeGeneral.switchesDelay == SWITCHES_DELAY_NONE ||
                   (tmr10ms_t)(get_tmr10ms() - potsLastposStart[i]) >
                       SWITCHES_DELAY()) {
          potsLastposStart[i] = 0;
          potsPos[i] = (pos << 4) | pos;
          if (previousStoredPos != pos) {
            PLAY_SWITCH_MOVED(SWSRC_LAST_SWITCH + i * XPOTS_MULTIPOS_COUNT + pos);
          }
        }
      }
    }
  }
}

uint8_t getSwitchCount()
{
  int count = 0;
  for (int i = 0; i < switchGetMaxSwitches(); ++i) {
    if (SWITCH_EXISTS(i)) {
      ++count;
    }
  }
  return count;
}

uint8_t switchGetMaxRow(uint8_t col)
{
  uint8_t lastrow = 0;
  for (int i = 0; i < switchGetMaxSwitches(); ++i) {
    if (SWITCH_EXISTS(i)) {
      auto switch_display = switchGetDisplayPosition(i);
      if (switch_display.col == col)
        lastrow = switch_display.row > lastrow ? switch_display.row : lastrow;
    }
  }
  return lastrow;
}

getvalue_t getValueForLogicalSwitch(mixsrc_t i)
{
  getvalue_t result = getValue(i);
  if (i>=MIXSRC_FIRST_INPUT && i<=MIXSRC_LAST_INPUT) {
    int8_t trimIdx = virtualInputsTrims[i-MIXSRC_FIRST_INPUT];
    if (trimIdx >= 0) {
      int16_t trim = trims[trimIdx];
      if (trimIdx == inputMappingConvertMode(inputMappingGetThrottle()) && g_model.throttleReversed)
        result -= trim;
      else
        result += trim;
    }
  }
  return result;
}

PACK(typedef struct {
  uint8_t state;
  uint8_t last;
}) ls_sticky_struct;

PACK(typedef struct {
  uint16_t state:1;
  uint16_t duration:15;
}) ls_stay_struct;

bool getLSStickyState(uint8_t idx)
{
  return lswFm[mixerCurrentFlightMode].lsw[idx].lastValue & 1;
}

void logicalSwitchesInit(bool force)
{
  for (unsigned int idx=0; idx<MAX_LOGICAL_SWITCHES; idx++) {
    LogicalSwitchData * ls = lswAddress(idx);
    if (ls->func == LS_FUNC_STICKY && (force || ls->lsPersist)) {
      lswFm[mixerCurrentFlightMode].lsw[idx].lastValue = ls->lsState;
    }
  }
}

bool getLogicalSwitch(uint8_t idx)
{
  LogicalSwitchData * ls = lswAddress(idx);
  LogicalSwitchContext &context = lswFm[mixerCurrentFlightMode].lsw[idx];
  bool result;

  swsrc_t s = ls->andsw;

  if (ls->func == LS_FUNC_NONE || (s && !getSwitch(s))) {
    if (ls->func != LS_FUNC_STICKY && ls->func != LS_FUNC_EDGE ) {
      // AND switch must not affect STICKY and EDGE processing
      context.lastValue = CS_LAST_VALUE_INIT;
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
    result = (context.lastValue <= 0);
  }
  else if (s == LS_FAMILY_STICKY) {
    result = (context.lastValue & (1<<0));
  }
  else if (s == LS_FAMILY_EDGE) {
    result = (context.lastValue & (1<<0));
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
      else if (v1 >= MIXSRC_FIRST_GVAR) {
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
          if (v1 >= MIXSRC_FIRST_GVAR && v1 <= MIXSRC_LAST_GVAR)
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
          if (context.lastValue == CS_LAST_VALUE_INIT) {
            context.lastValue = x;
          }
          int16_t diff = x - context.lastValue;
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
          if (result) {
            context.deltaTimer = 10;
          } else if (context.deltaTimer > 0) {
            // Hold active state for 100ms to ensure state change is seen
            context.deltaTimer -= 1;
            result = true;
          }
          if (result || update) {
            context.lastValue = x;
          }
          break;
        }
      }
    }
  }

DurationAndDelayProcessing:

    if (ls->delay || ls->duration) {
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

  uint16_t cs_idx = abs(swtch);

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
  else if (cs_idx <= SWSRC_LAST_SWITCH) {
    cs_idx -= SWSRC_FIRST_SWITCH;
#if defined(FUNCTION_SWITCHES)
    auto max_reg_pos = switchGetMaxSwitches() * 3;
    if (cs_idx >= max_reg_pos && cs_idx - max_reg_pos < switchGetMaxFctSwitches() * 3) {
      cs_idx -= max_reg_pos;
      div_t qr = div(cs_idx, 3);
      auto value = getFSLogicalState(qr.quot);
      result = qr.rem == 0 ? !value : (qr.rem == 2 ? value : false);
    } else
#endif
    {
      if (flags & GETSWITCH_MIDPOS_DELAY) {
        result = SWITCH_POSITION(cs_idx);
      } else {
        div_t qr = div(cs_idx, 3);
        if (SWITCH_EXISTS(qr.quot)) {
          result = switchState(cs_idx);
        } else {
          result = false;
        }
      }
    }
  }
  else if (cs_idx <= SWSRC_LAST_MULTIPOS_SWITCH) {
    result = POT_POSITION(cs_idx - SWSRC_FIRST_MULTIPOS_SWITCH);
  }
  else if (cs_idx <= SWSRC_LAST_TRIM) {
    uint8_t idx = cs_idx - SWSRC_FIRST_TRIM;
    idx = (inputMappingConvertMode(idx/2) << 1) + (idx & 1);
    result = trimDown(idx);
  }
  else if (cs_idx == SWSRC_RADIO_ACTIVITY) {
    result = (inactivity.counter < 2);
  }
  else if (cs_idx == SWSRC_TRAINER_CONNECTED) {
    result = isTrainerConnected();
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

uint8_t getXPotPosition(uint8_t idx)
{
  if (idx >= MAX_POTS || !IS_POT_MULTIPOS(idx)) return 0;
  return potsPos[idx] & 0x0F;
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
    if ((g_model.logicalSw[idx].func == LS_FUNC_STICKY) && (g_model.logicalSw[idx].lsState != result)) {
      g_model.logicalSw[idx].lsState = result;
      storageDirty(EE_MODEL);
    }
  }
}

static inline uint8_t _bits_set(uint8_t val, uint8_t bits)
{
  uint8_t bits_set = 0;
  do {
    if (val & 1) ++bits_set;
    val >>= 1;
  } while (--bits);

  return bits_set;
}

swarnstate_t switches_states = 0;
uint8_t fsswitches_states = 0;

swsrc_t getMovedSwitch()
{
  static tmr10ms_t s_move_last_time = 0;
  swsrc_t result = 0;

  // Switches
  auto max_reg_switches = switchGetMaxSwitches();
  for (uint8_t i = 0; i < max_reg_switches; i++) {
    if (SWITCH_EXISTS(i)) {
      swarnstate_t mask = ((swarnstate_t) 0x07 << (i * 3));
      uint8_t prev = (switches_states & mask) >> (i * 3);
      uint8_t next = (1024 + getValue(MIXSRC_FIRST_SWITCH + i)) / 1024 + 1;
      if (prev != next) {
        switches_states =
            (switches_states & (~mask)) | ((swarnstate_t)(next) << (i * 3));
        result = (3 * i) + next;
      }
    }
  }

#if defined(FUNCTION_SWITCHES)
  auto fsswitches_cur_state = getFSLogicalState();
  auto fsswitches_xor = fsswitches_cur_state ^ fsswitches_states;
  fsswitches_states = fsswitches_cur_state;

  if (fsswitches_xor) {
    // we have some change...
    if (_bits_set(fsswitches_xor, switchGetMaxFctSwitches()) > 1) {
      // multiple bits change: use the one that turned ON
      fsswitches_xor &= fsswitches_cur_state;
    } else {
      // only one bit changed: use the one that changed
    }

    auto sw = __builtin_ctz(fsswitches_xor);
    auto pos = (fsswitches_cur_state & fsswitches_xor) ? 2 : 0;
    result = (max_reg_switches + sw) * 3 + pos + SWSRC_FIRST_SWITCH;
  }
#endif

  // Multipos
  for (int i = 0; i < MAX_POTS; i++) {
    if (IS_POT_MULTIPOS(i)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[MAX_STICKS + i];
      if (IS_MULTIPOS_CALIBRATED(calib)) {
        uint8_t prev = potsPos[i] & 0x0F;
        uint8_t next = anaIn(MAX_STICKS + i) / (2 * RESX / calib->count);
        if (prev != next) {
          result = SWSRC_FIRST_MULTIPOS_SWITCH + i * XPOTS_MULTIPOS_COUNT + next;
        }
      }
    }
  }

  if ((tmr10ms_t)(get_tmr10ms() - s_move_last_time) > 100)
    result = 0;

  s_move_last_time = get_tmr10ms();
  return result;
}

bool isSwitchWarningRequired(uint16_t &bad_pots)
{
  swarnstate_t states = g_model.switchWarning;

  if (!mixerTaskRunning()) getADC();
  getMovedSwitch();

  bool warn = false;
  for (int i = 0; i < switchGetMaxSwitches(); i++) {
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
    for (int  i = 0; i < adcGetMaxInputs(ADC_INPUT_FLEX); i++) {
      if (!IS_POT_SLIDER_AVAILABLE(i)) continue;
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
  swarnstate_t states = g_model.switchWarning;
  uint16_t bad_pots = 0, last_bad_pots = 0xff;

#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  while (true) {

    if (!isSwitchWarningRequired(bad_pots))
      break;

    cancelSplash();
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
      for (int i = 0; i < switchGetMaxSwitches(); ++i) {
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
        for (int i = 0; i < MAX_POTS; i++) {
          if (!IS_POT_SLIDER_AVAILABLE(i)) continue;
          if (g_model.potsWarnEnabled & (1 << i)) {
            if (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1) {
              if (++numWarnings < 6) {
                drawSource(x, y, MIXSRC_FIRST_POT + i, INVERS);
                const char* symbol;
                auto warn_pos = g_model.potsWarnPosition[i];
                if (IS_SLIDER(i)) {
                  symbol =  warn_pos > GET_LOWRES_POT_POSITION(i)
                    ? STR_CHAR_UP
                    : STR_CHAR_DOWN;
                } else {
                  symbol =  warn_pos > GET_LOWRES_POT_POSITION(i)
                    ? STR_CHAR_RIGHT
                    : STR_CHAR_LEFT;
                }
                lcdDrawText(lcdNextPos, y, symbol, INVERS);
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
        int16_t *lastValue = &LS_LAST_VALUE(fm, i);
        if (*lastValue == 0 || *lastValue == CS_LAST_VALUE_INIT) {
          *lastValue = -lswTimerValue(ls->v1);
        } else if (*lastValue < 0) {
          if (++(*lastValue) == 0) *lastValue = lswTimerValue(ls->v2);
        } else {  // if (*lastValue > 0)
          if (--(*lastValue) == 0) *lastValue = -lswTimerValue(ls->v1);
        }
      } else if (ls->func == LS_FUNC_STICKY) {
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
      } else if (ls->func == LS_FUNC_EDGE) {
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
