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

#ifndef _DATACONSTANTS_H_
#define _DATACONSTANTS_H_

#include "board.h"
#include "storage/yaml/yaml_defs.h"

#define NUM_STICKS                     4

#if defined(EXPORT)
  #define LUA_EXPORT(...)              LEXP(__VA_ARGS__)
  #define LUA_EXPORT_MULTIPLE(...)     LEXP_MULTIPLE(__VA_ARGS__)
  #define LUA_EXPORT_EXTRA(...)        LEXP_EXTRA(__VA_ARGS__)
#else
  #define LUA_EXPORT(...)
  #define LUA_EXPORT_MULTIPLE(...)
  #define LUA_EXPORT_EXTRA(...)
#endif

#if defined(PCBHORUS) || defined(PCBNV14)
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
  #define MAX_CUSTOM_SCREENS           5
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

#if defined(PCBHORUS) || defined(PCBNV14)
  #define LEN_MODEL_NAME               15
  #define LEN_TIMER_NAME               8
  #define LEN_FLIGHT_MODE_NAME         10
  #define LEN_BITMAP_NAME              14
  #define LEN_EXPOMIX_NAME             6
  #define LEN_CHANNEL_NAME             6
  #define LEN_INPUT_NAME               4
  #define LEN_CURVE_NAME               3
  #define LEN_FUNCTION_NAME            6
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
  #define LEN_EXPOMIX_NAME             6
  #define LEN_CHANNEL_NAME             4
  #define LEN_INPUT_NAME               3
  #define LEN_CURVE_NAME               3
  #define LEN_FUNCTION_NAME            6
  #define MAX_CURVES                   32
  #define MAX_CURVE_POINTS             512
#endif

#if defined(PCBFRSKY) || defined(PCBNV14)
  #define NUM_MODULES                  2
#else
  #define NUM_MODULES                  1
#endif

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
  SPORT_MODULE
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
  #define IS_INTERNAL_MODULE_ENABLED() (g_model.moduleData[INTERNAL_MODULE].type != MODULE_TYPE_NONE)
#else
  #define IS_INTERNAL_MODULE_ENABLED() (false)
#endif

#define IS_EXTERNAL_MODULE_ENABLED() (g_model.moduleData[EXTERNAL_MODULE].type != MODULE_TYPE_NONE)

#if defined(HARDWARE_INTERNAL_MODULE)
  #define IS_MODULE_ENABLED(moduleIdx)         (moduleIdx==INTERNAL_MODULE ? IS_INTERNAL_MODULE_ENABLED() : moduleIdx==EXTERNAL_MODULE ? IS_EXTERNAL_MODULE_ENABLED() : false)
#else
  #define IS_MODULE_ENABLED(moduleIdx)         (moduleIdx==EXTERNAL_MODULE ? IS_EXTERNAL_MODULE_ENABLED() : false)
#endif

enum UartModes {
  UART_MODE_NONE,
  UART_MODE_TELEMETRY_MIRROR,
  UART_MODE_TELEMETRY,
  UART_MODE_SBUS_TRAINER,
  UART_MODE_LUA,
  UART_MODE_CLI,
  UART_MODE_GPS,
  UART_MODE_DEBUG,
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
#define TELEMETRY_SCREEN_TYPE(screenIndex) TelemetryScreenType((g_model.screensType >> (2*(screenIndex))) & 0x03)
#define IS_BARS_SCREEN(screenIndex)        (TELEMETRY_SCREEN_TYPE(screenIndex) == TELEMETRY_SCREEN_TYPE_BARS)

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

#define RESERVE_RANGE_FOR_GVARS        10
// even we do not spend space in EEPROM for 10 GVARS, we reserve the space inside the range of values, like offset, weight, etc.

#define MAX_GVARS                    9

enum SwitchSources {
  SWSRC_NONE = 0,

