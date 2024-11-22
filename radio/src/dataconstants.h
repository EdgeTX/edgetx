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

#if !defined(CFN_ONLY)

#include "board.h"
#include "storage/yaml/yaml_defs.h"


#if defined(EXPORT)
  #define LUA_EXPORT(...)              LEXP(__VA_ARGS__)
  #define LUA_EXPORT_MULTIPLE(...)     LEXP_MULTIPLE(__VA_ARGS__)
  #define LUA_EXPORT_EXTRA(...)        LEXP_EXTRA(__VA_ARGS__)
#else
  #define LUA_EXPORT(...)
  #define LUA_EXPORT_MULTIPLE(...)
  #define LUA_EXPORT_EXTRA(...)
#endif

#define LABELS_LENGTH 100 // Maximum length of the label string
#define LABEL_LENGTH 16

#if defined(COLORLCD) || defined(STM32H747xx)
  #define MAX_MODELS                   60
  #define MAX_OUTPUT_CHANNELS          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES             9
  #define MAX_MIXERS                   64
  #define MAX_EXPOS                    64
  #define MAX_LOGICAL_SWITCHES         64
  #define MAX_SPECIAL_FUNCTIONS        64 // number of functions assigned to switches
  #define MAX_SCRIPTS                  9
  #define MAX_INPUTS                   32
  #define MIN_TRAINER_CHANNELS         4
  #define DEF_TRAINER_CHANNELS         8
  #define MAX_TRAINER_CHANNELS         16
  #define MAX_TELEMETRY_SENSORS        60
  #define MAX_CUSTOM_SCREENS           10
#elif defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E)
  #define MAX_MODELS                   60
  #define MAX_OUTPUT_CHANNELS          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES             9
  #define MAX_MIXERS                   64
  #define MAX_EXPOS                    64
  #define MAX_LOGICAL_SWITCHES         64
  #define MAX_SPECIAL_FUNCTIONS        64 // number of functions assigned to switches
  #define MAX_SCRIPTS                  7
  #define MAX_INPUTS                   32
  #define MIN_TRAINER_CHANNELS         4
  #define DEF_TRAINER_CHANNELS         8
  #define MAX_TRAINER_CHANNELS         16
  #define MAX_TELEMETRY_SENSORS        60
#elif defined(PCBTARANIS)
  #define MAX_MODELS                   60
  #define MAX_OUTPUT_CHANNELS          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES             9
  #define MAX_MIXERS                   64
  #define MAX_EXPOS                    64
  #define MAX_LOGICAL_SWITCHES         64
  #define MAX_SPECIAL_FUNCTIONS        64 // number of functions assigned to switches
  #define MAX_SCRIPTS                  7
  #define MAX_INPUTS                   32
  #define MIN_TRAINER_CHANNELS         4
  #define DEF_TRAINER_CHANNELS         8
  #define MAX_TRAINER_CHANNELS         16
  #define MAX_TELEMETRY_SENSORS        40
#else
  #warning "Unknown board!"
#endif

#define MAX_TIMERS                     3
#define NUM_CAL_PPM                    4

enum CurveType {
  CURVE_TYPE_STANDARD,
  CURVE_TYPE_CUSTOM,
  CURVE_TYPE_LAST = CURVE_TYPE_CUSTOM
};

#define MIN_POINTS_PER_CURVE           3
#define MAX_POINTS_PER_CURVE           17

#if defined(COLORLCD)
  #define LEN_MODEL_NAME               15
  #define LEN_TIMER_NAME               8
  #define LEN_FLIGHT_MODE_NAME         10
  #define LEN_BITMAP_NAME              14
  #define LEN_EXPOMIX_NAME             6
  #define LEN_CHANNEL_NAME             6
  #define LEN_INPUT_NAME               4
  #define LEN_CURVE_NAME               3
  #define LEN_FUNCTION_NAME            8
  #define MAX_CURVES                   32
  #define MAX_CURVE_POINTS             512
