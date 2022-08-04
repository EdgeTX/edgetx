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

#pragma once

#include <stdlib.h>
#include "definitions.h"
#include "opentx_types.h"
#include "debounce.h"
#include "opentx_helpers.h"
#include "touch.h"

#if defined(SIMU)
#include "targets/simu/simpgmspace.h"
#endif

#include "board.h"

#include "usbd_conf.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "libopenui/src/libopenui_file.h"
#endif

#if defined(SIMU)
  #define SWITCH_SIMU(a, b)  (a)
#else
  #define SWITCH_SIMU(a, b)  (b)
#endif

#if defined(VARIO)
  #define CASE_VARIO(x) x,
#else
  #define CASE_VARIO(x)
#endif

#if defined(IMU)
#define CASE_IMU(x) x,
#else
#define CASE_IMU(x)
#endif

#if defined(BACKLIGHT_GPIO)
#define CASE_BACKLIGHT(x) x,
#else
#define CASE_BACKLIGHT(x)
#endif

#if defined(LUA)
  #define CASE_LUA(x) x,
#else
  #define CASE_LUA(x)
#endif

#if defined(RTCLOCK)
  #define CASE_RTCLOCK(x) x,
#else
  #define CASE_RTCLOCK(x)
#endif

#if defined(BUZZER)
  #define CASE_BUZZER(x) x,
#else
  #define CASE_BUZZER(x)
#endif

#if defined(AUDIO)
  #define CASE_AUDIO(x) x,
#else
  #define CASE_AUDIO(x)
#endif

#if defined(PWM_BACKLIGHT)
  #define CASE_PWM_BACKLIGHT(x) x,
#else
  #define CASE_PWM_BACKLIGHT(x)
#endif

#if defined(GPS)
  #define CASE_GPS(x) x,
#else
  #define CASE_GPS(x)
#endif

#if defined(VARIO)
  #define CASE_VARIO(x) x,
#else
  #define CASE_VARIO(x)
#endif

#if defined(HAPTIC)
  #define CASE_HAPTIC(x) x,
#else
  #define CASE_HAPTIC(x)
#endif

#if defined(SPLASH)
  #define CASE_SPLASH(x) x,
#else
  #define CASE_SPLASH(x)
#endif

#if defined(PWR_BUTTON_PRESS)
  #define CASE_PWR_BUTTON_PRESS(x) x,
#else
  #define CASE_PWR_BUTTON_PRESS(x)
#endif

#if defined(PXX1)
  #define CASE_PXX1(x) x,
#else
  #define CASE_PXX1(x)
#endif

#if defined(PXX2)
  #define CASE_PXX2(x) x,
#else
  #define CASE_PXX2(x)
#endif

#if defined(SDCARD)
  #define CASE_SDCARD(x) x,
#else
  #define CASE_SDCARD(x)
#endif

#if defined(BLUETOOTH)
  #define CASE_BLUETOOTH(x) x,
#else
  #define CASE_BLUETOOTH(x)
#endif

#if defined(HELI)
  #define CASE_HELI(x) x,
#else
  #define CASE_HELI(x)
#endif

#if defined(FLIGHT_MODES)
  #define CASE_FLIGHT_MODES(x) x,
#else
  #define CASE_FLIGHT_MODES(x)
#endif

#if defined(GVARS)
  #define CASE_GVARS(x) x,
#else
  #define CASE_GVARS(x)
#endif

#if defined(PCBX9DP) || defined(PCBX9E)
  #define CASE_PCBX9E_PCBX9DP(x) x,
#else
  #define CASE_PCBX9E_PCBX9DP(x)
#endif

#if defined(PCBX9E)
  #define CASE_PCBX9E(x) x,
#else
  #define CASE_PCBX9E(x)
#endif

  #define CASE_CAPACITY(x)

#if defined(FAI)
  #define IS_FAI_ENABLED() true
  #define IF_FAI_CHOICE(x)
#elif defined(FAI_CHOICE)
  #define IS_FAI_ENABLED() g_eeGeneral.fai
  #define IF_FAI_CHOICE(x) x,
#else
  #define IS_FAI_ENABLED() false
  #define IF_FAI_CHOICE(x)
#endif

#define IS_FAI_FORBIDDEN(idx) (IS_FAI_ENABLED() && isFaiForbidden(idx))

#if defined(BLUETOOTH)
  #if defined(X9E)
    #define IS_BLUETOOTH_TRAINER()       (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
    #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE)
  #else
    #define IS_BLUETOOTH_TRAINER()       (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH || g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
    #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE || g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
  #endif
#else
  #define IS_BLUETOOTH_TRAINER()       false
  #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE)
#endif

#if defined(LUA) || defined(PXX2) || defined(MULTIMODULE)
  #define RADIO_TOOLS
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
enum RotaryEncoderMode {
  ROTARY_ENCODER_MODE_NORMAL,
  ROTARY_ENCODER_MODE_INVERT_BOTH,
  ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM,
  ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT
};
#endif