  SWSRC_FIRST_SWITCH SKIP,

#if defined(STORAGE_SWITCH_A)
  SWSRC_SA0 = SWSRC_FIRST_SWITCH,
  SWSRC_SA1,
  SWSRC_SA2,
#endif

#if defined(STORAGE_SWITCH_B)
  SWSRC_SB0,
  SWSRC_SB1,
  SWSRC_SB2,
#endif

#if defined(STORAGE_SWITCH_C)
  SWSRC_SC0,
  SWSRC_SC1,
  SWSRC_SC2,
#endif

#if defined(STORAGE_SWITCH_D)
  SWSRC_SD0,
  SWSRC_SD1,
  SWSRC_SD2,
#endif

#if defined(FUNCTION_SWITCHES) && defined(RADIO_TPRO)
  SWSRC_FIRST_FUNCTION_SWITCH SKIP,
  SWSRC_SE0 = SWSRC_FIRST_FUNCTION_SWITCH,
  SWSRC_SE1,
  SWSRC_SE2,
#elif defined(STORAGE_SWITCH_E)
  SWSRC_SE0,
  SWSRC_SE1,
  SWSRC_SE2,
#endif

#if defined(STORAGE_SWITCH_F)
  SWSRC_SF0,
  SWSRC_SF1,
  SWSRC_SF2,
#endif

#if defined(STORAGE_SWITCH_G)
  SWSRC_SG0,
  SWSRC_SG1,
  SWSRC_SG2,
#endif

#if defined(STORAGE_SWITCH_H)
  SWSRC_SH0,
  SWSRC_SH1,
  SWSRC_SH2,
#endif

#if defined(STORAGE_SWITCH_I)
  SWSRC_SI0,
  SWSRC_SI1,
  SWSRC_SI2,
#endif

#if defined(STORAGE_SWITCH_J)
  SWSRC_SJ0,
  SWSRC_SJ1,
  SWSRC_SJ2,
#endif

#if defined(STORAGE_SWITCH_K)
  SWSRC_SK0,
  SWSRC_SK1,
  SWSRC_SK2,
#endif

#if defined(STORAGE_SWITCH_L)
  SWSRC_SL0,
  SWSRC_SL1,
  SWSRC_SL2,
#endif

#if defined(STORAGE_SWITCH_M)
  SWSRC_SM0,
  SWSRC_SM1,
  SWSRC_SM2,
#endif

#if defined(STORAGE_SWITCH_N)
  SWSRC_SN0,
  SWSRC_SN1,
  SWSRC_SN2,
#endif

#if defined(STORAGE_SWITCH_O)
  SWSRC_SO0,
  SWSRC_SO1,
  SWSRC_SO2,
#endif

#if defined(STORAGE_SWITCH_P)
  SWSRC_SP0,
  SWSRC_SP1,
  SWSRC_SP2,
#endif

#if defined(STORAGE_SWITCH_Q)
  SWSRC_SQ0,
  SWSRC_SQ1,
  SWSRC_SQ2,
#endif

#if defined(STORAGE_SWITCH_R)
  SWSRC_SR0,
  SWSRC_SR1,
  SWSRC_SR2,
#endif

  SWSRC_LAST_SWITCH SKIP = SWSRC_FIRST_SWITCH + STORAGE_NUM_SWITCHES_POSITIONS - 1,

#if NUM_XPOTS > 0
  SWSRC_FIRST_MULTIPOS_SWITCH SKIP,
  SWSRC_LAST_MULTIPOS_SWITCH SKIP = SWSRC_FIRST_MULTIPOS_SWITCH + (NUM_XPOTS * XPOTS_MULTIPOS_COUNT) - 1,
#endif

  SWSRC_FIRST_TRIM SKIP,
  SWSRC_TrimRudLeft = SWSRC_FIRST_TRIM,
  SWSRC_TrimRudRight,
  SWSRC_TrimEleDown,
  SWSRC_TrimEleUp,

#if NUM_TRIMS > 2
  SWSRC_TrimThrDown,
  SWSRC_TrimThrUp,
  SWSRC_TrimAilLeft,
  SWSRC_TrimAilRight,
#endif

#if NUM_TRIMS > 4
  SWSRC_TrimT5Down,
  SWSRC_TrimT5Up,
  SWSRC_TrimT6Down,
  SWSRC_TrimT6Up,
#endif

  SWSRC_FIRST_LOGICAL_SWITCH SKIP,
  SWSRC_SW1 = SWSRC_FIRST_LOGICAL_SWITCH,
  SWSRC_SW2,
  // ...
  SWSRC_LAST_LOGICAL_SWITCH SKIP = SWSRC_FIRST_LOGICAL_SWITCH+MAX_LOGICAL_SWITCHES-1,

  SWSRC_ON,
  SWSRC_ONE,

  SWSRC_FIRST_FLIGHT_MODE SKIP,
  SWSRC_LAST_FLIGHT_MODE SKIP = SWSRC_FIRST_FLIGHT_MODE+MAX_FLIGHT_MODES-1,

  SWSRC_TELEMETRY_STREAMING,