#elif LCD_W == 212
  #define LEN_MODEL_NAME               12
  #define LEN_TIMER_NAME               8
  #define LEN_FLIGHT_MODE_NAME         10
  #define LEN_BITMAP_NAME              10
  #define LEN_EXPOMIX_NAME             6
  #define LEN_CHANNEL_NAME             6
  #define LEN_INPUT_NAME               4
  #define LEN_CURVE_NAME               3
  #define LEN_FUNCTION_NAME            8
  #define MAX_CURVES                   32
  #define MAX_CURVE_POINTS             512
#else
  #define LEN_MODEL_NAME               10
  #define LEN_TIMER_NAME               3
  #define LEN_FLIGHT_MODE_NAME         6
  #define LEN_BITMAP_NAME              0
  #define LEN_EXPOMIX_NAME             6
  #define LEN_CHANNEL_NAME             4
  #define LEN_INPUT_NAME               3
  #define LEN_CURVE_NAME               3
  #define LEN_FUNCTION_NAME            8
  #define MAX_CURVES                   32
  #define MAX_CURVE_POINTS             512
#endif

#define NUM_MODULES                    2

#define XPOTS_MULTIPOS_COUNT           6

#if defined(COLORLCD)
enum MainViews {
  VIEW_BLANK,
  VIEW_TIMERS_ALTITUDE,
  VIEW_CHANNELS,
  VIEW_TELEM1,
  VIEW_TELEM2,
  VIEW_TELEM3,
  VIEW_TELEM4,
  VIEW_COUNT
};
#elif LCD_W >= 212
enum MainViews {
  VIEW_TIMERS,
  VIEW_INPUTS,
  VIEW_SWITCHES,
  VIEW_COUNT
};
#else
enum MainViews {
  VIEW_OUTPUTS_VALUES,
  VIEW_OUTPUTS_BARS,
  VIEW_INPUTS,
  VIEW_TIMER2,
  VIEW_CHAN_MONITOR,
  VIEW_COUNT
};
#endif

enum BeeperMode {
  e_mode_quiet = -2,
  e_mode_alarms,
  e_mode_nokeys,
  e_mode_all
};

enum ModuleIndex {
  INTERNAL_MODULE,
  EXTERNAL_MODULE,
  // end of "normal" modules
  
  MAX_MODULES,

  // only used for power control
  // and firmware updates
  SPORT_MODULE = MAX_MODULES
};

enum ArmingMode {
  ARMING_MODE_FIRST = 0,
  ARMING_MODE_CH5 = ARMING_MODE_FIRST,
  ARMING_MODE_SWITCH = 1,
  ARMING_MODE_LAST = ARMING_MODE_SWITCH,
};

enum TrainerMode {
  TRAINER_MODE_OFF,
  TRAINER_MODE_MASTER_TRAINER_JACK,
  TRAINER_MODE_SLAVE,
  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE,
  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE,
  TRAINER_MODE_MASTER_SERIAL,
  TRAINER_MODE_MASTER_BLUETOOTH,
  TRAINER_MODE_SLAVE_BLUETOOTH,
  TRAINER_MODE_MULTI,
};

#define TRAINER_MODE_MIN() TRAINER_MODE_OFF
#define TRAINER_MODE_MAX() TRAINER_MODE_MULTI

enum SerialPort {
    SP_AUX1=0,
    SP_AUX2,
    SP_VCP,
    MAX_SERIAL_PORTS SKIP,
};

#define MAX_AUX_SERIAL (SP_AUX2 + 1)
#define STORAGE_SERIAL_PORTS 4

// GPS
#define PILOTPOS_MIN_HDOP 500

#if defined(HARDWARE_INTERNAL_MODULE)
#define IS_INTERNAL_MODULE_ENABLED() \
  (g_model.moduleData[INTERNAL_MODULE].type != MODULE_TYPE_NONE)
#else
#define IS_INTERNAL_MODULE_ENABLED() (false)
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
#define IS_EXTERNAL_MODULE_ENABLED() \
  (g_model.moduleData[EXTERNAL_MODULE].type != MODULE_TYPE_NONE)