// RESX range is used for internal calculation; The menu says -100.0 to 100.0; internally it is -1024 to 1024 to allow some optimizations
#define RESX_SHIFT 10
#define RESX       1024
#define RESXu      1024u
#define RESXul     1024ul
#define RESXl      1024l

#if defined(JACK_DETECT_GPIO)
#define CASE_JACK_DETECT(x) x,
#else
#define CASE_JACK_DETECT(x)
#endif

#if defined(DISK_CACHE)
  #include "disk_cache.h"
#endif

#include "debug.h"

#if defined(PCBFRSKY) || defined(PCBFLYSKY)
  #define SWSRC_THR                    SWSRC_SB2
  #define SWSRC_GEA                    SWSRC_SG2
  #define SWSRC_ID0                    SWSRC_SA0
  #define SWSRC_ID1                    SWSRC_SA1
  #define SWSRC_ID2                    SWSRC_SA2
  #define IS_MOMENTARY(sw)             false // TODO
#else
  #define SW_DSM2_BIND     SW_TRN
#endif

#include "myeeprom.h"
#include "curves.h"

void memswap(void * a, void * b, uint8_t size);

#if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E) || defined(PCBHORUS)
  #define POT_CONFIG(x)                ((g_eeGeneral.potsConfig >> (2*((x)-POT1)))&0x03)
  #define IS_POT_MULTIPOS(x)           (IS_POT(x) && POT_CONFIG(x)==POT_MULTIPOS_SWITCH)
  #define IS_POT_WITHOUT_DETENT(x)     (IS_POT(x) && POT_CONFIG(x)==POT_WITHOUT_DETENT)
  #define IS_SLIDER_AVAILABLE(x)       ((x) == SLIDER1 || (x) == SLIDER2 || (IS_SLIDER(x) && (g_eeGeneral.slidersConfig & (0x01 << ((x)-SLIDER1)))))
  #define IS_POT_AVAILABLE(x)          (IS_POT(x) && POT_CONFIG(x)!=POT_NONE)
  #define IS_POT_SLIDER_AVAILABLE(x)   (IS_POT_AVAILABLE(x) || IS_SLIDER_AVAILABLE(x))
  #define IS_MULTIPOS_CALIBRATED(cal)  (cal->count>0 && cal->count<XPOTS_MULTIPOS_COUNT)
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBNV14)
  #define POT_CONFIG(x)                ((g_eeGeneral.potsConfig >> (2*((x)-POT1)))&0x03)
  #define IS_POT_MULTIPOS(x)           (IS_POT(x) && POT_CONFIG(x)==POT_MULTIPOS_SWITCH)
  #define IS_POT_WITHOUT_DETENT(x)     (IS_POT(x) && POT_CONFIG(x)==POT_WITHOUT_DETENT)
  #define IS_POT_AVAILABLE(x)          (IS_POT(x) && POT_CONFIG(x)!=POT_NONE)
  #define IS_POT_SLIDER_AVAILABLE(x)   (IS_POT_AVAILABLE(x))
  #define IS_MULTIPOS_CALIBRATED(cal)  (cal->count>0 && cal->count<XPOTS_MULTIPOS_COUNT)
#else
  #define IS_POT_MULTIPOS(x)           (false)
  #define IS_POT_WITHOUT_DETENT(x)     (true)
  #define IS_POT_SLIDER_AVAILABLE(x)   (true)
  #define IS_MULTIPOS_CALIBRATED(cal)  (false)
#endif

#if NUM_XPOTS > 0
  #define IS_SWITCH_MULTIPOS(x)         (SWSRC_FIRST_MULTIPOS_SWITCH <= (x) && (x) <= SWSRC_LAST_MULTIPOS_SWITCH)
#else
  #define IS_SWITCH_MULTIPOS(x)         (false)
#endif

#define GET_LOWRES_POT_POSITION(i)     (getValue(MIXSRC_FIRST_POT+(i)) >> 4)
#define SAVE_POT_POSITION(i)           g_model.potsWarnPosition[i] = GET_LOWRES_POT_POSITION(i)

#define PPM_CENTER                     1500

#if defined(PPM_CENTER_ADJUSTABLE)
  #define PPM_CH_CENTER(ch)            (PPM_CENTER + limitAddress(ch)->ppmCenter)
#else
  #define PPM_CH_CENTER(ch)            (PPM_CENTER)
#endif

#include "fifo.h"
#include "io/frsky_sport.h"

#if defined(CLI)
#include "cli.h"
#endif

#include "timers.h"
#include "storage/storage.h"
#include "pulses/pulses.h"
#include "pulses/modules_helpers.h"

#define MASK_CFN_TYPE  uint64_t  // current max = 64 function switches
#define MASK_FUNC_TYPE uint32_t  // current max = 32 functions