  SWSRC_FIRST_SENSOR SKIP,
  SWSRC_LAST_SENSOR SKIP = SWSRC_FIRST_SENSOR+MAX_TELEMETRY_SENSORS-1,

  SWSRC_RADIO_ACTIVITY,

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

#if NUM_SWITCHES - NUM_FUNCTIONS_SWITCHES >= 8
  #define SWSRC_TRAINER SWSRC_SH2
#else
  #define SWSRC_TRAINER SWSRC_LAST_SWITCH,
#endif

#define SWSRC_LAST_TRIM                 (SWSRC_FIRST_TRIM + 2 * NUM_TRIMS - 1)

enum MixSources {
  MIXSRC_NONE,

  MIXSRC_FIRST_INPUT SKIP,              LUA_EXPORT_MULTIPLE("input", "Input [I%d]", MAX_INPUTS)
  MIXSRC_LAST_INPUT SKIP = MIXSRC_FIRST_INPUT+MAX_INPUTS-1,

#if defined(LUA_INPUTS)
  MIXSRC_FIRST_LUA SKIP,                LUA_EXPORT_MULTIPLE("lua", "Lua mix output %d", MAX_SCRIPTS*MAX_SCRIPT_OUTPUTS)
  MIXSRC_LAST_LUA SKIP = MIXSRC_FIRST_LUA+(MAX_SCRIPTS*MAX_SCRIPT_OUTPUTS)-1,
#endif

  MIXSRC_FIRST_STICK SKIP,
  MIXSRC_Rud = MIXSRC_FIRST_STICK,      LUA_EXPORT("rud", "Rudder")
  MIXSRC_Ele,                           LUA_EXPORT("ele", "Elevator")
  MIXSRC_Thr,                           LUA_EXPORT("thr", "Throttle")
  MIXSRC_Ail,                           LUA_EXPORT("ail", "Aileron")

  MIXSRC_LAST_STICK SKIP = MIXSRC_Ail,
  MIXSRC_FIRST_POT SKIP,
#if defined(PCBHORUS)
  MIXSRC_S1 = MIXSRC_FIRST_POT,         LUA_EXPORT("s1", "Potentiometer S1")
  MIXSRC_6POS,                          LUA_EXPORT("6pos", "Multipos Switch")
  MIXSRC_S2,                            LUA_EXPORT("s2", "Potentiometer S2")
#if defined(PCBX10)
  MIXSRC_EXT1,                          LUA_EXPORT("ext1", "Ext 1")
  MIXSRC_EXT2,                          LUA_EXPORT("ext2", "Ext 2")
  MIXSRC_EXT3,                          LUA_EXPORT("ext3", "Ext 3")
  MIXSRC_EXT4,                          LUA_EXPORT("ext4", "Ext 4")
#endif
  MIXSRC_FIRST_SLIDER SKIP,
#if defined(PCBX12S)
  MIXSRC_S3 = MIXSRC_FIRST_SLIDER,      LUA_EXPORT("s3", "Slider S3")
  MIXSRC_S4,                            LUA_EXPORT("s4", "Slider S4")
  MIXSRC_LS,                            LUA_EXPORT("ls", "Left rear slider")
  MIXSRC_RS,                            LUA_EXPORT("rs", "Right rear slider")
#else
  MIXSRC_LS = MIXSRC_FIRST_SLIDER,      LUA_EXPORT("ls", "Left slider")
  MIXSRC_RS,                            LUA_EXPORT("rs", "Right slider")
#endif
  MIXSRC_LAST_POT SKIP = MIXSRC_RS,
#elif defined(PCBX9E)
  MIXSRC_POT1 = MIXSRC_FIRST_POT,       LUA_EXPORT("s1", "Potentiometer 1")
  MIXSRC_POT2,                          LUA_EXPORT("s2", "Potentiometer 2")
  MIXSRC_POT3,                          LUA_EXPORT("s3", "Potentiometer 3")
  MIXSRC_POT4,                          LUA_EXPORT("s4", "Potentiometer 4 (X9E only)")
  MIXSRC_FIRST_SLIDER SKIP,
  MIXSRC_SLIDER1 = MIXSRC_FIRST_SLIDER, LUA_EXPORT("ls", "Left slider")
  MIXSRC_SLIDER2,                       LUA_EXPORT("rs", "Right slider")
  MIXSRC_SLIDER3,                       LUA_EXPORT("lcs", "Left center slider (X9E only)")
  MIXSRC_SLIDER4,                       LUA_EXPORT("rcs", "Right center slider (X9E only)")
  MIXSRC_LAST_POT SKIP = MIXSRC_SLIDER4,
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBNV14)
  MIXSRC_POT1 = MIXSRC_FIRST_POT,       LUA_EXPORT("s1", "Potentiometer 1")
  MIXSRC_POT2,                          LUA_EXPORT("s2", "Potentiometer 2")
  MIXSRC_FIRST_SLIDER SKIP = MIXSRC_POT2,
  MIXSRC_LAST_POT SKIP = MIXSRC_POT2,
#elif defined(PCBX9LITE)
  MIXSRC_POT1 = MIXSRC_FIRST_POT,       LUA_EXPORT("s1", "Potentiometer 1")
  MIXSRC_FIRST_SLIDER SKIP = MIXSRC_POT1,
  MIXSRC_LAST_POT SKIP = MIXSRC_POT1,
#elif defined(PCBTARANIS)
  MIXSRC_POT1 = MIXSRC_FIRST_POT,       LUA_EXPORT("s1", "Potentiometer 1")
  MIXSRC_POT2,                          LUA_EXPORT("s2", "Potentiometer 2")
  MIXSRC_POT3,                          LUA_EXPORT("s3", "Potentiometer 3")
  MIXSRC_FIRST_SLIDER SKIP,
  MIXSRC_SLIDER1 = MIXSRC_FIRST_SLIDER, LUA_EXPORT("ls", "Left slider")
  MIXSRC_SLIDER2,                       LUA_EXPORT("rs", "Right slider")
  MIXSRC_LAST_POT SKIP = MIXSRC_SLIDER2,
#else
  MIXSRC_P1 = MIXSRC_FIRST_POT,
  MIXSRC_P2,
  MIXSRC_P3,
  MIXSRC_LAST_POT SKIP = MIXSRC_P3,
#endif

#if defined(PCBHORUS)
  MIXSRC_MOUSE1,                        LUA_EXPORT("jsx", "Joystick X")
  MIXSRC_MOUSE2,                        LUA_EXPORT("jsy", "Joystick Y")
#endif

#if defined(IMU)
  MIXSRC_TILT_X,                       LUA_EXPORT("tiltx", "Tilt X")
  MIXSRC_TILT_Y,                       LUA_EXPORT("tilty", "Tilt Y")
#endif