#else
#define IS_EXTERNAL_MODULE_ENABLED() false
#endif

#define IS_MODULE_ENABLED(moduleIdx)                            \
  (g_model.moduleData[moduleIdx].type != MODULE_TYPE_NONE)

enum UartModes {
  UART_MODE_NONE,
  UART_MODE_TELEMETRY_MIRROR,
  UART_MODE_TELEMETRY,
  UART_MODE_SBUS_TRAINER,
  UART_MODE_LUA,
  UART_MODE_CLI,
  UART_MODE_GPS,
  UART_MODE_DEBUG,
  UART_MODE_SPACEMOUSE,
  UART_MODE_EXT_MODULE,
  UART_MODE_COUNT SKIP,
  UART_MODE_MAX SKIP = UART_MODE_COUNT-1
};

#define LEN_SWITCH_NAME    3
#define LEN_ANA_NAME       3
#define LEN_MODEL_FILENAME 16
#define LEN_BLUETOOTH_NAME 10

enum TelemetryProtocol
{
  PROTOCOL_TELEMETRY_FIRST,
  PROTOCOL_TELEMETRY_FRSKY_SPORT = PROTOCOL_TELEMETRY_FIRST,
  PROTOCOL_TELEMETRY_FRSKY_D,
  PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY,
  PROTOCOL_TELEMETRY_CROSSFIRE,
  PROTOCOL_TELEMETRY_SPEKTRUM,
  PROTOCOL_TELEMETRY_FLYSKY_IBUS,
  PROTOCOL_TELEMETRY_HITEC,
  PROTOCOL_TELEMETRY_HOTT,
  PROTOCOL_TELEMETRY_MLINK,
  PROTOCOL_TELEMETRY_MULTIMODULE,
  PROTOCOL_TELEMETRY_AFHDS3,
  PROTOCOL_TELEMETRY_GHOST,
  PROTOCOL_TELEMETRY_FLYSKY_NV14,
  PROTOCOL_TELEMETRY_DSMP,
  PROTOCOL_TELEMETRY_LAST=PROTOCOL_TELEMETRY_DSMP,
  PROTOCOL_TELEMETRY_LUA
};

#define TELEM_LABEL_LEN                4
enum TelemetryUnit {
  UNIT_RAW,
  UNIT_VOLTS,
  UNIT_AMPS,
  UNIT_MILLIAMPS,
  UNIT_KTS,
  UNIT_METERS_PER_SECOND,
  UNIT_FEET_PER_SECOND,
  UNIT_KMH,
  UNIT_SPEED = UNIT_KMH,
  UNIT_MPH,
  UNIT_METERS,
  UNIT_DIST = UNIT_METERS,
  UNIT_FEET,
  UNIT_CELSIUS,
  UNIT_TEMPERATURE = UNIT_CELSIUS,
  UNIT_FAHRENHEIT,
  UNIT_PERCENT,
  UNIT_MAH,
  UNIT_WATTS,
  UNIT_MILLIWATTS,
  UNIT_DB,
  UNIT_RPMS,
  UNIT_G,
  UNIT_DEGREE,
  UNIT_RADIANS,
  UNIT_MILLILITERS,
  UNIT_FLOZ,
  UNIT_MILLILITERS_PER_MINUTE,
  UNIT_HERTZ,
  UNIT_MS,
  UNIT_US,
  UNIT_KM,
  UNIT_DBM,
  UNIT_MAX = UNIT_DBM,
  UNIT_SPARE6,
  UNIT_SPARE7,
  UNIT_SPARE8,
  UNIT_SPARE9,
  UNIT_SPARE10,
  UNIT_HOURS,
  UNIT_MINUTES,
  UNIT_SECONDS,
  // FrSky format used for these fields, could be another format in the future
  UNIT_FIRST_VIRTUAL,
  UNIT_CELLS = UNIT_FIRST_VIRTUAL,
  UNIT_DATETIME,
  UNIT_GPS,
  UNIT_BITFIELD,
  UNIT_TEXT,
  // Internal units (not stored in sensor unit)
  UNIT_GPS_LONGITUDE,
  UNIT_GPS_LATITUDE,
  UNIT_DATETIME_YEAR,
  UNIT_DATETIME_DAY_MONTH,
  UNIT_DATETIME_HOUR_MIN,
  UNIT_DATETIME_SEC
};