struct CustomFunctionsContext {
  MASK_FUNC_TYPE activeFunctions;
  MASK_CFN_TYPE  activeSwitches;
  tmr10ms_t lastFunctionTime[MAX_SPECIAL_FUNCTIONS];

  inline bool isFunctionActive(uint8_t func)
  {
    return activeFunctions & ((MASK_FUNC_TYPE)1 << func);
  }

  void reset()
  {
    memclear(this, sizeof(*this));
  }
};

#include "strhelpers.h"
#include "gui.h"

#if !defined(SIMU)
  #define assert(x)
  #if !defined(DEBUG)
    #define printf printf_not_allowed
  #endif
#endif

extern const uint8_t bchout_ar[];
extern const uint8_t modn12x3[];

//convert from mode 1 to mode stickMode
//NOTICE!  =>  0..3 -> 0..3
#define RUD_STICK 0
#define ELE_STICK 1
#define THR_STICK 2
#define AIL_STICK 3
#define CONVERT_MODE(x)          (((x)<=AIL_STICK) ? *(modn12x3 + 4*g_eeGeneral.stickMode + (x)) : (x) )

#if defined(PCBXLITE)
  #define CONVERT_MODE_TRIMS(x)  (((x) == RUD_STICK) ? AIL_STICK : ((x) == AIL_STICK) ? RUD_STICK : (x))
#else
  #define CONVERT_MODE_TRIMS(x)  CONVERT_MODE(x)
#endif

extern uint8_t channelOrder(uint8_t x);
extern uint8_t channelOrder(uint8_t setup, uint8_t x);

#define THRCHK_DEADBAND                16

inline bool SPLASH_NEEDED()
{
#if defined(COLORLCD)
  return false;
#else
  return g_eeGeneral.splashMode != 3;
#endif
}

#if defined(PCBHORUS)
  #define SPLASH_TIMEOUT               0 /* we use the splash duration to load stuff from the SD */
#elif defined(PCBTARANIS)
  #define SPLASH_TIMEOUT               (g_eeGeneral.splashMode == -4 ? 1500 : (g_eeGeneral.splashMode <= 0 ? (400-g_eeGeneral.splashMode * 200) : (400 - g_eeGeneral.splashMode * 100)))
#else
  #define SPLASH_TIMEOUT               (4 * 100)  // 4 seconds
#endif

constexpr uint8_t HEART_TIMER_10MS = 0x01;
constexpr uint8_t HEART_TIMER_PULSES = 0x02; // when multiple modules this is the first one

constexpr uint8_t HEART_WDT_CHECK = HEART_TIMER_10MS
#if defined(HARDWARE_INTERNAL_MODULE)
                                    + (HEART_TIMER_PULSES << INTERNAL_MODULE)
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
                                    + (HEART_TIMER_PULSES << EXTERNAL_MODULE)
#endif
    ;  // end of HEART_WDT_CHECK

extern uint8_t heartbeat;

#define MAX_ALERT_TIME   60

#define LEN_STD_CHARS 40

#if defined(TRANSLATIONS_CZ)
#define ZCHAR_MAX (LEN_STD_CHARS)
#else
#define ZCHAR_MAX (LEN_STD_CHARS + LEN_SPECIAL_CHARS)
#endif

#include "keys.h"
#include "pwr.h"

#if defined(PCBFRSKY) || defined(PCBNV14)
div_t switchInfo(int switchPosition);
extern uint8_t potsPos[NUM_XPOTS];
#endif

bool trimDown(uint8_t idx);

#if defined(KEYS_GPIO_REG_BIND)
void bindButtonHandler(event_t event);
#endif

uint16_t evalChkSum();

void alert(const char * title, const char * msg, uint8_t sound);

#if !defined(GUI)

  #define RAISE_ALERT(...)
  #define ALERT(...)

#elif defined(COLORLCD)

#define TELEMETRY_CHECK_DELAY10ms 150

bool confirmationDialog(const char *title, const char *msg, bool checkPwr = true, const std::function<bool(void)>& closeCondition = nullptr);

void raiseAlert(const char *title, const char *msg, const char *info,
                uint8_t sound);

inline void RAISE_ALERT(const char *title, const char *msg, const char *info,
                        uint8_t sound)
{
  raiseAlert(title, msg, info, sound);
}
inline void ALERT(const char *title, const char *msg, uint8_t sound)
{
  raiseAlert(title, msg, "", sound);
}

#else // !COLORLCD && GUI

inline void RAISE_ALERT(const char *title, const char *msg, const char *info,
                        uint8_t sound)
{
  showAlertBox(title, msg, info, sound);
}

inline void ALERT(const char *title, const char *msg, uint8_t sound)
{
  alert(title, msg, sound);
}

#endif // !COLORLCD && GUI

enum PerOutMode {
  e_perout_mode_normal = 0,
  e_perout_mode_inactive_flight_mode = 1,
  e_perout_mode_notrainer = 2,
  e_perout_mode_notrims = 4,
  e_perout_mode_nosticks = 8,
  e_perout_mode_noinput = e_perout_mode_notrainer+e_perout_mode_notrims+e_perout_mode_nosticks
};