  MIXSRC_MAX,                          LUA_EXPORT("max", "MAX")

  MIXSRC_FIRST_HELI SKIP,
  MIXSRC_CYC1 = MIXSRC_FIRST_HELI,     LUA_EXPORT("cyc1", "Cyclic 1")
  MIXSRC_CYC2,                         LUA_EXPORT("cyc2", "Cyclic 2")
  MIXSRC_CYC3,                         LUA_EXPORT("cyc3", "Cyclic 3")
  MIXSRC_LAST_HELI SKIP = MIXSRC_CYC3,

  MIXSRC_FIRST_TRIM SKIP,
  MIXSRC_TrimRud = MIXSRC_FIRST_TRIM,  LUA_EXPORT("trim-rud", "Rudder trim")
  MIXSRC_TrimEle,                      LUA_EXPORT("trim-ele", "Elevator trim")
  MIXSRC_TrimThr,                      LUA_EXPORT("trim-thr", "Throttle trim")
  MIXSRC_TrimAil,                      LUA_EXPORT("trim-ail", "Aileron trim")
#if defined(PCBHORUS)
  MIXSRC_TrimT5,                       LUA_EXPORT("trim-t5", "Aux trim T5")
  MIXSRC_TrimT6,                       LUA_EXPORT("trim-t6", "Aux trim T6")
  MIXSRC_LAST_TRIM SKIP = MIXSRC_TrimT6,
#else
  MIXSRC_LAST_TRIM SKIP = MIXSRC_TrimAil,
#endif