// TODO: move to stdlcd UI
#if LCD_W >= 212
  #define NUM_LINE_ITEMS 3
#else
  #define NUM_LINE_ITEMS 2
#endif

#if defined(PCBTARANIS)
  #define MAX_TELEM_SCRIPT_INPUTS  8
#endif

enum TelemetryScreenType {
  TELEMETRY_SCREEN_TYPE_NONE,
  TELEMETRY_SCREEN_TYPE_VALUES,
  TELEMETRY_SCREEN_TYPE_BARS,
  TELEMETRY_SCREEN_TYPE_SCRIPT,
#if defined(LUA)
  TELEMETRY_SCREEN_TYPE_MAX = TELEMETRY_SCREEN_TYPE_SCRIPT
#else
  TELEMETRY_SCREEN_TYPE_MAX = TELEMETRY_SCREEN_TYPE_BARS
#endif
};

#define MAX_TELEMETRY_SCREENS 4

#define TELEMETRY_SCREEN_TYPE(screenIndex)                              \
  TelemetryScreenType((g_model.screensType >> (2 * (screenIndex))) & 0x03)

#define IS_BARS_SCREEN(screenIndex)                                     \
  (TELEMETRY_SCREEN_TYPE(screenIndex) == TELEMETRY_SCREEN_TYPE_BARS)

constexpr int16_t FAILSAFE_CHANNEL_HOLD = 2000;
constexpr int16_t FAILSAFE_CHANNEL_NOPULSE = 2001;

#define LEN_SCRIPT_FILENAME            6
#define LEN_SCRIPT_NAME                6
#define MAX_SCRIPT_INPUTS              6
#define MAX_SCRIPT_OUTPUTS             6

enum PotsWarnMode {
  POTS_WARN_OFF,
  POTS_WARN_MANUAL,
  POTS_WARN_AUTO
};

#define LEN_GVAR_NAME                3
#define GVAR_MAX                     1024
#define GVAR_MIN                     -GVAR_MAX

// we reserve the space inside the range of values, like offset, weight, etc.
#define RESERVE_RANGE_FOR_GVARS      10

#define MAX_GVARS                    9

// Maximum number analog inputs by type
#define MAX_STICKS        4

#if defined(COLORLCD)
  #define MAX_POTS        16
#else
  #define MAX_POTS        8
#endif

#define MAX_VBAT          1
#define MAX_RTC_BAT       1

#define MAX_ANALOG_INPUTS (MAX_STICKS + MAX_POTS + MAX_VBAT + MAX_RTC_BAT)
#define MAX_CALIB_ANALOG_INPUTS (MAX_STICKS + MAX_POTS)

#define MAX_SWITCHES      20

#if !defined(MAX_FLEX_SWITCHES)
#define MAX_FLEX_SWITCHES 0
#endif

#if NUM_TRIMS > 6
#define MAX_TRIMS 8
#else
#define MAX_TRIMS 6
#endif

#define MAX_XPOTS_POSITIONS (MAX_POTS * XPOTS_MULTIPOS_COUNT)

enum SwitchSources {
  SWSRC_NONE = 0,

  SWSRC_FIRST_SWITCH SKIP,
  SWSRC_LAST_SWITCH SKIP = SWSRC_FIRST_SWITCH + (MAX_SWITCHES * 3) - 1,

  SWSRC_FIRST_MULTIPOS_SWITCH SKIP,
  SWSRC_LAST_MULTIPOS_SWITCH SKIP = SWSRC_FIRST_MULTIPOS_SWITCH + MAX_XPOTS_POSITIONS - 1,