extern uint8_t mixerCurrentFlightMode;
extern uint8_t lastFlightMode;
extern uint8_t flightModeTransitionLast;

#if defined(SIMU)
  inline int availableMemory() { return 1000; }
#else
  extern unsigned char *heap;
  extern int _end;
  extern int _heap_end;
  #define availableMemory() ((unsigned int)((unsigned char *)&_heap_end - heap))
#endif


void evalFlightModeMixes(uint8_t mode, uint8_t tick10ms);
void evalMixes(uint8_t tick10ms);
void doMixerCalculations();
void doMixerPeriodicUpdates();

void checkTrims();
extern uint8_t currentBacklightBright;
void perMain();
void per10ms();

getvalue_t getValue(mixsrc_t i);

#define GETSWITCH_MIDPOS_DELAY   1
bool getSwitch(swsrc_t swtch, uint8_t flags=0);

void logicalSwitchesTimerTick();
void logicalSwitchesReset();

void evalLogicalSwitches(bool isCurrentFlightmode=true);
void logicalSwitchesCopyState(uint8_t src, uint8_t dst);

#if defined(PCBFRSKY) || defined(PCBFLYSKY)
  void getSwitchesPosition(bool startup);
#else
  #define getSwitchesPosition(...)
#endif

extern swarnstate_t switches_states;
swsrc_t getMovedSwitch();

int8_t getMovedSource(uint8_t min);
#define GET_MOVED_SOURCE(min, max) getMovedSource(min)

#if defined(FLIGHT_MODES)
  extern uint8_t getFlightMode();
#else
  #define getFlightMode() 0
#endif

#define getTrimFlightMode(phase, idx) (phase)

#if defined(GVARS)
  extern int8_t trimGvar[NUM_TRIMS];
  #define TRIM_REUSED(idx) trimGvar[idx] >= 0
#else
  #define TRIM_REUSED(idx) 0
#endif

trim_t getRawTrimValue(uint8_t phase, uint8_t idx);
int getTrimValue(uint8_t phase, uint8_t idx);

bool setTrimValue(uint8_t phase, uint8_t idx, int trim);

#include "gvars.h"

void flightReset(uint8_t check=true);

#define DURATION_MS_PREC2(x) ((x)/20)

#if defined(THRTRACE)
  #if defined(COLORLCD)
    #define MAXTRACE (LCD_W-2*10)
  #else
    #define MAXTRACE (LCD_W - 8)
  #endif
  extern uint8_t  s_traceBuf[MAXTRACE];
  extern uint16_t s_traceWr;
  extern uint8_t  s_cnt_10s;
  extern uint16_t s_cnt_samples_thr_10s;
  extern uint16_t s_sum_samples_thr_10s;
  #define RESET_THR_TRACE() s_traceWr = s_cnt_10s = s_cnt_samples_thr_10s = s_sum_samples_thr_10s = s_timeCum16ThrP = s_timeCumThr = 0
#else
  #define RESET_THR_TRACE() s_timeCum16ThrP = s_timeCumThr = 0
#endif

#if defined(SPLASH)
  void doSplash();
#endif

// disabled function (not used anywhere)
#define READ_ONLY() false
#define READ_ONLY_UNLOCKED() true

void checkLowEEPROM();
void checkThrottleStick();
void checkSwitches();
void checkAlarm();
void checkAll();

void getADC();
static inline void GET_ADC_IF_MIXER_NOT_RUNNING()
{
  if (s_pulses_paused) {
    getADC();
  }
}

#include "sbus.h"

void resetBacklightTimeout();
void checkBacklight();

uint16_t isqrt32(uint32_t n);

#if defined(BOOT)
#define pauseMixerCalculations()
#define resumeMixerCalculations()
#else
#include "tasks.h"
extern RTOS_MUTEX_HANDLE mixerMutex;
inline void pauseMixerCalculations()
{
  RTOS_LOCK_MUTEX(mixerMutex);
}

inline void resumeMixerCalculations()
{
  RTOS_UNLOCK_MUTEX(mixerMutex);
}
#endif

void setDefaultOwnerId();
void generalDefault();

uint32_t hash(const void * ptr, uint32_t size);

#define calc100to256_16Bits(x) calc100to256(x)
#define calc100toRESX_16Bits(x) calc100toRESX(x)

#define calc100to256_16Bits(x) calc100to256(x)
#define calc100toRESX_16Bits(x) calc100toRESX(x)

inline int calc100to256(int x)
{
  return divRoundClosest(x*256, 100);
}

inline int calc100toRESX(int x)
{
  return divRoundClosest(x*RESX, 100);
}

inline int calc1000toRESX(int x)
{
  return divRoundClosest(x*RESX, 1000);
}

inline int calcRESXto1000(int x)
{
  return divRoundClosest(x*1000, RESX);
}