  MIXSRC_FIRST_SWITCH SKIP,

#if defined(HARDWARE_SWITCH_A)
  MIXSRC_SA = MIXSRC_FIRST_SWITCH,  LUA_EXPORT("sa", "Switch A")
  MIXSRC_SB,                        LUA_EXPORT("sb", "Switch B")
  MIXSRC_SC,                        LUA_EXPORT("sc", "Switch C")
#endif
#if defined(HARDWARE_SWITCH_D)
  MIXSRC_SD,                        LUA_EXPORT("sd", "Switch D")
#elif defined(STORAGE_SWITCH_D)
  MIXSRC_SD,
#endif
#if defined(HARDWARE_SWITCH_E)
  MIXSRC_SE,                        LUA_EXPORT("se", "Switch E")
#elif defined(STORAGE_SWITCH_E)
  MIXSRC_SE,
#endif
#if defined(HARDWARE_SWITCH_F)
  MIXSRC_SF,                        LUA_EXPORT("sf", "Switch F")
#elif defined(STORAGE_SWITCH_F)
  MIXSRC_SF,
#endif
#if defined(HARDWARE_SWITCH_G)
  MIXSRC_SG,                        LUA_EXPORT("sg", "Switch G")
#elif defined(STORAGE_SWITCH_G)
  MIXSRC_SG,
#endif
#if defined(HARDWARE_SWITCH_H)
  MIXSRC_SH,                        LUA_EXPORT("sh", "Switch H")
#elif defined(STORAGE_SWITCH_H)
  MIXSRC_SH,
#endif
#if defined(HARDWARE_SWITCH_I)
  MIXSRC_SI,                        LUA_EXPORT("si", "Switch I")
#elif defined(STORAGE_SWITCH_I)
  MIXSRC_SI,
#endif
#if defined(HARDWARE_SWITCH_J)
  MIXSRC_SJ,                        LUA_EXPORT("sj", "Switch J")
#elif defined(STORAGE_SWITCH_J)
  MIXSRC_SJ,
#endif
#if defined(HARDWARE_SWITCH_K)
  MIXSRC_SK,                        LUA_EXPORT("sk", "Switch K")
  MIXSRC_SL,                        LUA_EXPORT("sl", "Switch L")
  MIXSRC_SM,                        LUA_EXPORT("sm", "Switch M")
  MIXSRC_SN,                        LUA_EXPORT("sn", "Switch N")
  MIXSRC_SO,                        LUA_EXPORT("so", "Switch O")
  MIXSRC_SP,                        LUA_EXPORT("sp", "Switch P")
  MIXSRC_SQ,                        LUA_EXPORT("sq", "Switch Q")
  MIXSRC_SR,                        LUA_EXPORT("sr", "Switch R")
#endif
  MIXSRC_FIRST_LOGICAL_SWITCH SKIP,
  MIXSRC_SW1 = MIXSRC_FIRST_LOGICAL_SWITCH, LUA_EXPORT_MULTIPLE("ls", "Logical switch L%d", MAX_LOGICAL_SWITCHES)
  MIXSRC_LAST_LOGICAL_SWITCH SKIP = MIXSRC_FIRST_LOGICAL_SWITCH+MAX_LOGICAL_SWITCHES-1,

  MIXSRC_FIRST_TRAINER SKIP,                LUA_EXPORT_MULTIPLE("trn", "Trainer input %d", MAX_TRAINER_CHANNELS)
  MIXSRC_LAST_TRAINER SKIP = MIXSRC_FIRST_TRAINER+MAX_TRAINER_CHANNELS-1,

  MIXSRC_FIRST_CH SKIP,
  MIXSRC_CH1 = MIXSRC_FIRST_CH,             LUA_EXPORT_MULTIPLE("ch", "Channel CH%d", MAX_OUTPUT_CHANNELS)
  MIXSRC_CH2,
  MIXSRC_CH3,
  MIXSRC_CH4,
  MIXSRC_CH5,
  MIXSRC_CH6,
  MIXSRC_CH7,
  MIXSRC_CH8,
  MIXSRC_CH9,
  MIXSRC_CH10,
  MIXSRC_CH11,
  MIXSRC_CH12,
  MIXSRC_CH13,
  MIXSRC_CH14,
  MIXSRC_CH15,
  MIXSRC_CH16,
  MIXSRC_LAST_CH SKIP = MIXSRC_CH1+MAX_OUTPUT_CHANNELS-1,

  MIXSRC_FIRST_GVAR SKIP,
  MIXSRC_GVAR1 = MIXSRC_FIRST_GVAR,         LUA_EXPORT_MULTIPLE("gvar", "Global variable %d", MAX_GVARS)
  MIXSRC_LAST_GVAR SKIP = MIXSRC_FIRST_GVAR+MAX_GVARS-1,