  SWSRC_FIRST_TRIM SKIP,
  SWSRC_LAST_TRIM SKIP = SWSRC_FIRST_TRIM + 2 * MAX_TRIMS - 1,

  SWSRC_FIRST_LOGICAL_SWITCH SKIP,
  SWSRC_LAST_LOGICAL_SWITCH SKIP = SWSRC_FIRST_LOGICAL_SWITCH + MAX_LOGICAL_SWITCHES - 1,

  SWSRC_ON,
  SWSRC_ONE,

  SWSRC_FIRST_FLIGHT_MODE SKIP,
  SWSRC_LAST_FLIGHT_MODE SKIP = SWSRC_FIRST_FLIGHT_MODE+MAX_FLIGHT_MODES-1,

  SWSRC_TELEMETRY_STREAMING,

  SWSRC_FIRST_SENSOR SKIP,
  SWSRC_LAST_SENSOR SKIP = SWSRC_FIRST_SENSOR+MAX_TELEMETRY_SENSORS-1,

  SWSRC_RADIO_ACTIVITY,

  SWSRC_TRAINER_CONNECTED,

#if defined(DEBUG_LATENCY)
  SWSRC_LATENCY_TOGGLE,
#endif

  SWSRC_COUNT SKIP,

  SWSRC_OFF = -SWSRC_ON,

  SWSRC_LAST SKIP = SWSRC_COUNT-1,
  SWSRC_FIRST SKIP = -SWSRC_LAST,

  SWSRC_LAST_IN_LOGICAL_SWITCHES SKIP = SWSRC_COUNT-1,
  SWSRC_LAST_IN_MIXES SKIP = SWSRC_COUNT-1,

  SWSRC_FIRST_IN_LOGICAL_SWITCHES SKIP = -SWSRC_LAST_IN_LOGICAL_SWITCHES,
  SWSRC_FIRST_IN_MIXES SKIP = -SWSRC_LAST_IN_MIXES,

  SWSRC_INVERT SKIP = SWSRC_COUNT+1,
};

enum MixSources {
  MIXSRC_NONE,

  MIXSRC_FIRST SKIP,
  MIXSRC_FIRST_INPUT SKIP = MIXSRC_FIRST,
  MIXSRC_LAST_INPUT SKIP = MIXSRC_FIRST_INPUT + MAX_INPUTS - 1,

#if defined(LUA_INPUTS)
  MIXSRC_FIRST_LUA SKIP,
  MIXSRC_LAST_LUA SKIP =
      MIXSRC_FIRST_LUA + (MAX_SCRIPTS * MAX_SCRIPT_OUTPUTS) - 1,
#endif

  // Semantic sticks
  MIXSRC_FIRST_STICK SKIP,
  MIXSRC_LAST_STICK SKIP = MIXSRC_FIRST_STICK + MAX_STICKS - 1,

  MIXSRC_FIRST_POT SKIP,
  MIXSRC_LAST_POT SKIP = MIXSRC_FIRST_POT + MAX_POTS - 1,

#if defined(IMU)
  MIXSRC_TILT_X,
  MIXSRC_TILT_Y,
#endif

#if defined(PCBHORUS)
  MIXSRC_FIRST_SPACEMOUSE SKIP,
  MIXSRC_SPACEMOUSE_A = MIXSRC_FIRST_SPACEMOUSE,
  MIXSRC_SPACEMOUSE_B,
  MIXSRC_SPACEMOUSE_C,
  MIXSRC_SPACEMOUSE_D,
  MIXSRC_SPACEMOUSE_E,
  MIXSRC_SPACEMOUSE_F,
  MIXSRC_LAST_SPACEMOUSE SKIP = MIXSRC_SPACEMOUSE_F,
#endif

  MIXSRC_MIN,
  MIXSRC_MAX,

  MIXSRC_FIRST_HELI SKIP,
  MIXSRC_LAST_HELI SKIP = MIXSRC_FIRST_HELI + 2,

  MIXSRC_FIRST_TRIM SKIP,
  MIXSRC_LAST_TRIM SKIP = MIXSRC_FIRST_TRIM + MAX_TRIMS - 1,