inline int calcRESXto100(int x)
{
  return divRoundClosest(x*100, RESX);
}

#define g_blinkTmr10ms    (*(uint8_t*)&g_tmr10ms)

#include "trainer.h"

int expo(int x, int k);

inline void getMixSrcRange(const int source, int16_t & valMin, int16_t & valMax, LcdFlags * flags = 0)
{
  if (source >= MIXSRC_FIRST_TRIM && source <= MIXSRC_LAST_TRIM) {
    valMax = g_model.extendedTrims ? TRIM_EXTENDED_MAX : TRIM_MAX;
    valMin = -valMax;
  }
#if defined(LUA_INPUTS)
  else if (source >= MIXSRC_FIRST_LUA && source <= MIXSRC_LAST_LUA) {
    valMax = 30000;
    valMin = -valMax;
  }
#endif
  else if (source < MIXSRC_FIRST_CH) {
    valMax = 100;
    valMin = -valMax;
  }
  else if (source <= MIXSRC_LAST_CH) {
    valMax = g_model.extendedLimits ? LIMIT_EXT_PERCENT : 100;
    valMin = -valMax;
  }
#if defined(GVARS)
  else if (source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR) {
    valMax = min<int>(CFN_GVAR_CST_MAX, MODEL_GVAR_MAX(source-MIXSRC_FIRST_GVAR));
    valMin = max<int>(CFN_GVAR_CST_MIN, MODEL_GVAR_MIN(source-MIXSRC_FIRST_GVAR));
    if (flags && g_model.gvars[source-MIXSRC_FIRST_GVAR].prec)
      *flags |= PREC1;
  }
#endif
  else if (source == MIXSRC_TX_VOLTAGE) {
    valMax =  255;
    valMin = 0;
    if (flags)
      *flags |= PREC1;
  }
  else if (source == MIXSRC_TX_TIME) {
    valMax =  23 * 60 + 59;
    valMin = 0;
  }
  else if (source >= MIXSRC_FIRST_TIMER && source <= MIXSRC_LAST_TIMER) {
    valMax =  9 * 60 * 60 - 1;
    valMin = -valMax;
    if (flags)
      *flags |= TIMEHOUR;
  }
  else {
    valMax = 30000;
    valMin = -valMax;
  }
}
#if defined(GVAR_MAX)
inline void getGVarIncDecRange(int16_t & valMin, int16_t & valMax)
{
  int16_t rng = abs(valMax - valMin);
  valMin = -rng;
  valMax = rng;
}
#endif

void applyExpos(int16_t * anas, uint8_t mode, uint8_t ovwrIdx=0, int16_t ovwrValue=0);
int16_t applyLimits(uint8_t channel, int32_t value);

void evalInputs(uint8_t mode);
uint16_t anaIn(uint8_t chan);

#define FLASH_DURATION 20 /*200ms*/

FlightModeData * flightModeAddress(uint8_t idx);
ExpoData * expoAddress(uint8_t idx);
MixData * mixAddress(uint8_t idx);
LimitData * limitAddress(uint8_t idx);
LogicalSwitchData * lswAddress(uint8_t idx);

void applyDefaultTemplate();
void instantTrim();
void evalTrims();
void copyTrimsToOffset(uint8_t ch);
void copySticksToOffset(uint8_t ch);
void copyMinMaxToOutputs(uint8_t ch);
void moveTrimsToOffsets();

inline bool isExpoActive(uint8_t expo)
{
  return swOn[expo].activeExpo;
}

inline bool isMixActive(uint8_t mix)
{
  return swOn[mix].activeMix;
}

enum FunctionsActive {
  FUNCTION_TRAINER_STICK1,
  FUNCTION_TRAINER_CHANNELS = FUNCTION_TRAINER_STICK1 + NUM_STICKS,
  FUNCTION_INSTANT_TRIM,
  FUNCTION_VARIO,
#if defined(SDCARD)
  FUNCTION_LOGS,
#endif
  FUNCTION_BACKGND_MUSIC,
  FUNCTION_BACKGND_MUSIC_PAUSE,
  FUNCTION_BACKLIGHT,
  FUNCTION_RACING_MODE,
#if defined(HARDWARE_TOUCH)
  FUNCTION_DISABLE_TOUCH,
#endif
};

#define VARIO_FREQUENCY_ZERO   700/*Hz*/
#define VARIO_FREQUENCY_RANGE  1000/*Hz*/
#define VARIO_REPEAT_ZERO      500/*ms*/
#define VARIO_REPEAT_MAX       80/*ms*/

extern CustomFunctionsContext modelFunctionsContext;
extern CustomFunctionsContext globalFunctionsContext;
inline bool isFunctionActive(uint8_t func)
{
  return globalFunctionsContext.isFunctionActive(func) || modelFunctionsContext.isFunctionActive(func);
}
void evalFunctions(const CustomFunctionData * functions, CustomFunctionsContext & functionsContext);
inline void customFunctionsReset()
{
  globalFunctionsContext.reset();
  modelFunctionsContext.reset();
}

