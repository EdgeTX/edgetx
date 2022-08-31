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

CustomFunctionsContext modelFunctionsContext = { 0 };

CustomFunctionsContext globalFunctionsContext = { 0 };

#if defined(DEBUG)
/*
 * This is a test function for debugging purpose, you may insert there your code and compile with the option DEBUG=YES
 */
void testFunc()
{
#ifdef SIMU
  printf("testFunc\n"); fflush(stdout);
#endif

  // for testing the WD reset uncomment the following line
  // while (1);
}
#endif

PLAY_FUNCTION(playValue, source_t idx)
{
  if (IS_FAI_FORBIDDEN(idx))
    return;

  if (idx == MIXSRC_NONE)
    return;

  getvalue_t val = getValue(idx);

  if (idx >= MIXSRC_FIRST_TELEM) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[(idx-MIXSRC_FIRST_TELEM) / 3];
    uint8_t attr = 0;
    if (telemetrySensor.prec > 0) {
      if (telemetrySensor.prec == 2) {
        if (val >= 5000) {
          val = divRoundClosest(val, 100);
        }
        else {
          val = divRoundClosest(val, 10);
          attr = PREC1;
        }
      }
      else {
        if (val >= 500) {
          val = divRoundClosest(val, 10);
        }
        else {
          attr = PREC1;
        }
      }
    }
    PLAY_NUMBER(val, telemetrySensor.unit == UNIT_CELLS ? UNIT_VOLTS : telemetrySensor.unit, attr);
  }
  else if (idx >= MIXSRC_FIRST_TIMER && idx <= MIXSRC_LAST_TIMER) {
    int flag = 0;
    if (val > LONG_TIMER_DURATION || -val > LONG_TIMER_DURATION) {
      flag = PLAY_LONG_TIMER;
    }
    PLAY_DURATION(val, flag);
  } else if (idx == MIXSRC_TX_TIME) {
    PLAY_DURATION(val * 60, PLAY_TIME);
  } else if (idx == MIXSRC_TX_VOLTAGE) {
    PLAY_NUMBER(val, UNIT_VOLTS, PREC1);
  } else {
    if (idx <= MIXSRC_LAST_CH) {
      val = calcRESXto100(val);
    }
    PLAY_NUMBER(val, 0, 0);
  }
}