  MIXSRC_FIRST_SWITCH SKIP,
  MIXSRC_LAST_SWITCH SKIP = MIXSRC_FIRST_SWITCH + MAX_SWITCHES - 1,

#if defined(FUNCTION_SWITCHES)
  MIXSRC_FIRST_CUSTOMSWITCH_GROUP SKIP,
  MIXSRC_LAST_CUSTOMSWITCH_GROUP SKIP = MIXSRC_FIRST_CUSTOMSWITCH_GROUP + NUM_FUNCTIONS_GROUPS - 1,
#endif
  MIXSRC_FIRST_LOGICAL_SWITCH SKIP,
  MIXSRC_LAST_LOGICAL_SWITCH SKIP = MIXSRC_FIRST_LOGICAL_SWITCH + MAX_LOGICAL_SWITCHES - 1,

  MIXSRC_FIRST_TRAINER SKIP,
  MIXSRC_LAST_TRAINER SKIP = MIXSRC_FIRST_TRAINER + MAX_TRAINER_CHANNELS - 1,

  MIXSRC_FIRST_CH SKIP,
  MIXSRC_LAST_CH SKIP = MIXSRC_FIRST_CH + MAX_OUTPUT_CHANNELS - 1,

  MIXSRC_FIRST_GVAR SKIP,
  MIXSRC_LAST_GVAR SKIP = MIXSRC_FIRST_GVAR + MAX_GVARS - 1,

  MIXSRC_TX_VOLTAGE,
  MIXSRC_TX_TIME,
  MIXSRC_TX_GPS,

  MIXSRC_FIRST_TIMER SKIP,
  MIXSRC_LAST_TIMER SKIP = MIXSRC_FIRST_TIMER + MAX_TIMERS - 1,

  MIXSRC_FIRST_TELEM SKIP,
  MIXSRC_LAST_TELEM SKIP = MIXSRC_FIRST_TELEM + 3 * MAX_TELEMETRY_SENSORS - 1,

  MIXSRC_INVERT SKIP,
  MIXSRC_VALUE SKIP,  // Special case to trigger source as value conversion
};

#define MIXSRC_LAST                 MIXSRC_LAST_GVAR
#define INPUTSRC_FIRST              MIXSRC_FIRST_STICK
#define INPUTSRC_LAST               MIXSRC_LAST_TELEM

#if defined(FUNCTION_SWITCHES)
#define MIXSRC_LAST_REGULAR_SWITCH  (MIXSRC_FIRST_SWITCH + switchGetMaxSwitches() - 1)
#define MIXSRC_FIRST_FS_SWITCH      (MIXSRC_LAST_REGULAR_SWITCH + 1)
#endif

enum BacklightMode {
  e_backlight_mode_off  = 0,
  e_backlight_mode_keys = 1,
  e_backlight_mode_sticks = 2,
  e_backlight_mode_all = e_backlight_mode_keys+e_backlight_mode_sticks,
  e_backlight_mode_on
};

enum TimerModes {
  TMRMODE_OFF,
  TMRMODE_ON,
  TMRMODE_START,
  TMRMODE_THR,
  TMRMODE_THR_REL,
  TMRMODE_THR_START,
  TMRMODE_COUNT SKIP,
  TMRMODE_MAX SKIP = TMRMODE_COUNT - 1
};

enum CountDownModes {
  COUNTDOWN_SILENT,
  COUNTDOWN_BEEPS,
  COUNTDOWN_VOICE,
  COUNTDOWN_NON_HAPTIC_LAST = COUNTDOWN_VOICE,
#if defined(HAPTIC)
  COUNTDOWN_HAPTIC,
  COUNTDOWN_BEEPS_AND_HAPTIC,
  COUNTDOWN_VOICE_AND_HPTIC,
#endif
  COUNTDOWN_COUNT SKIP
};