#include "telemetry/telemetry.h"
#include "crc.h"

#define PLAY_REPEAT(x)            (x)                 /* Range 0 to 15 */
#define PLAY_NOW                  0x10
#define PLAY_BACKGROUND           0x20

enum AUDIO_SOUNDS {
  AUDIO_HELLO,
  AU_BYE,
  AU_THROTTLE_ALERT,
  AU_SWITCH_ALERT,
  AU_BAD_RADIODATA,
  AU_TX_BATTERY_LOW,
  AU_INACTIVITY,
  AU_RSSI_ORANGE,
  AU_RSSI_RED,
  AU_RAS_RED,
  AU_TELEMETRY_LOST,
  AU_TELEMETRY_BACK,
  AU_TRAINER_LOST,
  AU_TRAINER_BACK,
  AU_SENSOR_LOST,
  AU_SERVO_KO,
  AU_RX_OVERLOAD,
  AU_MODEL_STILL_POWERED,
  AU_ERROR,
  AU_WARNING1,
  AU_WARNING2,
  AU_WARNING3,
  AU_TRIM_MIDDLE,
  AU_TRIM_MIN,
  AU_TRIM_MAX,
  AU_STICK1_MIDDLE,
  AU_STICK2_MIDDLE,
  AU_STICK3_MIDDLE,
  AU_STICK4_MIDDLE,
#if defined(PCBFRSKY)
  AU_POT1_MIDDLE,
  AU_POT2_MIDDLE,
#if defined(PCBX9E)
  AU_POT3_MIDDLE,
  AU_POT4_MIDDLE,
#endif
#if defined(PCBX10)
  AU_POT4_MIDDLE,
  AU_POT5_MIDDLE,
  AU_POT6_MIDDLE,
  AU_POT7_MIDDLE,
#endif
  AU_SLIDER1_MIDDLE,
  AU_SLIDER2_MIDDLE,
#if defined(PCBX9E)
  AU_SLIDER3_MIDDLE,
  AU_SLIDER4_MIDDLE,
#endif
#else
  AU_POT1_MIDDLE,
  AU_POT2_MIDDLE,
  AU_POT3_MIDDLE,
#endif
  AU_MIX_WARNING_1,
  AU_MIX_WARNING_2,
  AU_MIX_WARNING_3,
  AU_TIMER1_ELAPSED,
  AU_TIMER2_ELAPSED,
  AU_TIMER3_ELAPSED,

  AU_SPECIAL_SOUND_FIRST,
  AU_SPECIAL_SOUND_BEEP1 = AU_SPECIAL_SOUND_FIRST,
  AU_SPECIAL_SOUND_BEEP2,
  AU_SPECIAL_SOUND_BEEP3,
  AU_SPECIAL_SOUND_WARN1,
  AU_SPECIAL_SOUND_WARN2,
  AU_SPECIAL_SOUND_CHEEP,
  AU_SPECIAL_SOUND_RATATA,
  AU_SPECIAL_SOUND_TICK,
  AU_SPECIAL_SOUND_SIREN,
  AU_SPECIAL_SOUND_RING,
  AU_SPECIAL_SOUND_SCIFI,
  AU_SPECIAL_SOUND_ROBOT,
  AU_SPECIAL_SOUND_CHIRP,
  AU_SPECIAL_SOUND_TADA,
  AU_SPECIAL_SOUND_CRICKET,
  AU_SPECIAL_SOUND_ALARMC,
  AU_SPECIAL_SOUND_LAST,

  AU_NONE = 0xff
};

#if defined(AUDIO)
#include "audio.h"
#endif

#include "buzzer.h"
#include "translations.h"

#if defined(HAPTIC)
#include "haptic.h"
#endif

#if defined(SDCARD)
#include "sdcard.h"
#endif

#if defined(RTCLOCK)
#include "rtc.h"
#endif


void checkBattery();
void opentxClose(uint8_t shutdown=true);
void opentxInit();
void opentxResume();

constexpr uint8_t OPENTX_START_NO_SPLASH = 0x01;
constexpr uint8_t OPENTX_START_NO_CALIBRATION = 0x02;
constexpr uint8_t OPENTX_START_NO_CHECKS = 0x04;

#if defined(STATUS_LEDS)
  #define LED_ERROR_BEGIN()            ledRed()
#if defined(RADIO_T8)
  // Because of green backlit logo, green is preferred on this radio
  #define LED_ERROR_END()              ledGreen()
  #define LED_BIND()                   ledBlue()
#else
  #define LED_ERROR_END()              ledBlue()
#endif
#else
  #define LED_ERROR_BEGIN()
  #define LED_ERROR_END()
#endif

// Re-useable byte array to save having multiple buffers
#if LCD_W <= 212
constexpr uint8_t SD_SCREEN_FILE_LENGTH = 32;
#else
constexpr uint8_t SD_SCREEN_FILE_LENGTH = 64;
#endif