void playCustomFunctionFile(const CustomFunctionData * sd, uint8_t id)
{
  if (sd->play.name[0] != '\0') {
    char filename[sizeof(SOUNDS_PATH) + LEN_FUNCTION_NAME + sizeof(SOUNDS_EXT)] = SOUNDS_PATH "/";
    strncpy(filename + SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
    strncpy(filename + sizeof(SOUNDS_PATH), sd->play.name, LEN_FUNCTION_NAME);
    filename[sizeof(SOUNDS_PATH) + LEN_FUNCTION_NAME] = '\0';
    strcat(filename + sizeof(SOUNDS_PATH), SOUNDS_EXT);
    PLAY_FILE(filename, sd->func == FUNC_BACKGND_MUSIC ? PLAY_BACKGROUND : 0, id);
  }
}

bool isRepeatDelayElapsed(const CustomFunctionData * functions, CustomFunctionsContext & functionsContext, uint8_t index)
{
  const CustomFunctionData * cfn = &functions[index];
  tmr10ms_t tmr10ms = get_tmr10ms();
  uint8_t repeatParam = CFN_PLAY_REPEAT(cfn);
  if (!IS_SILENCE_PERIOD_ELAPSED() && repeatParam == CFN_PLAY_REPEAT_NOSTART) {
    functionsContext.lastFunctionTime[index] = tmr10ms;
  }
  if (!functionsContext.lastFunctionTime[index] || (repeatParam && repeatParam!=CFN_PLAY_REPEAT_NOSTART && (signed)(tmr10ms-functionsContext.lastFunctionTime[index])>=100*repeatParam)) {
    functionsContext.lastFunctionTime[index] = tmr10ms;
    return true;
  }
  else {
    return false;
  }
}

#define VOLUME_HYSTERESIS 10            // how much must a input value change to actually be considered for new volume setting
getvalue_t requiredSpeakerVolumeRawLast = 1024 + 1; //initial value must be outside normal range

void evalFunctions(const CustomFunctionData * functions, CustomFunctionsContext & functionsContext)
{
  MASK_FUNC_TYPE newActiveFunctions  = 0;
  MASK_CFN_TYPE  newActiveSwitches = 0;

  uint8_t playFirstIndex = (functions == g_model.customFn ? 1 : 1+MAX_SPECIAL_FUNCTIONS);
  #define PLAY_INDEX   (i+playFirstIndex)

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS; i++) {
    safetyCh[i] = OVERRIDE_CHANNEL_UNDEFINED;
  }
#endif

#if defined(GVARS)
  for (uint8_t i=0; i<NUM_TRIMS; i++) {
    trimGvar[i] = -1;
  }
#endif

  for (uint8_t i=0; i<MAX_SPECIAL_FUNCTIONS; i++) {
    const CustomFunctionData * cfn = &functions[i];
    swsrc_t swtch = CFN_SWITCH(cfn);
    if (swtch) {
      MASK_CFN_TYPE switch_mask = ((MASK_CFN_TYPE)1 << i);

      bool active = getSwitch(swtch, IS_PLAY_FUNC(CFN_FUNC(cfn)) ? GETSWITCH_MIDPOS_DELAY : 0);

      if (HAS_ENABLE_PARAM(CFN_FUNC(cfn))) {
        active &= (bool)CFN_ACTIVE(cfn);
      }

      if (active) {
        switch (CFN_FUNC(cfn)) {

#if defined(OVERRIDE_CHANNEL_FUNCTION)
          case FUNC_OVERRIDE_CHANNEL:
            safetyCh[CFN_CH_INDEX(cfn)] = CFN_PARAM(cfn);
            break;
#endif

          case FUNC_TRAINER:
          {
            uint8_t param = CFN_CH_INDEX(cfn);
            if (param == 0)
              newActiveFunctions |= 0x0F;
            else if (param <= NUM_STICKS)
              newActiveFunctions |= (1 << (param - 1));
            else if (param == NUM_STICKS + 1)
              newActiveFunctions |= (1u << FUNCTION_TRAINER_CHANNELS);
            break;
          }

          case FUNC_INSTANT_TRIM:
            newActiveFunctions |= (1u << FUNCTION_INSTANT_TRIM);
            if (!isFunctionActive(FUNCTION_INSTANT_TRIM)) {
              if (IS_INSTANT_TRIM_ALLOWED()) {
                instantTrim();
              }
            }
            break;

          case FUNC_RESET:
            switch (CFN_PARAM(cfn)) {
              case FUNC_RESET_TIMER1:
              case FUNC_RESET_TIMER2:
              case FUNC_RESET_TIMER3:
                timerReset(CFN_PARAM(cfn));
                break;
              case FUNC_RESET_FLIGHT:
              	if (!(functionsContext.activeSwitches & switch_mask)) {
                  mainRequestFlags |= (1 << REQUEST_FLIGHT_RESET);     // on systems with threads flightReset() must not be called from the mixers thread!
                }
                break;
              case FUNC_RESET_TELEMETRY:
                telemetryReset();
                break;
            }
            if (CFN_PARAM(cfn)>=FUNC_RESET_PARAM_FIRST_TELEM) {
              uint8_t item = CFN_PARAM(cfn)-FUNC_RESET_PARAM_FIRST_TELEM;
              if (item < MAX_TELEMETRY_SENSORS) {
                telemetryItems[item].clear();
              }
            }
            break;

          case FUNC_SET_TIMER:
            timerSet(CFN_TIMER_INDEX(cfn), CFN_PARAM(cfn));
            break;

          case FUNC_SET_FAILSAFE:
            setCustomFailsafe(CFN_PARAM(cfn));
            break;

#if defined(DANGEROUS_MODULE_FUNCTIONS)
          case FUNC_RANGECHECK:
          case FUNC_BIND:
          {
            unsigned int moduleIndex = CFN_PARAM(cfn);
            if (moduleIndex < NUM_MODULES) {
              moduleState[moduleIndex].mode = 1 + CFN_FUNC(cfn) - FUNC_RANGECHECK;
            }
            break;
          }
#endif  

#if defined(GVARS)
          case FUNC_ADJUST_GVAR:
            if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_CONSTANT) {
              SET_GVAR(CFN_GVAR_INDEX(cfn), CFN_PARAM(cfn), mixerCurrentFlightMode);
            }
            else if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_GVAR) {
              SET_GVAR(CFN_GVAR_INDEX(cfn), GVAR_VALUE(CFN_PARAM(cfn), getGVarFlightMode(mixerCurrentFlightMode, CFN_PARAM(cfn))), mixerCurrentFlightMode);
            }
            else if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_INCDEC) {
              if (!(functionsContext.activeSwitches & switch_mask)) {
                SET_GVAR(CFN_GVAR_INDEX(cfn), limit<int16_t>(MODEL_GVAR_MIN(CFN_GVAR_INDEX(cfn)), GVAR_VALUE(CFN_GVAR_INDEX(cfn), getGVarFlightMode(mixerCurrentFlightMode, CFN_GVAR_INDEX(cfn))) + CFN_PARAM(cfn), MODEL_GVAR_MAX(CFN_GVAR_INDEX(cfn))), mixerCurrentFlightMode);
              }
            }
            else if (CFN_PARAM(cfn) >= MIXSRC_FIRST_TRIM && CFN_PARAM(cfn) <= MIXSRC_LAST_TRIM) {
              trimGvar[CFN_PARAM(cfn)-MIXSRC_FIRST_TRIM] = CFN_GVAR_INDEX(cfn);
            }
            else {
              SET_GVAR(CFN_GVAR_INDEX(cfn), limit<int16_t>(MODEL_GVAR_MIN(CFN_GVAR_INDEX(cfn)), calcRESXto100(getValue(CFN_PARAM(cfn))), MODEL_GVAR_MAX(CFN_GVAR_INDEX(cfn))), mixerCurrentFlightMode);
            }
            break;
#endif

          case FUNC_VOLUME:
          {
            getvalue_t raw = getValue(CFN_PARAM(cfn));
            // only set volume if input changed more than hysteresis
            if (abs(requiredSpeakerVolumeRawLast - raw) > VOLUME_HYSTERESIS) {
              requiredSpeakerVolumeRawLast = raw;
            }
            requiredSpeakerVolume = ((1024 + requiredSpeakerVolumeRawLast) * VOLUME_LEVEL_MAX) / 2048;
            break;
          }

#if defined(SDCARD)
          case FUNC_PLAY_SOUND:
          case FUNC_PLAY_TRACK:
          case FUNC_PLAY_VALUE:
#if defined(HAPTIC)
          case FUNC_HAPTIC:
#endif
          {
            if (isRepeatDelayElapsed(functions, functionsContext, i)) {
              if (!IS_PLAYING(PLAY_INDEX)) {
                if (CFN_FUNC(cfn) == FUNC_PLAY_SOUND) {
                  if (audioQueue.isEmpty()) {
                    AUDIO_PLAY(AU_SPECIAL_SOUND_FIRST + CFN_PARAM(cfn));
                  }
                }
                else if (CFN_FUNC(cfn) == FUNC_PLAY_VALUE) {
                  PLAY_VALUE(CFN_PARAM(cfn), PLAY_INDEX);
                }
#if defined(HAPTIC)
                else if (CFN_FUNC(cfn) == FUNC_HAPTIC) {
                  haptic.event(AU_SPECIAL_SOUND_LAST+CFN_PARAM(cfn));
                }
#endif
                else {
                  playCustomFunctionFile(cfn, PLAY_INDEX);
                }
              }
            }
            break;
          }

          case FUNC_BACKGND_MUSIC:
            if (!(newActiveFunctions & (1 << FUNCTION_BACKGND_MUSIC))) {
              newActiveFunctions |= (1 << FUNCTION_BACKGND_MUSIC);
              if (!IS_PLAYING(PLAY_INDEX)) {
                playCustomFunctionFile(cfn, PLAY_INDEX);
              }
            }
            break;

          case FUNC_BACKGND_MUSIC_PAUSE:
            newActiveFunctions |= (1 << FUNCTION_BACKGND_MUSIC_PAUSE);
            break;

#else
          case FUNC_PLAY_SOUND:
          case FUNC_PLAY_TRACK:
          case FUNC_PLAY_BOTH:
          case FUNC_PLAY_VALUE:
          {
            tmr10ms_t tmr10ms = get_tmr10ms();
            uint8_t repeatParam = CFN_PLAY_REPEAT(cfn);
            if (!functionsContext.lastFunctionTime[i] || (CFN_FUNC(cfn)==FUNC_PLAY_BOTH && active!=(bool)(functionsContext.activeSwitches&switch_mask)) || (repeatParam && (signed)(tmr10ms-functionsContext.lastFunctionTime[i])>=1000*repeatParam)) {
              functionsContext.lastFunctionTime[i] = tmr10ms;
              uint8_t param = CFN_PARAM(cfn);
              if (CFN_FUNC(cfn) == FUNC_PLAY_SOUND) {
                AUDIO_PLAY(AU_SPECIAL_SOUND_FIRST+param);
              }
              else if (CFN_FUNC(cfn) == FUNC_PLAY_VALUE) {
                PLAY_VALUE(param, PLAY_INDEX);
              }
              else {
#if defined(GVARS)
                if (CFN_FUNC(cfn) == FUNC_PLAY_TRACK && param > 250)
                  param = GVAR_VALUE(param-251, getGVarFlightMode(mixerCurrentFlightMode, param-251));
#endif
                PUSH_CUSTOM_PROMPT(active ? param : param+1, PLAY_INDEX);
              }
            }
            if (!active) {
              // PLAY_BOTH would change activeFnSwitches otherwise
              switch_mask = 0;
            }
            break;
          }
#endif

#if defined(VARIO)
          case FUNC_VARIO:
            newActiveFunctions |= (1u << FUNCTION_VARIO);
            break;
#endif


#if defined(SDCARD)
          case FUNC_LOGS:
            if (CFN_PARAM(cfn)) {
              newActiveFunctions |= (1u << FUNCTION_LOGS);
              logDelay100ms = CFN_PARAM(cfn);                   // logging period is 0..25.5s in 100ms increments
            }
            break;
#endif

          case FUNC_SCREENSHOT:
            if (!(functionsContext.activeSwitches & switch_mask)) {
              mainRequestFlags |= (1u << REQUEST_SCREENSHOT);
            }
            break;

#if defined(PXX2)
          case FUNC_RACING_MODE:
            if (isRacingModeEnabled()) {
              newActiveFunctions |= (1u << FUNCTION_RACING_MODE);
            }
            break;
#endif
#if defined(HARDWARE_TOUCH)
          case FUNC_DISABLE_TOUCH:
            newActiveFunctions |= (1u << FUNCTION_DISABLE_TOUCH);
        	break;
#endif
#if defined(DEBUG)
          case FUNC_TEST:
            testFunc();
            break;
#endif
        }

        newActiveSwitches |= switch_mask;
      }
      else {
        functionsContext.lastFunctionTime[i] = 0;
#if defined(DANGEROUS_MODULE_FUNCTIONS)
        if (functionsContext.activeSwitches & switch_mask) {
          switch (CFN_FUNC(cfn)) {
            case FUNC_RANGECHECK:
            case FUNC_BIND:
            {
              unsigned int moduleIndex = CFN_PARAM(cfn);
              if (moduleIndex < NUM_MODULES) {
                moduleState[moduleIndex].mode = 0;
              }
              break;
            }
          }
        }
#endif
      }
    }
  }

  functionsContext.activeSwitches   = newActiveSwitches;
  functionsContext.activeFunctions  = newActiveFunctions;
}