  MIXSRC_TX_VOLTAGE,                        LUA_EXPORT("tx-voltage", "Transmitter battery voltage [volts]")
  MIXSRC_TX_TIME,                           LUA_EXPORT("clock", "RTC clock [minutes from midnight]")
  MIXSRC_TX_GPS,
  MIXSRC_FIRST_RESERVE SKIP,
  MIXSRC_RESERVE3 SKIP,
  MIXSRC_RESERVE4 SKIP,
  MIXSRC_LAST_RESERVE SKIP,
  MIXSRC_FIRST_TIMER SKIP,
  MIXSRC_TIMER1 = MIXSRC_FIRST_TIMER,       LUA_EXPORT("timer1", "Timer 1 value [seconds]")
  MIXSRC_TIMER2,                            LUA_EXPORT("timer2", "Timer 2 value [seconds]")
  MIXSRC_TIMER3,                            LUA_EXPORT("timer3", "Timer 3 value [seconds]")
  MIXSRC_LAST_TIMER SKIP = MIXSRC_TIMER3,

  MIXSRC_FIRST_TELEM SKIP,                  LUA_EXPORT_MULTIPLE("telem", "Telemetry sensor %d", MAX_TELEMETRY_SENSORS)
  MIXSRC_LAST_TELEM SKIP = MIXSRC_FIRST_TELEM+3*MAX_TELEMETRY_SENSORS-1
};

#if defined(__cplusplus)
static_assert(MIXSRC_FIRST_LOGICAL_SWITCH == MIXSRC_FIRST_SWITCH + STORAGE_NUM_SWITCHES, "Wrong switches definition in MIXSRC list");
#endif

#define MIXSRC_FIRST                (MIXSRC_NONE + 1)
#define MIXSRC_LAST                 MIXSRC_LAST_CH
#define MIXSRC_LAST_SWITCH          (MIXSRC_FIRST_SWITCH + STORAGE_NUM_SWITCHES - 1)
#define INPUTSRC_FIRST              MIXSRC_Rud
#define INPUTSRC_LAST               MIXSRC_LAST_TELEM
#if defined(FUNCTION_SWITCHES)
#define MIXSRC_LAST_REGULAR_SWITCH  (MIXSRC_FIRST_SWITCH + NUM_REGULAR_SWITCHES - 1)
#define MIXSRC_FIRST_FS_SWITCH      (MIXSRC_LAST_REGULAR_SWITCH + 1)
#endif

enum BacklightMode {
  e_backlight_mode_off  = 0,
  e_backlight_mode_keys = 1,
  e_backlight_mode_sticks = 2,
  e_backlight_mode_all = e_backlight_mode_keys+e_backlight_mode_sticks,
  e_backlight_mode_on
};

enum Functions {
  // first the functions which need a checkbox
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
  // then the other functions
  FUNC_FIRST_WITHOUT_ENABLE SKIP,
  FUNC_PLAY_SOUND = FUNC_FIRST_WITHOUT_ENABLE,
  FUNC_PLAY_TRACK,
  FUNC_PLAY_VALUE,
  FUNC_RESERVE4,
  FUNC_PLAY_SCRIPT,
  FUNC_RESERVE5,
  FUNC_BACKGND_MUSIC,
  FUNC_BACKGND_MUSIC_PAUSE,
  FUNC_VARIO,
  FUNC_HAPTIC,
  FUNC_LOGS,
  FUNC_SCREENSHOT,
  FUNC_RACING_MODE,
#if defined(COLORLCD)
  FUNC_DISABLE_TOUCH,
#endif
#if defined(DEBUG)
  FUNC_TEST, // should remain the last before MAX as not added in Companion
#endif
  FUNC_MAX SKIP
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
#if defined(HAPTIC)
  COUNTDOWN_HAPTIC,
#endif
  COUNTDOWN_COUNT SKIP
};

enum ResetFunctionParam {
  FUNC_RESET_TIMER1,
  FUNC_RESET_TIMER2,
  FUNC_RESET_TIMER3,
  FUNC_RESET_FLIGHT,
  FUNC_RESET_TELEMETRY,
  FUNC_RESET_PARAM_FIRST_TELEM,
  FUNC_RESET_PARAM_LAST_TELEM = FUNC_RESET_PARAM_FIRST_TELEM + MAX_TELEMETRY_SENSORS,
  FUNC_RESET_PARAMS_COUNT SKIP,
  FUNC_RESET_PARAM_LAST SKIP = FUNC_RESET_PARAMS_COUNT-1,
};

enum AdjustGvarFunctionParam {
  FUNC_ADJUST_GVAR_CONSTANT,
  FUNC_ADJUST_GVAR_SOURCE,
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

#endif // _DATACONSTANTS_H_