#if defined(BLUETOOTH)
#include "bluetooth.h"
#endif

constexpr uint8_t TEXT_FILENAME_MAXLEN = 40;

#if defined(GHOST)
  #include "telemetry/ghost_menu.h"
#endif

union ReusableBuffer
{
  struct {
#if defined(EEPROM_RLC) && LCD_W < 212
    uint16_t eepromfree;
#endif
#if defined(SDCARD) && !defined(COLORLCD)
    char menu_bss[POPUP_MENU_MAX_LINES][MENU_LINE_LENGTH];
    char mainname[45]; // because reused for SD backup / restore, max backup filename 44 chars: "/MODELS/MODEL0134353-2014-06-19-04-51-27.bin"
#elif !defined(COLORLCD)
    char mainname[LEN_MODEL_NAME];
#endif
  } modelsel;

  struct {
    char msg[64];
    uint8_t r9mPower;
    int8_t antennaMode;
    uint8_t previousType;
    uint8_t newType;
    BindInformation bindInformation;
    PXX2ModuleSetup pxx2;
#if defined(BLUETOOTH)
    struct {
      char devices[MAX_BLUETOOTH_DISTANT_ADDR][LEN_BLUETOOTH_ADDR+1];
      uint8_t devicesCount;
    } bt;
#endif
  } moduleSetup;

  struct {
    int16_t midVals[NUM_STICKS+NUM_POTS+NUM_SLIDERS+STORAGE_NUM_MOUSE_ANALOGS];
    int16_t loVals[NUM_STICKS+NUM_POTS+NUM_SLIDERS+STORAGE_NUM_MOUSE_ANALOGS];
    int16_t hiVals[NUM_STICKS+NUM_POTS+NUM_SLIDERS+STORAGE_NUM_MOUSE_ANALOGS];
    uint8_t state;
#if defined(PCBFRSKY)
    struct {
      uint8_t stepsCount;
      int16_t steps[XPOTS_MULTIPOS_COUNT];
      uint8_t lastCount;
      int16_t lastPosition;
    } xpotsCalib[NUM_XPOTS];
#endif
  } calib;

#if defined(SDCARD)
  struct {
    char lines[NUM_BODY_LINES][SD_SCREEN_FILE_LENGTH+1+1]; // the last char is used to store the flags (directory) of the line
    uint32_t available;
    uint16_t offset;
    uint16_t count;
    char originalName[SD_SCREEN_FILE_LENGTH+1];
    OtaUpdateInformation otaUpdateInformation;
    char otaReceiverVersion[sizeof(TR_CURRENT_VERSION) + 12];
  } sdManager;
#endif

  struct
  {
    char id[27];
  } version;

  PXX2HardwareAndSettings hardwareAndSettings; // radio_version

  struct {
    ModuleInformation modules[NUM_MODULES];
    char msg[64];
#if !defined(COLORLCD)
    uint8_t linesCount;
#endif
  } radioTools;

  struct {
    int8_t antennaMode;
  } radioHardware;

  struct {
    uint8_t stickMode;
#if defined(ROTARY_ENCODER_NAVIGATION)
    uint8_t rotaryEncoderMode;
#endif
  } generalSettings;

  struct {
    uint8_t bars[LCD_W];
    uint8_t max[LCD_W];
    uint32_t freq;
    uint32_t span;
    uint32_t step;
    uint32_t track;
    uint8_t spanDefault;
    uint8_t spanMax;
    uint16_t freqDefault;
    uint16_t freqMax;
    uint16_t freqMin;
    uint8_t dirty;
    uint8_t moduleOFF;
  } spectrumAnalyser;

#if defined(GHOST)
  struct {
    GhostMenuData line[GHST_MENU_LINES + 1];
    uint8_t menuStatus;
    uint8_t menuAction;
    uint8_t buttonAction;
  } ghostMenu;
#endif

  struct {
    uint32_t freq;
    int16_t power;
    int16_t peak;
    uint8_t attn;
    uint8_t dirty;
  } powerMeter;

  struct {
    int8_t preset;
  } curveEdit;

#if !defined(COLORLCD)
  struct {
    char filename[TEXT_FILENAME_MAXLEN];
    char lines[NUM_BODY_LINES][LCD_COLS + 1];
    int linesCount;
  } viewText;
#endif

  struct {
    bool longNames;
    bool secondPage;
    bool mixersView;
  } viewChannels;

  struct {
    uint8_t maxNameLen;
  } modelFailsafe;

  struct {
    ModuleInformation internalModule;
  } viewMain;

  // Data for the USB mass storage driver. If USB mass storage runs no menu is not allowed to be displayed
  uint8_t MSC_BOT_Data[MSC_MEDIA_PACKET];
};

extern ReusableBuffer reusableBuffer;