enum ResetFunctionParam {
  FUNC_RESET_TIMER1,
  FUNC_RESET_TIMER2,
  FUNC_RESET_TIMER3,
  FUNC_RESET_FLIGHT,
  FUNC_RESET_TELEMETRY,
  FUNC_RESET_TRIMS,
  FUNC_RESET_PARAM_FIRST_TELEM,
  FUNC_RESET_PARAM_LAST_TELEM = FUNC_RESET_PARAM_FIRST_TELEM + MAX_TELEMETRY_SENSORS,
  FUNC_RESET_PARAMS_COUNT SKIP,
  FUNC_RESET_PARAM_LAST SKIP = FUNC_RESET_PARAMS_COUNT-1,
};

enum AdjustGvarFunctionParam {
  FUNC_ADJUST_GVAR_CONSTANT,
  FUNC_ADJUST_GVAR_SOURCE,
  FUNC_ADJUST_GVAR_SOURCERAW,
  FUNC_ADJUST_GVAR_GVAR,
  FUNC_ADJUST_GVAR_INCDEC,
};

enum BluetoothModes {
  BLUETOOTH_OFF,
  BLUETOOTH_TELEMETRY,
  BLUETOOTH_TRAINER,
#if defined(PCBX9E)
  BLUETOOTH_MAX SKIP = BLUETOOTH_TELEMETRY
#else
  BLUETOOTH_MAX SKIP = BLUETOOTH_TRAINER
#endif
};

enum HatsMode {
  HATSMODE_TRIMS_ONLY,
  HATSMODE_KEYS_ONLY,
  HATSMODE_SWITCHABLE,
  HATSMODE_GLOBAL
};

enum UartSampleModes {
  UART_SAMPLE_MODE_NORMAL = 0,
  UART_SAMPLE_MODE_ONEBIT,

  UART_SAMPLE_MODE_MAX SKIP = UART_SAMPLE_MODE_ONEBIT
};

// PXX2 constants
#define PXX2_LEN_REGISTRATION_ID            8
#define PXX2_LEN_RX_NAME                    8
#define PXX2_MAX_RECEIVERS_PER_MODULE       3

// A model On/Off setting that can also take a global value
enum ModelOverridableEnable {
  OVERRIDE_GLOBAL,
  OVERRIDE_OFF,
  OVERRIDE_ON
};

#define SELECTED_THEME_NAME_LEN 26

// PPM Units
enum PPMUnit {
    PPM_PERCENT_PREC0,
    PPM_PERCENT_PREC1,
    PPM_US
};

#endif

enum Functions {
  FUNC_OVERRIDE_CHANNEL,
  FUNC_TRAINER,
  FUNC_INSTANT_TRIM,
  FUNC_RESET,
  FUNC_SET_TIMER,
  FUNC_ADJUST_GVAR,
  FUNC_VOLUME,
  FUNC_SET_FAILSAFE,
  FUNC_RANGECHECK,
  FUNC_BIND,
  FUNC_PLAY_SOUND,
  FUNC_PLAY_TRACK,
  FUNC_PLAY_VALUE,
  FUNC_PLAY_SCRIPT,
  FUNC_BACKGND_MUSIC,
  FUNC_BACKGND_MUSIC_PAUSE,
  FUNC_VARIO,
  FUNC_HAPTIC,
  FUNC_LOGS,
  FUNC_BACKLIGHT,
  FUNC_SCREENSHOT,
  FUNC_RACING_MODE,
#if defined(COLORLCD) || defined(CFN_ONLY)
  FUNC_DISABLE_TOUCH,
#endif
  FUNC_SET_SCREEN,
  FUNC_DISABLE_AUDIO_AMP,
  FUNC_RGB_LED,
#if defined(VIDEO_SWITCH) || defined(CFN_ONLY)
  FUNC_LCD_TO_VIDEO,
#endif
#if defined(FUNCTION_SWITCHES) || defined(CFN_ONLY)
  FUNC_PUSH_CUST_SWITCH,
#endif
  FUNC_TEST, // MUST remain last
#if defined(DEBUG)
  FUNC_MAX SKIP
#else
  FUNC_MAX SKIP = FUNC_TEST
#endif
};