uint8_t zlen(const char *str, uint8_t size);
bool zexist(const char *str, uint8_t size);
char * strcat_zchar(char *dest, const char *name, uint8_t size, const char spaceSym = 0, const char *defaultName=nullptr, uint8_t defaultNameSize=0, uint8_t defaultIdx=0);
#define strcatFlightmodeName(dest, idx) strcat_zchar(dest, g_model.flightModeData[idx].name, LEN_FLIGHT_MODE_NAME, 0, STR_FM, PSIZE(TR_FM), idx+1)

#if !defined(STORAGE_MODELSLIST)

#define strcat_modelname(dest, idx)                                     \
  strcat_zchar(dest, modelHeaders[idx].name, LEN_MODEL_NAME, 0, STR_MODEL, \
               PSIZE(TR_MODEL), idx + 1)

#define strcat_currentmodelname(dest, foo)      \
  strcat_modelname(dest, g_eeGeneral.currModel)

#else

#define strcat_currentmodelname(dest, spaceSym)                         \
  strcat_zchar(dest, g_model.header.name, LEN_MODEL_NAME, spaceSym)

#endif

#define ZLEN(s) zlen(s, sizeof(s))
#define ZEXIST(s) zexist(s, sizeof(s))

// Stick tolerance varies between transmitters, Higher is better
#define STICK_TOLERANCE 64

ls_telemetry_value_t maxTelemValue(source_t channel);

getvalue_t convert16bitsTelemValue(source_t channel, ls_telemetry_value_t value);
getvalue_t convertLswTelemValue(LogicalSwitchData * cs);

inline getvalue_t convertTelemValue(source_t channel, ls_telemetry_value_t value)
{
  return convert16bitsTelemValue(channel, value);
}

extern uint8_t g_vbat100mV;

inline uint8_t GET_TXBATT_BARS(uint8_t barsMax)
{
  return limit<int8_t>(0, divRoundClosest(barsMax * (g_vbat100mV - g_eeGeneral.vBatMin - 90), 30 + g_eeGeneral.vBatMax - g_eeGeneral.vBatMin), barsMax);
}

inline bool IS_TXBATT_WARNING()
{
  return g_vbat100mV <= g_eeGeneral.vBatWarn;
}

enum TelemetryViews {
  TELEMETRY_CUSTOM_SCREEN_1,
  TELEMETRY_CUSTOM_SCREEN_2,
  TELEMETRY_CUSTOM_SCREEN_3,
  TELEMETRY_CUSTOM_SCREEN_4,
  TELEMETRY_VIEW_MAX = TELEMETRY_CUSTOM_SCREEN_4
};

extern uint8_t s_frsky_view;

constexpr uint32_t EARTH_RADIUS = 6371009;

void varioWakeup();

#if defined(AUDIO) && defined(BUZZER)
  #define IS_SOUND_OFF() (g_eeGeneral.buzzerMode==e_mode_quiet && g_eeGeneral.beepMode==e_mode_quiet)
#else
  #define IS_SOUND_OFF() (g_eeGeneral.beepMode == e_mode_quiet)
#endif

#define IS_IMPERIAL_ENABLE() (g_eeGeneral.imperial)

#if defined(PCBTARANIS)
  extern const unsigned char logo_taranis[];
#endif

#include "lua/lua_api.h"

#if defined(SDCARD)
enum ClipboardType {
  CLIPBOARD_TYPE_NONE,
  CLIPBOARD_TYPE_CUSTOM_SWITCH,
  CLIPBOARD_TYPE_CUSTOM_FUNCTION,
  CLIPBOARD_TYPE_SD_FILE,
};

#if defined(SIMU)
  #define CLIPBOARD_PATH_LEN 1024
#else
  #define CLIPBOARD_PATH_LEN 32
#endif

struct Clipboard {
  ClipboardType type;
  union {
    LogicalSwitchData csw;
    CustomFunctionData cfn;
    struct {
      char directory[CLIPBOARD_PATH_LEN];
      char filename[CLIPBOARD_PATH_LEN];
    } sd;
  } data;
};

extern Clipboard clipboard;
#endif

#if defined(INTERNAL_GPS)
  #include "gps.h"
#endif

#if defined(JACK_DETECT_GPIO)
enum JackMode {
  JACK_UNSELECTED_MODE,
  JACK_HEADPHONE_MODE,
  JACK_TRAINER_MODE,
  JACK_MAX_MODE = JACK_TRAINER_MODE
};
#endif

#if defined(IMU)
#include "gyro.h"
#endif

#if defined(DEBUG_LATENCY)
extern uint8_t latencyToggleSwitch;
#endif

inline bool isAsteriskDisplayed()
{
#if defined(ASTERISK) || !defined(WATCHDOG) || defined(LOG_TELEMETRY) || defined(LOG_BLUETOOTH) || defined(DEBUG_LATENCY)
  return true;
#endif

  return globalData.unexpectedShutdown;
}

#include "module.h"

extern CircularBuffer<uint8_t, 8> luaSetStickySwitchBuffer;
